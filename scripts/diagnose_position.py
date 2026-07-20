#!/usr/bin/env python3
#   Copyright (c) 2026 Juan Minor
#   GPL-3.0-or-later (see repository LICENSE)
"""
Diagnose why the engine chose a specific move at a specific position --
built to investigate the arena-match stalemate finding (a crushing material
lead thrown away by walking into one of only 2 stalemating moves out of 55
legal options), but works for any single move at any position.

Two independent checks, either or both:

  --pth CHECKPOINT   raw network only (no search): lists every legal move
                      with its policy prior and the value head's read of
                      the resulting position, flagging any move that
                      stalemates the opponent. Tells you whether the value
                      head recognizes the trap when asked directly.

  --model ONNX --nodes N [N ...]
                      the real engine via UCI, once per node count. Tells
                      you whether more search budget alone avoids the trap
                      (same test that explained the back-rank-mate-in-1
                      result needing 50000 nodes in tactics.py).

Usage:
  python3 scripts/diagnose_position.py \
      --fen "6n1/2p5/p1bp4/3k4/7q/8/r1r5/1n3K2 b - - 1 36" \
      --pth onnx/fenrir.pth \
      --model onnx/fenrir.onnx --nodes 800 5000 20000 50000
"""

import argparse
import os
import shutil
import sys
import tempfile

import chess
import chess.engine


def stalemating_moves(board):
    result = set()
    for mv in board.legal_moves:
        b2 = board.copy()
        b2.push(mv)
        if b2.is_stalemate():
            result.add(mv.uci())
    return result


def run_pth_check(fen, pth_path):
    import torch

    sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "training"))
    from model import AlphaZeroNet, infer_value_channels
    from train import ChessDataset, uci_to_index

    state = torch.load(pth_path, map_location="cpu", weights_only=True)
    net = AlphaZeroNet(value_channels=infer_value_channels(state))
    net.load_state_dict(state)
    net.eval()
    encoder = ChessDataset("__no_data_dir__")

    board = chess.Board(fen)
    turn = fen.split(" ")[1]
    traps = stalemating_moves(board)

    with torch.no_grad():
        logits, _ = net(encoder.fen_to_tensor(fen).unsqueeze(0))
        probs = torch.softmax(logits[0], dim=0)

        rows = []
        for mv in board.legal_moves:
            uci = mv.uci()
            prior = probs[uci_to_index(uci, turn)].item()
            b2 = board.copy()
            b2.push(mv)
            if b2.is_game_over():
                resulting_value = None
            else:
                next_fen = b2.fen()
                _, v = net(encoder.fen_to_tensor(next_fen).unsqueeze(0))
                resulting_value = v.item()
            rows.append((uci, prior, resulting_value, uci in traps))

    rows.sort(key=lambda r: -r[1])
    print(f"{'move':<8} {'prior':>8} {'value after (mover persp.)':>28}  flag")
    print("-" * 60)
    for uci, prior, val, is_trap in rows[:15]:
        val_str = f"{val:+.4f}" if val is not None else "  (terminal)"
        flag = "  <-- STALEMATES OPPONENT" if is_trap else ""
        print(f"{uci:<8} {prior:>8.4f} {val_str:>28}{flag}")
    if len(rows) > 15:
        print(f"... ({len(rows) - 15} more legal moves not shown)")
    print(f"\n{len(traps)} of {len(rows)} legal moves stalemate the opponent: {sorted(traps)}")


def run_engine_check(fen, model_path, binary, cfg, node_counts):
    base_dir = tempfile.mkdtemp(prefix="fenrir_diag_")
    work_dir = os.path.join(base_dir, "engine")
    os.makedirs(os.path.join(work_dir, "onnx"))
    os.makedirs(os.path.join(work_dir, "logs"))
    shutil.copy(model_path, os.path.join(work_dir, "onnx", "fenrir.onnx"))
    if cfg and os.path.exists(cfg):
        shutil.copy(cfg, os.path.join(work_dir, "fenrir.cfg"))

    board = chess.Board(fen)
    traps = stalemating_moves(board)

    engine = chess.engine.SimpleEngine.popen_uci(os.path.abspath(binary), cwd=work_dir)
    try:
        print(f"{'nodes':>8} {'played':>8}  verdict")
        print("-" * 40)
        for nodes in node_counts:
            result = engine.play(board.copy(), chess.engine.Limit(nodes=nodes))
            played = result.move.uci() if result.move else "none"
            verdict = "STALEMATE TRAP" if played in traps else "safe"
            print(f"{nodes:>8} {played:>8}  {verdict}")
    finally:
        engine.quit()
        shutil.rmtree(base_dir, ignore_errors=True)


def main():
    parser = argparse.ArgumentParser(description="Diagnose one move at one position")
    parser.add_argument("--fen", required=True)
    parser.add_argument("--pth", help="raw value/policy dump over every legal move")
    parser.add_argument("--model", help="ONNX weights: run the real engine via UCI")
    parser.add_argument("--nodes", type=int, nargs="+", default=[800, 5000, 20000, 50000])
    parser.add_argument("--binary", default="bin/fenrir")
    parser.add_argument("--cfg", default="fenrir.cfg")
    args = parser.parse_args()

    if not args.pth and not args.model:
        sys.exit("pass --pth and/or --model")

    if args.pth:
        print("=== raw policy/value over every legal move ===")
        run_pth_check(args.fen, args.pth)
        print()

    if args.model:
        print("=== engine move choice vs node budget ===")
        run_engine_check(args.fen, args.model, args.binary, args.cfg, args.nodes)


if __name__ == "__main__":
    main()
