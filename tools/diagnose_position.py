#!/usr/bin/env python3
#   Copyright (c) 2026 Juan Minor
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
  python3 tools/diagnose_position.py \
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


def run_engine_check(fens, model_path, binary, cfg, node_counts):
    """fens: list of FEN strings. One engine process, reused across all of
    them and all node counts -- much faster than relaunching per position."""
    base_dir = tempfile.mkdtemp(prefix="fenrir_diag_")
    work_dir = os.path.join(base_dir, "engine")
    os.makedirs(os.path.join(work_dir, "onnx"))
    os.makedirs(os.path.join(work_dir, "logs"))
    shutil.copy(model_path, os.path.join(work_dir, "onnx", "fenrir.onnx"))
    if cfg and os.path.exists(cfg):
        shutil.copy(cfg, os.path.join(work_dir, "fenrir.cfg"))

    engine = chess.engine.SimpleEngine.popen_uci(os.path.abspath(binary), cwd=work_dir)
    results = []
    try:
        header = f"{'#':>3} {'nodes':>8} {'played':>8}  verdict"
        print(header)
        print("-" * len(header))
        for i, fen in enumerate(fens, 1):
            board = chess.Board(fen)
            traps = stalemating_moves(board)
            for nodes in node_counts:
                result = engine.play(board.copy(), chess.engine.Limit(nodes=nodes))
                played = result.move.uci() if result.move else "none"
                is_trap = played in traps
                verdict = "STALEMATE TRAP" if is_trap else "safe"
                print(f"{i:>3} {nodes:>8} {played:>8}  {verdict}")
                results.append((i, fen, nodes, played, is_trap))
    finally:
        engine.quit()
        shutil.rmtree(base_dir, ignore_errors=True)
    return results


def main():
    parser = argparse.ArgumentParser(description="Diagnose one or many positions")
    parser.add_argument("--fen", help="single FEN")
    parser.add_argument("--fen-file", help="file with one FEN per line, for batch testing")
    parser.add_argument("--pth", help="raw value/policy dump over every legal move "
                                       "(single --fen only)")
    parser.add_argument("--model", help="ONNX weights: run the real engine via UCI")
    parser.add_argument("--nodes", type=int, nargs="+", default=[800, 5000, 20000, 50000])
    parser.add_argument("--binary", default="bin/fenrir")
    parser.add_argument("--cfg", default="fenrir.cfg")
    args = parser.parse_args()

    if not args.fen and not args.fen_file:
        sys.exit("pass --fen or --fen-file")
    if not args.pth and not args.model:
        sys.exit("pass --pth and/or --model")

    fens = [args.fen] if args.fen else [
        line.strip() for line in open(args.fen_file) if line.strip() and not line.startswith("#")
    ]

    if args.pth:
        if len(fens) > 1:
            sys.exit("--pth is single-position only; use --fen, not --fen-file, with it")
        print("=== raw policy/value over every legal move ===")
        run_pth_check(fens[0], args.pth)
        print()

    if args.model:
        print(f"=== engine move choice vs node budget ({len(fens)} position(s)) ===")
        results = run_engine_check(fens, args.model, args.binary, args.cfg, args.nodes)
        if len(fens) > 1:
            traps_hit = sum(1 for *_, is_trap in results if is_trap)
            positions_ever_trapped = len({i for i, *_, is_trap in results if is_trap})
            print(f"\n{traps_hit} of {len(results)} (position, node-count) pairs still walked "
                  f"into a trap; {positions_ever_trapped} of {len(fens)} positions failed at "
                  f"at least one tested node count")


if __name__ == "__main__":
    main()
