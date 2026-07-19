import torch
import torch.nn as nn
import torch.nn.functional as F

class ResBlock(nn.Module):
    def __init__(self, channels):
        super().__init__()
        self.conv1 = nn.Conv2d(channels, channels, kernel_size=3, padding=1, bias=False)
        self.bn1 = nn.BatchNorm2d(channels)
        self.conv2 = nn.Conv2d(channels, channels, kernel_size=3, padding=1, bias=False)
        self.bn2 = nn.BatchNorm2d(channels)

    def forward(self, x):
        residual = x
        x = F.relu(self.bn1(self.conv1(x)))
        x = self.bn2(self.conv2(x))
        x += residual
        return F.relu(x)

def infer_value_channels(state_dict, default=3):
    """Read the value head width out of a checkpoint.

    Lets every tool load any checkpoint regardless of which value-head
    width it was trained with, so widening the head never orphans older
    checkpoints (they stay usable for arena comparisons forever).
    """
    weight = state_dict.get("value_conv.weight")
    return int(weight.shape[0]) if weight is not None else default


class AlphaZeroNet(nn.Module):
    def __init__(self, num_blocks=10, channels=256, value_channels=3):
        """value_channels defaults to 3 — the width everything has been
        trained with so far, so constructing AlphaZeroNet() is unchanged.
        Widening it (e.g. 32) grows the value head's capacity to rank
        degrees of winning; use scripts/migrate_value_head.py to carry
        existing weights across the change."""
        super().__init__()
        # Input: 14 channels (12 pieces, 1 color, 1 castling rights), 8x8 board
        self.conv_input = nn.Conv2d(14, channels, kernel_size=3, padding=1, bias=False)
        self.bn_input = nn.BatchNorm2d(channels)

        self.res_blocks = nn.ModuleList([ResBlock(channels) for _ in range(num_blocks)])

        # Policy Head: Expanded to 4672 for AlphaZero 73-plane spatial move encoding
        self.policy_conv = nn.Conv2d(channels, 32, kernel_size=1, bias=False)
        self.policy_bn = nn.BatchNorm2d(32)
        self.policy_fc = nn.Linear(32 * 8 * 8, 4672)

        # Value Head
        self.value_channels = value_channels
        self.value_conv = nn.Conv2d(channels, value_channels, kernel_size=1, bias=False)
        self.value_bn = nn.BatchNorm2d(value_channels)
        self.value_fc1 = nn.Linear(value_channels * 8 * 8, 256)
        self.value_fc2 = nn.Linear(256, 1)

    def forward(self, x):
        x = F.relu(self.bn_input(self.conv_input(x)))
        for block in self.res_blocks:
            x = block(x)

        # Policy Head
        p = F.relu(self.policy_bn(self.policy_conv(x)))
        p = p.view(p.size(0), -1)
        policy = self.policy_fc(p)

        # Value Head
        v = F.relu(self.value_bn(self.value_conv(x)))
        v = v.view(v.size(0), -1)
        v = F.relu(self.value_fc1(v))
        value = torch.tanh(self.value_fc2(v))

        return policy, value
