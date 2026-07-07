import os
import sys
import torch
import torch.optim as optim
import torch.nn.functional as F
import chess.pgn
import time
import zlib
import concurrent.futures
import io
from torch.utils.data import Dataset, DataLoader

# Import the model from the training directory
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'training'))
from model import AlphaZeroNet

def uci_to_index(uci_move):
    # Using zlib.crc32 instead of hash() because hash() is randomized per process in Python 3,
    # which would scramble the policy indices when using multiprocessing.
    return zlib.crc32(uci_move.encode()) % 4096

def fen_to_tensor(fen):
    import chess
    board = chess.Board(fen)
    tensor = torch.zeros((14, 8, 8), dtype=torch.float32)

    # Optimized from iterating 64 squares to iterating only occupied squares
    for square, piece in board.piece_map().items():
        color_offset = 0 if piece.color == chess.WHITE else 6
        piece_type_offset = piece.piece_type - 1
        channel = color_offset + piece_type_offset

        rank = chess.square_rank(square)
        file = chess.square_file(square)
        tensor[channel, rank, file] = 1.0

    color_val = 1.0 if board.turn == chess.WHITE else -1.0
    tensor[12, :, :] = color_val

    castling_rights = fen.split(' ')[2]
    castling_val = float(ord(castling_rights[0])) if len(castling_rights) > 0 else 0.0
    tensor[13, :, :] = castling_val

    return tensor

def process_game_text(game_text):
    """Worker function for multiprocessing."""
    import chess.pgn
    import io
    import torch
    import zlib

    game = chess.pgn.read_game(io.StringIO(game_text))
    if game is None:
        return None, None, None

    result_str = game.headers.get("Result", "*")
    if result_str == "1-0":
        outcome = 1.0
    elif result_str == "0-1":
        outcome = -1.0
    elif result_str == "1/2-1/2":
        outcome = 0.0
    else:
        return None, None, None # Skip unfinished games

    tensors = []
    policies = []
    values = []

    board = game.board()
    for move in game.mainline_moves():
        # Highly optimized fen_to_tensor logic inlined to avoid extra fen() calls where possible
        fen = board.fen()
        tensor = torch.zeros((14, 8, 8), dtype=torch.float32)

        for square, piece in board.piece_map().items():
            color_offset = 0 if piece.color == chess.WHITE else 6
            piece_type_offset = piece.piece_type - 1
            channel = color_offset + piece_type_offset

            rank = chess.square_rank(square)
            file = chess.square_file(square)
            tensor[channel, rank, file] = 1.0

        tensor[12, :, :] = 1.0 if board.turn == chess.WHITE else -1.0

        castling_rights = fen.split(' ')[2]
        tensor[13, :, :] = float(ord(castling_rights[0])) if len(castling_rights) > 0 else 0.0

        tensors.append(tensor)

        # Policy: store just the index to save 20GB of RAM per chunk
        idx = zlib.crc32(move.uci().encode()) % 4096
        policies.append(torch.tensor(idx, dtype=torch.long))

        # Value: Who won the game
        values.append(torch.tensor([outcome], dtype=torch.float32))

        board.push(move)

    if not tensors:
        return None, None, None
    return torch.stack(tensors).numpy(), torch.stack(policies).numpy(), torch.stack(values).numpy()

# SupervisedDataset removed in favor of torch.utils.data.TensorDataset

def parse_pgn_file(pgn_path, max_games=10000):
    tensors = []
    policies = []
    values = []

    print(f"Parsing PGN file: {pgn_path}")

    game_strings = []
    with open(pgn_path, 'r') as pgn_file:
        current_game = []
        while len(game_strings) < max_games:
            line = pgn_file.readline()
            if not line:
                if current_game:
                    game_strings.append("".join(current_game))
                break

            if line.startswith("[Event ") and current_game:
                game_strings.append("".join(current_game))
                current_game = [line]
            else:
                current_game.append(line)

    print(f"Read {len(game_strings)} games from file, dispatching to worker pool...")

    with concurrent.futures.ProcessPoolExecutor() as executor:
        results = executor.map(process_game_text, game_strings)
        for i, (res_tensors, res_policies, res_values) in enumerate(results):
            if res_tensors is not None:
                tensors.append(res_tensors)
                policies.append(res_policies)
                values.append(res_values)
            if (i + 1) % 100 == 0:
                print(f"Parsed {i + 1} games...")

    if tensors:
        tensors = torch.cat(tensors, dim=0)
        policies = torch.cat(policies, dim=0)
        values = torch.cat(values, dim=0)

    return tensors, policies, values

def train_supervised(pgn_path="games.pgn", chunk_size=30000):
    if not os.path.exists(pgn_path):
        print(f"Error: Could not find {pgn_path}. Please download an uncompressed PGN database from lichess and place it here.")
        return

    # AWS Optimization: Force native CUDA for maximum performance
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    # AWS Optimization: Enable TF32 and CuDNN Benchmarking for Tensor Cores
    torch.backends.cudnn.benchmark = True
    torch.backends.cuda.matmul.allow_tf32 = True
    torch.backends.cudnn.allow_tf32 = True

    # Initialize fresh network
    model = AlphaZeroNet().to(device)
    optimizer = optim.Adam(model.parameters(), lr=0.001)
    
    # Enable Mixed Precision to double the speed
    scaler = torch.amp.GradScaler('cuda')

    total_games_processed = 0
    if os.path.exists("fenrir_checkpoint.pth") and os.path.exists("fenrir_checkpoint.json"):
        print("Found checkpoint! Resuming training...")
        model.load_state_dict(torch.load("fenrir_checkpoint.pth", map_location=device, weights_only=False))
        import json
        with open("fenrir_checkpoint.json", "r") as f:
            meta = json.load(f)
            total_games_processed = meta.get("total_games_processed", 0)
        print(f"Resuming from game {total_games_processed}...")

    print(f"Opening PGN file: {pgn_path}")

    with open(pgn_path, 'r') as pgn_file:
        current_game = []

        # Skip games we already processed
        if total_games_processed > 0:
            print(f"Skipping the first {total_games_processed} games in the file... (this may take a minute)")
            skipped = 0
            while True:
                line = pgn_file.readline()
                if not line: break
                if line.startswith("[Event "):
                    skipped += 1
                    if skipped > total_games_processed:
                        current_game = [line]
                        break
            print("Finished skipping!")

        while True:
            # Parse a chunk of data
            print(f"\n--- Parsing next {chunk_size} games (Total so far: {total_games_processed}) ---")

            game_strings = []
            while len(game_strings) < chunk_size:
                line = pgn_file.readline()
                if not line:
                    if current_game:
                        game_strings.append("".join(current_game))
                        current_game = []
                    break

                if line.startswith("[Event ") and current_game:
                    game_strings.append("".join(current_game))
                    current_game = [line]
                else:
                    current_game.append(line)

            if not game_strings:
                print("Reached end of PGN file!")
                break

            tensors = []
            policies = []
            values = []

            t0 = time.time()
            # Use ProcessPoolExecutor to leverage all CPU cores
            # IPC overhead is now minimal because workers return a single stacked tensor per game!
            with concurrent.futures.ProcessPoolExecutor() as executor:
                results = executor.map(process_game_text, game_strings)
                for res_tensors, res_policies, res_values in results:
                    if res_tensors is not None:
                        tensors.append(torch.from_numpy(res_tensors))
                        policies.append(torch.from_numpy(res_policies))
                        values.append(torch.from_numpy(res_values))

            games_parsed = len(game_strings)
            total_games_processed += games_parsed
            del game_strings

            if len(tensors) == 0:
                continue

            tensors = torch.cat(tensors, dim=0)
            policies = torch.cat(policies, dim=0)
            values = torch.cat(values, dim=0)

            print(f"Finished parsing chunk in {time.time()-t0:.2f}s ({tensors.size(0)} positions)")

            # AWS Optimization: Massive batch size to fully saturate the 24GB A10G GPU
            dataset = torch.utils.data.TensorDataset(tensors, policies, values)
            dataloader = DataLoader(dataset, batch_size=4096, shuffle=True, pin_memory=torch.cuda.is_available())

            print(f"Starting training on chunk ({len(dataset)} positions)...")
            model.train()

            # AWS Optimization: 1 epoch per chunk (since we have 94 Million games, multiple passes are mathematically redundant and waste time!)
            epochs_per_chunk = 1
            for epoch in range(epochs_per_chunk):
                total_loss = 0
                batches = 0
                for batch_idx, (b_tensors, b_policies, b_values) in enumerate(dataloader):
                    b_tensors = b_tensors.to(device, non_blocking=True)
                    b_policies = b_policies.to(device, non_blocking=True)
                    b_values = b_values.to(device, non_blocking=True)

                    optimizer.zero_grad(set_to_none=True)
                    
                    with torch.amp.autocast('cuda'):
                        pred_policies, pred_values = model(b_tensors)

                        # AWS Optimization: Native C++ Cross Entropy is 100x faster than manual one-hot scattering
                        policy_loss = F.cross_entropy(pred_policies, b_policies)
                        value_loss = F.mse_loss(pred_values, b_values)

                        loss = policy_loss + value_loss
                        
                    scaler.scale(loss).backward()
                    scaler.step(optimizer)
                    scaler.update()

                    total_loss += loss.item()
                    batches += 1

                print(f"  Epoch {epoch+1}/{epochs_per_chunk} | Avg Loss: {total_loss/max(1, batches):.4f}")

            # Save checkpoint after each chunk so progress isn't lost if stopped early!
            torch.save(model.state_dict(), "fenrir_checkpoint.pth")
            import json
            with open("fenrir_checkpoint.json", "w") as f:
                json.dump({"total_games_processed": total_games_processed}, f)
            print("Checkpoint saved to fenrir_checkpoint.pth")

            # Also export to ONNX periodically so you can test it in C++ without waiting for the whole run
            try:
                dummy_input = torch.randn(1, 14, 8, 8, device=device)
                torch.onnx.export(
                    model, dummy_input, "supervised_checkpoint.onnx",
                    export_params=True, opset_version=14, do_constant_folding=True,
                    input_names=['input'], output_names=['policy', 'value'],
                    dynamic_axes={'input': {0: 'batch_size'}, 'policy': {0: 'batch_size'}, 'value': {0: 'batch_size'}}
                )
                print("ONNX checkpoint saved to supervised_checkpoint.onnx")
            except Exception as e:
                print(f"Could not export ONNX checkpoint: {e}")

            # Explicitly free memory to prevent OOM when loading the next chunk
            del dataloader
            del dataset
            del tensors
            del policies
            del values

            import gc
            gc.collect()
            if torch.cuda.is_available():
                torch.cuda.empty_cache()

            # AWS Optimization: Removed the 5 million games limit so it parses the entire 86M game dataset

    print("Training complete! Exporting to supervised.onnx...")

    dummy_input = torch.randn(1, 14, 8, 8, device=device)
    torch.onnx.export(
        model,
        dummy_input,
        "supervised.onnx",
        export_params=True,
        opset_version=14,
        do_constant_folding=True,
        input_names=['input'],
        output_names=['policy', 'value'],
        dynamic_axes={'input': {0: 'batch_size'},
                      'policy': {0: 'batch_size'},
                      'value': {0: 'batch_size'}}
    )

    # Save PyTorch checkpoint
    torch.save(model.state_dict(), "fenrir.pth")
    print("Done! You can now copy supervised.onnx and fenrir.pth into your engine's onnx folder.")

if __name__ == "__main__":
    pgn = sys.argv[1] if len(sys.argv) > 1 else "games.pgn"
    train_supervised(pgn)
