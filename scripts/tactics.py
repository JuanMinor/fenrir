#!/usr/bin/env python3
#   Copyright (c) 2026 Juan Minor
#   GPL-3.0-or-later (see repository LICENSE)
"""
Tactics exam: absolute chess-skill measurement with known-correct answers.

Two modes, measuring two different things:
  --model X.onnx      "Fenrir the player": full engine via UCI at --nodes,
                      scored on whether it plays an accepted move.
  --pth X.pth         "Fenrir the intuition": raw policy network only, no
                      search — are accepted moves in the net's top-1/top-3?

Positions are deliberately elementary (mate-in-1, win-the-hanging-piece)
so solutions are unambiguous. Solve-rate across checkpoints is an
objective learning curve, unlike arena scores which are only relative.

Usage:
  python3 scripts/tactics.py --model /tmp/now.onnx --nodes 800
  python3 scripts/tactics.py --pth checkpoints/<latest>.pth
"""

import argparse
import os
import shutil
import sys
import tempfile

import chess
import chess.engine

# (name, fen, accepted UCI moves)
POSITIONS = [
    ("back-rank mate in 1 (w)", "6k1/5ppp/8/8/8/8/5PPP/R5K1 w - - 0 1", ["a1a8"]),
    ("queen mate in 1 (w)", "6k1/5ppp/8/8/8/8/5PPP/3Q2K1 w - - 0 1", ["d1d8"]),
    ("rook mate in 1, cornered king (w)", "k7/pp6/8/8/8/8/8/K6R w - - 0 1", ["h1h8"]),
    ("rook ladder mate in 1 (w)", "7k/R7/1R6/8/8/8/8/7K w - - 0 1", ["b6b8"]),
    ("take the hanging queen, rook (w)", "k7/8/8/3q4/8/8/3R4/K7 w - - 0 1", ["d2d5"]),
    ("take the hanging queen, knight (w)", "k7/8/8/3q4/5N2/8/8/K7 w - - 0 1", ["f4d5"]),
    ("take the rook, not the pawn (w)", "k7/8/8/1r3p2/8/3B4/8/K7 w - - 0 1", ["d3b5"]),
    ("queen mate in 1 (b)", "3q2k1/5ppp/8/8/8/8/5PPP/6K1 b - - 0 1", ["d8d1"]),
    ("rook mate in 1, cornered king (b)", "k6r/8/8/8/8/8/PP6/K7 b - - 0 1", ["h8h1"]),
    ("take the hanging queen, rook (b)", "K7/8/8/3Q4/8/8/3r4/k7 b - - 0 1", ["d2d5"]),
]


def run_engine_mode(model_path, binary, cfg, nodes):
    base_dir = tempfile.mkdtemp(prefix="fenrir_tactics_")
    work_dir = os.path.join(base_dir, "engine")
    os.makedirs(os.path.join(work_dir, "onnx"))
    os.makedirs(os.path.join(work_dir, "logs"))
    shutil.copy(model_path, os.path.join(work_dir, "onnx", "fenrir.onnx"))
    if cfg and os.path.exists(cfg):
        shutil.copy(cfg, os.path.join(work_dir, "fenrir.cfg"))

    engine = chess.engine.SimpleEngine.popen_uci(os.path.abspath(binary), cwd=work_dir)
    solved = 0
    try:
        print(f"{'position':<40} {'played':>8} {'verdict':>9}")
        print("-" * 60)
        for name, fen, accepted in POSITIONS:
            board = chess.Board(fen)
            result = engine.play(board, chess.engine.Limit(nodes=nodes))
            played = result.move.uci() if result.move else "none"
            ok = played in accepted
            solved += ok
            print(f"{name:<40} {played:>8} {'SOLVED' if ok else 'missed':>9}")
    finally:
        engine.quit()
        shutil.rmtree(base_dir, ignore_errors=True)
    print(f"\nengine solve rate: {solved}/{len(POSITIONS)} at {nodes} nodes")


def run_intuition_mode(pth_path):
    import torch

    sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "training"))
    from model import AlphaZeroNet, infer_value_channels
    from train import ChessDataset, uci_to_index

    state = torch.load(pth_path, map_location="cpu", weights_only=True)
    net = AlphaZeroNet(value_channels=infer_value_channels(state))
    net.load_state_dict(state)
    net.eval()
    encoder = ChessDataset("__no_data_dir__")

    top1 = top3 = 0
    print(f"{'position':<40} {'top-3 policy moves':>28} {'verdict':>9}")
    print("-" * 80)
    with torch.no_grad():
        for name, fen, accepted in POSITIONS:
            board = chess.Board(fen)
            turn = fen.split(" ")[1]
            logits, _ = net(encoder.fen_to_tensor(fen).unsqueeze(0))
            legal = [(m.uci(), logits[0][uci_to_index(m.uci(), turn)].item())
                     for m in board.legal_moves]
            legal.sort(key=lambda x: -x[1])
            ranked = [u for u, _ in legal]
            hit1 = ranked[0] in accepted
            hit3 = any(u in accepted for u in ranked[:3])
            top1 += hit1
            top3 += hit3
            print(f"{name:<40} {' '.join(ranked[:3]):>28} "
                  f"{'TOP-1' if hit1 else 'top-3' if hit3 else 'missed':>9}")
    n = len(POSITIONS)
    print(f"\nintuition: top-1 {top1}/{n}, top-3 {top3}/{n} (no search)")


def main():
    parser = argparse.ArgumentParser(description="Fenrir tactics exam")
    parser.add_argument("--model", help="ONNX weights: full-engine mode via UCI")
    parser.add_argument("--pth", help="PyTorch checkpoint: raw-policy intuition mode")
    parser.add_argument("--nodes", type=int, default=800)
    parser.add_argument("--binary", default="bin/fenrir")
    parser.add_argument("--cfg", default="fenrir.cfg")
    args = parser.parse_args()

    if not args.model and not args.pth:
        sys.exit("pass --model <onnx> and/or --pth <checkpoint>")
    if args.model:
        run_engine_mode(args.model, args.binary, args.cfg, args.nodes)
    if args.pth:
        run_intuition_mode(args.pth)


if __name__ == "__main__":
    main()
