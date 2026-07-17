#!/usr/bin/env python3
"""Probe a Fenrir checkpoint's value head directly on canned positions.

Bypasses the search entirely: encodes a handful of positions with known
material situations exactly as train.py does, runs the raw network, and
prints the tanh value (side-to-move perspective, -1 lost .. +1 won).
A value head that has learned material shows strong correct signs on the
lopsided positions; one stuck at ~0 everywhere has learned nothing.

Usage: python3 scripts/probe_value.py <ckpt1.pth> [ckpt2.pth ...]
"""

import os
import sys

import torch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "training"))
from model import AlphaZeroNet  # noqa: E402
from train import ChessDataset  # noqa: E402

POSITIONS = [
    ("start, white to move (expect ~0)", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
    ("white +4Q, WHITE to move (expect >> 0)", "k7/8/8/8/8/8/8/QQQQK3 w - - 0 1"),
    ("white +4Q, BLACK to move (expect << 0)", "k7/8/8/8/8/8/8/QQQQK3 b - - 0 1"),
    ("black +4Q, WHITE to move (expect << 0)", "K7/8/8/8/8/8/8/qqqqk3 w - - 0 1"),
    ("black +4Q, BLACK to move (expect >> 0)", "K7/8/8/8/8/8/8/qqqqk3 b - - 0 1"),
    ("white +1R midgame, WHITE to move (expect > 0)", "1k1r4/pp3ppp/8/8/8/8/PP3PPP/1K1R3R w - - 0 1"),
]


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)

    encoder = ChessDataset("__no_data_dir__")  # only used for fen_to_tensor

    models = []
    for path in sys.argv[1:]:
        model = AlphaZeroNet()
        model.load_state_dict(torch.load(path, map_location="cpu", weights_only=True))
        model.eval()
        models.append((os.path.basename(path), model))

    header = f"{'position':<44}" + "".join(f"{name[:18]:>20}" for name, _ in models)
    print(header)
    print("-" * len(header))
    with torch.no_grad():
        for label, fen in POSITIONS:
            tensor = encoder.fen_to_tensor(fen).unsqueeze(0)
            row = f"{label:<44}"
            for _, model in models:
                _, value = model(tensor)
                row += f"{value.item():>20.4f}"
            print(row)


if __name__ == "__main__":
    main()
