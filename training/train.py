import os
import json
import time
import torch
import torch.optim as optim
import torch.nn.functional as F
from torch.utils.data import Dataset, DataLoader
from model import AlphaZeroNet

# Stub: Map algebraic moves to indices 0-4095
def uci_to_index(uci_move):
    return hash(uci_move) % 4096

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
            if filename.endswith(".jsonl"):
                filepath = os.path.join(self.data_dir, filename)
                try:
                    with open(filepath, 'r') as f:
                        for line in f:
                            if not line.strip():
                                continue
                            data = json.loads(line)
                            self.buffer.append(data)
                    os.remove(filepath)
                    new_files_count += 1
                except Exception as e:
                    print(f"Error reading {filename}: {e}")
        
        # Keep only the latest 100,000 samples (replay buffer)
        if len(self.samples) > 100000:
            self.samples = self.samples[-100000:]
        
        return new_files_count
                            
    def load_data(self):
        self.update()
                            
    def fen_to_tensor(self, fen):
        # Stub: parse FEN to 14x8x8 tensor matching C++ engine representation
        # Consider using `python-chess` library to parse FEN directly into bitboards
        return torch.zeros((14, 8, 8), dtype=torch.float32)

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, idx):
        sample = self.samples[idx]
        tensor = self.fen_to_tensor(sample['fen'])
        
        policy = torch.zeros(4096, dtype=torch.float32)
        for move, prob in sample['policy'].items():
            policy[uci_to_index(move)] = prob
            
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
    
    # Path logic mirroring C++ output
    data_dir = "data/selfplay/"
    
    print(f"Watching directory for data: {data_dir}")
    
    dataset = ChessDataset(data_dir)
    
    while True:
        new_files = dataset.update()
        if len(dataset) < 32 or new_files == 0:
            print(f"Waiting for new data... Buffer size: {len(dataset)}. New files: {new_files}")
            time.sleep(1)
            continue
            
        dataloader = DataLoader(dataset, batch_size=32, shuffle=True)
        
        model.train()
        total_loss = 0
        for batch_idx, (tensors, policies, values) in enumerate(dataloader):
            tensors, policies, values = tensors.to(device), policies.to(device), values.to(device)
            
            optimizer.zero_grad()
            
            pred_policies, pred_values = model(tensors)
            
            # Policy loss (Cross Entropy) and Value loss (MSE)
            # Softmax is applied intrinsically by cross_entropy if targets are probabilities? 
            # Note: PyTorch cross_entropy with probabilities requires careful handling, 
            # or manual log_softmax + NLL_loss. We'll use cross_entropy natively assuming logits.
            policy_loss = -torch.sum(policies * F.log_softmax(pred_policies, dim=1), dim=1).mean()
            value_loss = F.mse_loss(pred_values, values)
            
            loss = policy_loss + value_loss
            loss.backward()
            optimizer.step()
            
            total_loss += loss.item()
            
        print(f"Epoch finished. Loss: {total_loss/len(dataloader):.4f}")
        
        # Export back to ONNX for C++ Engine atomically
        dummy_input = torch.randn(1, 14, 8, 8, device=device)
        onnx_tmp_path = "onnx/fenrir.onnx.tmp"
        onnx_path = "onnx/fenrir.onnx"
        torch.onnx.export(model, dummy_input, onnx_tmp_path, 
                         input_names=['input'], output_names=['policy', 'value'],
                         dynamic_axes={'input': {0: 'batch_size'},
                                       'policy': {0: 'batch_size'},
                                       'value': {0: 'batch_size'}})
        while True:
            try:
                os.replace(onnx_tmp_path, onnx_path)
                break
            except PermissionError:
                time.sleep(0.01)
        print(f"Exported updated weights to {onnx_path}")


if __name__ == "__main__":
    train()
