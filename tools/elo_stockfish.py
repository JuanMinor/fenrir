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
Estimate Fenrir's Elo by playing it against Stockfish at one or more
calibrated strength settings (UCI_LimitStrength + UCI_Elo), which Stockfish
itself is tuned to honor regardless of how much time/nodes it's given.

Pass several --elo values to sweep and find the crossover where Fenrir's
score is ~50% -- that crossover is a much steadier estimate than a single
match's logistic-formula conversion, which gets noisy fast near 0% or 100%.

Every move is printed with the resulting FEN as the game is played, so you
can watch how it went; pass --pgn to also save full games for replay in
any chess GUI.

Usage:
  python3 tools/elo_stockfish.py --model onnx/fenrir.onnx --nodes 800 \
      --elo 1400 1600 1800 2000 --games 20

  # single fixed opponent, quick look:
  python3 tools/elo_stockfish.py --model onnx/fenrir.onnx --nodes 800 \
      --elo 1500 --games 10 --pgn /tmp/fenrir_vs_sf.pgn
"""

import argparse
import math
import os
import random
import shutil
import sys
import tempfile
from collections import Counter

import chess
import chess.engine
import chess.pgn  # module level: a function-local import would shadow `chess`

PIECE_VALUES = {chess.PAWN: 1, chess.KNIGHT: 3, chess.BISHOP: 3, chess.ROOK: 5, chess.QUEEN: 9}


def material_balance(board):
    balance = 0
    for piece in board.piece_map().values():
        value = PIECE_VALUES.get(piece.piece_type, 0)
        balance += value if piece.color == chess.WHITE else -value
    return balance


def find_stockfish(path_arg):
    if path_arg:
        return path_arg
    found = shutil.which("stockfish")
    if found:
        return found
    if os.path.exists("/usr/games/stockfish"):
        return "/usr/games/stockfish"
    sys.exit("stockfish not found: install it (apt-get install stockfish) or pass --stockfish <path>")


def make_fenrir_dir(base_dir, model_path, cfg_path):
    work_dir = os.path.join(base_dir, "fenrir")
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


def play_game(white, white_limit, white_name, black, black_limit, black_name,
              opening_board, max_plies, show_fens):
    """Returns (white_result, final_board, termination)."""
    board = opening_board.copy()
    while not board.is_game_over(claim_draw=True) and board.ply() < max_plies:
        to_move_white = board.turn == chess.WHITE
        engine, limit, name = (white, white_limit, white_name) if to_move_white \
            else (black, black_limit, black_name)
        try:
            result = engine.play(board, limit)
        except chess.engine.EngineError as e:
            print(f"  engine error ({e}); forfeit for {'white' if to_move_white else 'black'}")
            return (0.0 if to_move_white else 1.0), board, "forfeit"
        if result.move is None or result.move not in board.legal_moves:
            return (0.0 if to_move_white else 1.0), board, "forfeit"
        board.push(result.move)
        if show_fens:
            print(f"    ply {board.ply():3d} {name:10} {result.move.uci():6} {board.fen()}")

    outcome = board.outcome(claim_draw=True)
    if outcome is None:
        return 0.5, board, "ply-cap"
    reason = outcome.termination.name.lower().replace("_", "-")
    if outcome.winner is None:
        return 0.5, board, reason
    return (1.0 if outcome.winner == chess.WHITE else 0.0), board, reason


def run_match(fenrir, fenrir_limit, stockfish, sf_elo, games, random_plies, seed,
              max_plies, pgn_path, show_fens):
    stockfish.configure({"UCI_LimitStrength": True, "UCI_Elo": sf_elo})
    sf_limit = chess.engine.Limit(time=0.1)

    pairs = max(1, games // 2)
    score = 0.0
    wins = draws = losses = 0
    terminations = Counter()
    material_sum = 0.0

    for pair in range(pairs):
        opening = random_opening(random_plies, seed + pair)
        for fenrir_is_white in (True, False):
            white, white_limit, white_name = (fenrir, fenrir_limit, "fenrir") if fenrir_is_white \
                else (stockfish, sf_limit, "stockfish")
            black, black_limit, black_name = (stockfish, sf_limit, "stockfish") if fenrir_is_white \
                else (fenrir, fenrir_limit, "fenrir")
            print(f"  game vs SF elo {sf_elo}: fenrir as {'white' if fenrir_is_white else 'black'}")
            white_result, final_board, termination = play_game(
                white, white_limit, white_name, black, black_limit, black_name,
                opening, max_plies, show_fens)
            terminations[termination] += 1
            fenrir_result = white_result if fenrir_is_white else 1.0 - white_result
            score += fenrir_result
            if fenrir_result == 1.0:
                wins += 1
            elif fenrir_result == 0.0:
                losses += 1
            else:
                draws += 1
            fenrir_material = material_balance(final_board) * (1 if fenrir_is_white else -1)
            material_sum += fenrir_material
            if pgn_path:
                game = chess.pgn.Game.from_board(final_board)
                game.headers["Event"] = f"Fenrir vs Stockfish(elo={sf_elo})"
                game.headers["White"] = "Fenrir" if fenrir_is_white else f"Stockfish {sf_elo}"
                game.headers["Black"] = f"Stockfish {sf_elo}" if fenrir_is_white else "Fenrir"
                game.headers["Result"] = "1-0" if white_result == 1.0 else "0-1" if white_result == 0.0 else "1/2-1/2"
                with open(pgn_path, "a") as f:
                    print(game, file=f, end="\n\n")
            print(f"    result: {'fenrir wins' if fenrir_result == 1.0 else 'SF wins' if fenrir_result == 0.0 else 'draw'}"
                  f"  ({termination})  [F {wins} / D {draws} / SF {losses}]")

    total = wins + draws + losses
    return {
        "elo": sf_elo, "total": total, "score": score,
        "wins": wins, "draws": draws, "losses": losses,
        "terminations": terminations, "avg_material": material_sum / total,
    }


def elo_estimate(score_fraction, opponent_elo, n):
    """Logistic Elo-difference formula, with a continuity correction so a
    perfect 0% or 100% score doesn't blow up log10(x/0)."""
    p = min(max(score_fraction, 1.0 / (2 * n)), 1.0 - 1.0 / (2 * n))
    diff = 400.0 * math.log10(p / (1.0 - p))
    return opponent_elo + diff


def main():
    parser = argparse.ArgumentParser(description="Estimate Fenrir's Elo vs calibrated Stockfish")
    parser.add_argument("--model", required=True, help="Fenrir ONNX weights")
    parser.add_argument("--nodes", type=int, default=800, help="Fenrir simulations per move")
    parser.add_argument("--binary", default="bin/fenrir")
    parser.add_argument("--cfg", default="fenrir.cfg")
    parser.add_argument("--stockfish", help="path to stockfish binary (default: auto-detect)")
    parser.add_argument("--elo", type=int, nargs="+", default=[1500],
                         help="one or more Stockfish UCI_Elo targets to test/sweep "
                              "(valid range is detected from the actual binary at startup)")
    parser.add_argument("--games", type=int, default=20, help="games per elo setting (rounded down to pairs)")
    parser.add_argument("--max-plies", type=int, default=400)
    parser.add_argument("--random-plies", type=int, default=4)
    parser.add_argument("--seed", type=int, default=0)
    parser.add_argument("--pgn", help="append all played games here for review in any chess GUI")
    parser.add_argument("--quiet", action="store_true", help="suppress per-move FEN output")
    args = parser.parse_args()

    binary = os.path.abspath(args.binary)
    if not os.path.exists(binary):
        sys.exit(f"engine binary not found: {binary}")
    sf_path = find_stockfish(args.stockfish)

    base_dir = tempfile.mkdtemp(prefix="fenrir_elo_")
    fenrir_dir = make_fenrir_dir(base_dir, args.model, args.cfg)
    fenrir_limit = chess.engine.Limit(nodes=args.nodes)

    # Launch Stockfish alone first and validate --elo against what this
    # specific binary actually reports before starting Fenrir at all --
    # calling sys.exit() after both engines are up would leave their
    # background UCI threads running and hang the process on exit instead
    # of returning, since nothing would have quit() them.
    stockfish = chess.engine.SimpleEngine.popen_uci(sf_path)
    try:
        if "UCI_Elo" not in stockfish.options or "UCI_LimitStrength" not in stockfish.options:
            sys.exit(f"{sf_path} has no UCI_Elo/UCI_LimitStrength option -- too old for calibrated-strength testing")
        sf_min = stockfish.options["UCI_Elo"].min
        sf_max = stockfish.options["UCI_Elo"].max
        print(f"detected Stockfish UCI_Elo range: {sf_min}-{sf_max}")
        for e in args.elo:
            if not (sf_min <= e <= sf_max):
                sys.exit(f"--elo {e} out of this Stockfish binary's supported range [{sf_min}, {sf_max}]")
    except SystemExit:
        stockfish.quit()
        raise

    fenrir = chess.engine.SimpleEngine.popen_uci(binary, cwd=fenrir_dir)

    print(f"Fenrir ({args.model}, {args.nodes} nodes/move) vs Stockfish ({sf_path}, calibrated)")
    print(f"elo settings: {args.elo}, {args.games} games each\n")

    results = []
    try:
        for sf_elo in sorted(args.elo):
            print(f"=== Stockfish UCI_Elo {sf_elo} ===")
            res = run_match(fenrir, fenrir_limit, stockfish, sf_elo, args.games,
                             args.random_plies, args.seed, args.max_plies,
                             args.pgn, not args.quiet)
            results.append(res)
            pct = 100.0 * res["score"] / res["total"]
            p = res["score"] / res["total"]
            sigma = (p * (1.0 - p) / res["total"]) ** 0.5
            print(f"  --- vs elo {sf_elo}: {res['score']:.1f}/{res['total']} = {pct:.1f}% "
                  f"(+/- {200.0*sigma:.1f}% at 2-sigma)  "
                  f"[F {res['wins']} / D {res['draws']} / SF {res['losses']}]  "
                  f"avg material {res['avg_material']:+.1f}\n")
    finally:
        fenrir.quit()
        stockfish.quit()
        shutil.rmtree(base_dir, ignore_errors=True)

    print("=" * 70)
    print("SUMMARY")
    print("=" * 70)
    print(f"{'SF elo':>8} {'score':>10} {'  pct':>8}")
    for res in results:
        pct = 100.0 * res["score"] / res["total"]
        print(f"{res['elo']:>8} {res['score']:>5.1f}/{res['total']:<4} {pct:>7.1f}%")

    if len(results) == 1:
        res = results[0]
        p = res["score"] / res["total"]
        est = elo_estimate(p, res["elo"], res["total"])
        print(f"\nsingle-point estimate: Fenrir ~= {est:.0f} Elo "
              f"(logistic conversion vs a {res['elo']}-rated opponent)")
        print("  note: this conversion is noisy away from a 50% score; a multi-point")
        print("  --elo sweep bracketing the crossover gives a steadier estimate.")
    else:
        crossover = None
        for a, b in zip(results, results[1:]):
            pa = a["score"] / a["total"] - 0.5
            pb = b["score"] / b["total"] - 0.5
            if pa >= 0 >= pb and pa != pb:
                frac = pa / (pa - pb)
                crossover = a["elo"] + frac * (b["elo"] - a["elo"])
                break
        if crossover is not None:
            print(f"\ncrossover (50% score point): Fenrir ~= {crossover:.0f} Elo")
        else:
            best = min(results, key=lambda r: abs(r["score"] / r["total"] - 0.5))
            print(f"\nno sign change across tested elo settings; closest to 50% was "
                  f"elo {best['elo']} ({100.0*best['score']/best['total']:.1f}%) -- "
                  f"widen the --elo range to bracket the true crossover")


if __name__ == "__main__":
    main()
