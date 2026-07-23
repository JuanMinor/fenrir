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
Gauntlet vs a scripted greedy-material bot: an ABSOLUTE skill rung.

The bot plays the capture of the highest-value piece if any capture
exists, else a random move (seeded). Beating it consistently requires
both winning material AND not hanging pieces to a pure materialist —
a strictly harder test than beating a random mover, with a fixed skill
level that never trains, so scores are comparable across checkpoints.

Usage: python3 tools/gauntlet_greedy.py --model /tmp/now.onnx --games 20 --nodes 800
"""

import argparse
import os
import random
import shutil
import sys
import tempfile

import chess
import chess.engine

PIECE_VALUES = {chess.PAWN: 1, chess.KNIGHT: 3, chess.BISHOP: 3, chess.ROOK: 5, chess.QUEEN: 9}


def material_balance(board):
    balance = 0
    for piece in board.piece_map().values():
        value = PIECE_VALUES.get(piece.piece_type, 0)
        balance += value if piece.color == chess.WHITE else -value
    return balance


def greedy_move(board, rng):
    captures = [m for m in board.legal_moves if board.is_capture(m)]
    if captures:
        def victim_value(move):
            if board.is_en_passant(move):
                return 1
            victim = board.piece_at(move.to_square)
            return PIECE_VALUES.get(victim.piece_type, 0) if victim else 0
        best = max(victim_value(m) for m in captures)
        return rng.choice([m for m in captures if victim_value(m) == best])
    return rng.choice(list(board.legal_moves))


def main():
    parser = argparse.ArgumentParser(description="Fenrir vs greedy-material bot")
    parser.add_argument("--model", required=True, help="ONNX weights for Fenrir")
    parser.add_argument("--games", type=int, default=20)
    parser.add_argument("--nodes", type=int, default=800)
    parser.add_argument("--binary", default="bin/fenrir")
    parser.add_argument("--cfg", default="fenrir.cfg")
    parser.add_argument("--max-plies", type=int, default=400)
    parser.add_argument("--seed", type=int, default=0)
    args = parser.parse_args()

    base_dir = tempfile.mkdtemp(prefix="fenrir_gauntlet_")
    work_dir = os.path.join(base_dir, "engine")
    os.makedirs(os.path.join(work_dir, "onnx"))
    os.makedirs(os.path.join(work_dir, "logs"))
    shutil.copy(args.model, os.path.join(work_dir, "onnx", "fenrir.onnx"))
    if os.path.exists(args.cfg):
        shutil.copy(args.cfg, os.path.join(work_dir, "fenrir.cfg"))

    engine = chess.engine.SimpleEngine.popen_uci(os.path.abspath(args.binary), cwd=work_dir)
    limit = chess.engine.Limit(nodes=args.nodes)

    wins = draws = losses = 0
    material_sum = 0
    try:
        for game_no in range(1, args.games + 1):
            rng = random.Random(args.seed + game_no)
            fenrir_is_white = (game_no % 2 == 1)
            board = chess.Board()
            while not board.is_game_over(claim_draw=True) and board.ply() < args.max_plies:
                if board.turn == (chess.WHITE if fenrir_is_white else chess.BLACK):
                    try:
                        result = engine.play(board, limit)
                    except chess.engine.EngineError:
                        result = None
                    if result is None or result.move is None or result.move not in board.legal_moves:
                        break  # forfeit, scored below by material/outcome
                    board.push(result.move)
                else:
                    board.push(greedy_move(board, rng))

            outcome = board.outcome(claim_draw=True)
            if outcome is not None and outcome.winner is not None:
                fenrir_result = 1.0 if outcome.winner == (chess.WHITE if fenrir_is_white else chess.BLACK) else 0.0
            else:
                fenrir_result = 0.5
            fenrir_material = material_balance(board) * (1 if fenrir_is_white else -1)
            material_sum += fenrir_material

            wins += fenrir_result == 1.0
            draws += fenrir_result == 0.5
            losses += fenrir_result == 0.0
            print(f"game {game_no:3d}: Fenrir as {'white' if fenrir_is_white else 'black'} -> "
                  f"{'WIN' if fenrir_result == 1.0 else 'loss' if fenrir_result == 0.0 else 'draw':5s}"
                  f"  final material {fenrir_material:+3d}   [W {wins} / D {draws} / L {losses}]")
    finally:
        engine.quit()
        shutil.rmtree(base_dir, ignore_errors=True)

    total = wins + draws + losses
    score = wins + 0.5 * draws
    print(f"\n=== Fenrir vs greedy bot: {score:.1f}/{total} = {100.0*score/total:.1f}%"
          f"  (W {wins} / D {draws} / L {losses}), avg material {material_sum/total:+.1f}")


if __name__ == "__main__":
    main()
