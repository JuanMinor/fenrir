import os
import json
import time
import torch
import torch.optim as optim
import torch.nn.functional as F
from torch.utils.data import Dataset, DataLoader
from model import AlphaZeroNet

# Map algebraic moves to AlphaZero 4672-channel actions
# Takes the active turn side ('w' or 'b') to properly map string castling text
def uci_to_index(uci_move, turn='w'):
    uci_move = uci_move.strip()

    # 1. Catch and translate standard python-chess SAN castling text
    if uci_move == "O-O":
        uci_move = "e1g1" if turn == 'w' else "e8g8"
    elif uci_move == "O-O-O":
        uci_move = "e1c1" if turn == 'w' else "e8c8"

    # Absolute safety catch for any other malformed short strings
    if len(uci_move) < 4:
        return 0

    from_file = ord(uci_move[0]) - ord('a')
    from_rank = int(uci_move[1]) - 1
    to_file = ord(uci_move[2]) - ord('a')
    to_rank = int(uci_move[3]) - 1

    from_sq = from_rank * 8 + from_file
    dx = to_file - from_file
    dy = to_rank - from_rank

    promo = uci_move[4].lower() if len(uci_move) > 4 else None
    channel = 0

    # 1. Underpromotions (Knight, Bishop, Rook)
    if promo and promo != 'q':
        promo_dir = dx + 1
        promo_type = {'n': 0, 'b': 1, 'r': 2}[promo]
        channel = 64 + (promo_dir * 3) + promo_type

    # 2. Knight moves
    elif abs(dx) * abs(dy) == 2:
        knight_lookups = [(1,2), (2,1), (2,-1), (1,-2), (-1,-2), (-2,-1), (-2,1), (-1,2)]
        channel = 56 + knight_lookups.index((dx, dy))

    # 3. Queen-like moves (Straight, Diagonal, Queen Promotions)
    else:
        step_x = 0 if dx == 0 else (1 if dx > 0 else -1)
        step_y = 0 if dy == 0 else (1 if dy > 0 else -1)
        distance = max(abs(dx), abs(dy))

        dirs = [(0,1), (1,1), (1,0), (1,-1), (0,-1), (-1,-1), (-1,0), (-1,1)]
        dir_idx = dirs.index((step_x, step_y))
        channel = (dir_idx * 7) + (distance - 1)

    return (from_sq * 73) + channel

class ChessDataset(Dataset):
    def __init__(self, data_dir):
        self.data_dir = data_dir
        self.samples = []
        self.buffer = self.samples
        self.load_data()

    def update(self):
        new_files_count = 0
        if not os.path.exists(self.data_dir):
            return 0

        for filename in os.listdir(self.data_dir):
            if filename.startswith("READY_") and filename.endswith(".jsonl"):
                filepath = os.path.join(self.data_dir, filename)
                try:
                    with open(filepath, 'r') as f:
                        for line in f:
                            line = line.strip()
                            if not line:
                                continue
                            try:
                                data = json.loads(line)
                                import chess
                                chess.Board(data['fen']) # Validate FEN structure
                                self.buffer.append(data)
                            except (json.JSONDecodeError, ValueError):
                                pass # Skip partially flushed lines or corrupted FENs
                    os.remove(filepath)
                    new_files_count += 1
                except Exception as e:
                    print(f"Error reading {filename}: {e}")
                    try:
                        os.remove(filepath)
                    except:
                        pass

        # Keep only the latest 100,000 samples (replay buffer)
        if len(self.samples) > 100000:
            self.samples = self.samples[-100000:]

        return new_files_count

    def load_data(self):
        self.update()

    def fen_to_tensor(self, fen):
        import chess
        board = chess.Board(fen)
        tensor = torch.zeros((14, 8, 8), dtype=torch.float32)

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

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, idx):
        sample = self.samples[idx]
        tensor = self.fen_to_tensor(sample['fen'])

        # Set output tensor structure to AlphaZero standard 4,672
        policy = torch.zeros(4672, dtype=torch.float32)

        # Safely extract the current active turn from the FEN string to guide the castling parser
        fen_parts = sample['fen'].split(' ')
        turn_color = fen_parts[1] if len(fen_parts) > 1 else 'w'

        for move, prob in sample['policy'].items():
            policy[uci_to_index(move, turn_color)] = prob

        value = torch.tensor([sample['result']], dtype=torch.float32)
        # Normalize result from [0, 1] to [-1, 1] for tanh
        value = (value * 2) - 1

        return tensor, policy, value

def train():
    try:
        import torch_directml
        has_dml = True
    except ImportError:
        has_dml = False

    if has_dml and torch_directml.is_available():
        device = torch_directml.device()
    else:
        device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    print(f"Using device: {device}")

    model = AlphaZeroNet().to(device)
    optimizer = optim.Adam(model.parameters(), lr=0.001)

    # Check for PyTorch checkpoint to resume training
    checkpoint_path = "onnx/fenrir.pth"
    if os.path.exists(checkpoint_path):
        print(f"Loading existing weights from {checkpoint_path}...")
        model.load_state_dict(torch.load(checkpoint_path, weights_only=True))

    # Path logic mirroring C++ output
    data_dir = "data/selfplay/"

    print(f"Watching directory for data: {data_dir}")

    dataset = ChessDataset(data_dir)

    while True:
        new_files = dataset.update()
        if len(dataset) < 4096 or new_files == 0:
            print(f"Waiting for new data... Buffer size: {len(dataset)}. New files: {new_files}")
            time.sleep(1)
            continue

        dataloader = DataLoader(dataset, batch_size=4096, shuffle=True)

        model.train()
        total_loss = 0
        batches_processed = 0

        # Prevent massive overfitting by doing a maximum of 50 random batches per update
        for batch_idx, (tensors, policies, values) in enumerate(dataloader):
            if batches_processed >= 50:
                break

            tensors, policies, values = tensors.to(device), policies.to(device), values.to(device)

            optimizer.zero_grad()

            pred_policies, pred_values = model(tensors)

            policy_loss = -torch.sum(policies * F.log_softmax(pred_policies, dim=1), dim=1).mean()
            value_loss = F.mse_loss(pred_values, values)

            loss = policy_loss + value_loss
            loss.backward()
            optimizer.step()

            total_loss += loss.item()
            batches_processed += 1

        avg_loss = total_loss / max(1, batches_processed)
        print(f"Training cycle finished. Loss: {avg_loss:.4f} (over {batches_processed} batches)")

        # Export back to ONNX for C++ Engine atomically
        dummy_input = torch.randn(1, 14, 8, 8, device=device)
        onnx_tmp_path = "onnx/fenrir.onnx.tmp"
        onnx_path = "onnx/fenrir.onnx"
        torch.onnx.export(
            model,
            dummy_input,
            onnx_tmp_path,
            export_params=True,
            opset_version=14,
            do_constant_folding=True,
            input_names=['input'],
            output_names=['policy', 'value'],
            dynamic_axes={'input': {0: 'batch_size'},
                          'policy': {0: 'batch_size'},
                          'value': {0: 'batch_size'}}
        )
        while True:
            try:
                os.replace(onnx_tmp_path, onnx_path)
                break
            except PermissionError:
                time.sleep(0.01)

        # Save PyTorch checkpoint for resuming
        torch.save(model.state_dict(), "onnx/fenrir.pth")
        print(f"Exported updated weights to {onnx_path} and onnx/fenrir.pth")


if __name__ == "__main__":
    train()
