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
Arena: play two Fenrir models against each other and report the score.

Fenrir loads its weights from onnx/fenrir.onnx relative to its working
directory, so each side gets a private working directory containing its own
onnx/fenrir.onnx (and a copy of fenrir.cfg when present). Games are played
in color-swapped pairs from a shared randomized opening so neither side
benefits from opening luck.

Usage (from the repo root, LD_LIBRARY_PATH already set as for self-play):
  python3 scripts/arena.py --model-a checkpoints/fenrir_A.onnx \
                           --model-b checkpoints/fenrir_B.onnx \
                           --games 100 --nodes 800

Smoke test (fast, tiny searches, adjudicates early):
  python3 scripts/arena.py --model-a onnx/fenrir.onnx --model-b onnx/fenrir.onnx \
                           --games 2 --nodes 8 --max-plies 20
"""

import argparse
import os
import random
import shutil
import sys
import tempfile
from collections import Counter

import chess
import chess.engine
import chess.pgn  # module level: a function-local import would shadow `chess`


def make_engine_dir(base_dir, name, model_path, cfg_path):
    work_dir = os.path.join(base_dir, name)
    os.makedirs(os.path.join(work_dir, "onnx"))
    os.makedirs(os.path.join(work_dir, "logs"))
    shutil.copy(model_path, os.path.join(work_dir, "onnx", "fenrir.onnx"))
    if cfg_path and os.path.exists(cfg_path):
        shutil.copy(cfg_path, os.path.join(work_dir, "fenrir.cfg"))
    return work_dir


def random_opening(plies, seed):
    rng = random.Random(seed)
    board = chess.Board()
    for _ in range(plies):
        moves = list(board.legal_moves)
        if not moves:
            break
        board.push(rng.choice(moves))
    return board


PIECE_VALUES = {chess.PAWN: 1, chess.KNIGHT: 3, chess.BISHOP: 3, chess.ROOK: 5, chess.QUEEN: 9}


def material_balance(board):
    """White material minus black material, in pawns."""
    balance = 0
    for piece in board.piece_map().values():
        value = PIECE_VALUES.get(piece.piece_type, 0)
        balance += value if piece.color == chess.WHITE else -value
    return balance


def play_game(white, black, opening_board, nodes, max_plies):
    """Returns (white_result, final_board, termination); result 1.0/0.0/0.5.

    termination distinguishes a real checkmate from a technical win
    (forfeit) and from each drawing rule, so "did it actually mate?" is
    answerable from the match output.
    """
    board = opening_board.copy()
    limit = chess.engine.Limit(nodes=nodes)
    while not board.is_game_over(claim_draw=True) and board.ply() < max_plies:
        engine = white if board.turn == chess.WHITE else black
        try:
            result = engine.play(board, limit)
        except chess.engine.EngineError as e:
            # Engine produced an illegal/null move in a live position:
            # forfeit for the side to move.
            print(f"  engine error ({e}); forfeit for {'white' if board.turn else 'black'}")
            return (0.0 if board.turn == chess.WHITE else 1.0), board, "forfeit"
        if result.move is None or result.move not in board.legal_moves:
            return (0.0 if board.turn == chess.WHITE else 1.0), board, "forfeit"
        board.push(result.move)

    outcome = board.outcome(claim_draw=True)
    if outcome is None:
        return 0.5, board, "ply-cap"
    reason = outcome.termination.name.lower().replace("_", "-")
    if outcome.winner is None:
        return 0.5, board, reason
    return (1.0 if outcome.winner == chess.WHITE else 0.0), board, reason


def main():
    parser = argparse.ArgumentParser(description="Fenrir model-vs-model arena")
    parser.add_argument("--model-a", required=True, help="ONNX weights for side A")
    parser.add_argument("--model-b", required=True, help="ONNX weights for side B")
    parser.add_argument("--games", type=int, default=100, help="total games (rounded down to pairs)")
    parser.add_argument("--nodes", type=int, default=800, help="fixed simulations per move")
    parser.add_argument("--binary", default="bin/fenrir", help="engine binary path")
    parser.add_argument("--cfg", default="fenrir.cfg", help="tuning cfg copied to both engines")
    parser.add_argument("--max-plies", type=int, default=400, help="draw adjudication cap")
    parser.add_argument("--random-plies", type=int, default=4, help="shared random opening plies per pair")
    parser.add_argument("--seed", type=int, default=0, help="opening seed base")
    parser.add_argument("--pgn", help="append played games to this PGN file (review in any chess GUI)")
    args = parser.parse_args()

    binary = os.path.abspath(args.binary)
    if not os.path.exists(binary):
        sys.exit(f"engine binary not found: {binary}")

    pairs = max(1, args.games // 2)
    base_dir = tempfile.mkdtemp(prefix="fenrir_arena_")
    dir_a = make_engine_dir(base_dir, "A", args.model_a, args.cfg)
    dir_b = make_engine_dir(base_dir, "B", args.model_b, args.cfg)

    print(f"Arena: {args.model_a} (A) vs {args.model_b} (B)")
    print(f"{pairs * 2} games, {args.nodes} nodes/move, openings: {args.random_plies} random plies\n")

    engine_a = chess.engine.SimpleEngine.popen_uci(binary, cwd=dir_a)
    engine_b = chess.engine.SimpleEngine.popen_uci(binary, cwd=dir_b)

    score_a = 0.0
    wins_a = draws = wins_b = 0
    material_sum = 0
    terminations = Counter()
    try:
        for pair in range(pairs):
            opening = random_opening(args.random_plies, args.seed + pair)
            for a_is_white in (True, False):
                white, black = (engine_a, engine_b) if a_is_white else (engine_b, engine_a)
                white_result, final_board, termination = play_game(
                    white, black, opening, args.nodes, args.max_plies)
                terminations[termination] += 1
                a_result = white_result if a_is_white else 1.0 - white_result
                score_a += a_result
                if a_result == 1.0:
                    wins_a += 1
                elif a_result == 0.0:
                    wins_b += 1
                else:
                    draws += 1
                # Final material from A's perspective: distinguishes "can't
                # outplay the opponent" (near 0 in draws) from "wins material
                # but can't checkmate" (large + in draws).
                a_material = material_balance(final_board) * (1 if a_is_white else -1)
                material_sum += a_material
                game_no = pair * 2 + (1 if a_is_white else 2)
                if args.pgn:
                    game = chess.pgn.Game.from_board(final_board)
                    game.headers["Event"] = "Fenrir arena"
                    game.headers["White"] = "A" if a_is_white else "B"
                    game.headers["Black"] = "B" if a_is_white else "A"
                    game.headers["Result"] = "1-0" if white_result == 1.0 else "0-1" if white_result == 0.0 else "1/2-1/2"
                    with open(args.pgn, "a") as pgn_file:
                        print(game, file=pgn_file, end="\n\n")
                print(f"game {game_no:3d}: A as {'white' if a_is_white else 'black'} -> "
                      f"{'A wins' if a_result == 1.0 else 'B wins' if a_result == 0.0 else 'draw':7s}"
                      f"  {termination:<18} material A{a_material:+3d}"
                      f"   [A {wins_a} / D {draws} / B {wins_b}]")
    finally:
        engine_a.quit()
        engine_b.quit()
        shutil.rmtree(base_dir, ignore_errors=True)

    total = wins_a + draws + wins_b
    print()
    print("how games ended:")
    for reason, count in terminations.most_common():
        print(f"  {reason:<20}{count:>4}  ({100.0 * count / total:.0f}%)")
    if terminations.get("forfeit"):
        print("  !! forfeits are illegal/null engine moves, not chess wins — investigate")
    print(f"\naverage final material from A's perspective: {material_sum / total:+.1f}")
    print("  (near 0 once games end decisively: material at a checkmate is incidental,")
    print("   unlike an unconverted win that parks at +15)")
    pct = 100.0 * score_a / total
    # Two-sigma band on the score percentage, treating each game as a trial.
    p = score_a / total
    sigma = (p * (1.0 - p) / total) ** 0.5
    print(f"\n=== RESULT: A scores {score_a:.1f}/{total} = {pct:.1f}%  (+/- {200.0 * sigma:.1f}% at 2-sigma)")
    print(f"    A wins {wins_a}, draws {draws}, B wins {wins_b}")
    if pct - 200.0 * sigma > 50.0:
        print("    A is significantly stronger.")
    elif pct + 200.0 * sigma < 50.0:
        print("    B is significantly stronger.")
    else:
        print("    No significant difference at this sample size.")


if __name__ == "__main__":
    main()
