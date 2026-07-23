#!/usr/bin/env python3
#   Copyright (c) 2026 Juan Minor
#   GPL-3.0-or-later (see repository LICENSE)
"""
Estimate GPU compute consumed by self-play game generation alone (not
training), from the network's own per-position FLOPs cost, the configured
MCTS simulation budget, and real average game length pulled from worker
logs (same source/regex data_health.py uses).

Assumes ~1 NN forward pass per MCTS simulation. That's an upper bound --
a simulation that lands on an already-expanded node or a terminal
position (checkmate/stalemate/draw, scored exactly per mcts.cpp) doesn't
need a fresh forward pass -- so real usage is somewhat below this.

Usage:
  python3 scripts/selfplay_compute.py onnx/fenrir.pth --simulations 800
  python3 scripts/selfplay_compute.py onnx/fenrir.pth --simulations 800 --games-per-hour 3780
"""

import argparse
import glob
import re

import torch

GAME_RE = re.compile(r"Game \d+ finished in (\d+) moves\. Result: ([\d.]+)")


def flops_per_position(state_dict):
    """Multiply-add = 2 FLOPs, same convention as inspect_model.py's report_flops."""
    board = 8 * 8
    flops = 0
    for name, tensor in state_dict.items():
        if not name.endswith("weight") or not torch.is_floating_point(tensor):
            continue
        shape = tuple(tensor.shape)
        if len(shape) == 4:  # conv: [out, in, kh, kw]
            out_c, in_c, k_h, k_w = shape
            flops += 2 * in_c * out_c * k_h * k_w * board
        elif len(shape) == 2:  # linear: [out, in]
            out_f, in_f = shape
            flops += 2 * in_f * out_f
    return flops


def avg_plies_per_game(log_glob):
    lens = []
    for path in glob.glob(log_glob):
        with open(path, errors="replace") as f:
            for line in f:
                m = GAME_RE.search(line)
                if m:
                    lens.append(int(m.group(1)))
    if not lens:
        return None, 0
    return sum(lens) / len(lens), len(lens)


def main():
    parser = argparse.ArgumentParser(description="Estimate self-play (game generation) GPU compute usage")
    parser.add_argument("checkpoint", help=".pth checkpoint (e.g. onnx/fenrir.pth)")
    parser.add_argument("--simulations", type=int, default=800, help="MCTS simulation budget per move")
    parser.add_argument("--log-glob", default="logs/gpu*_inst*.log")
    parser.add_argument("--avg-plies", type=float, help="override measured average game length (plies)")
    parser.add_argument("--games-per-hour", type=float, help="aggregate self-play throughput, e.g. from game_rate.py")
    args = parser.parse_args()

    state = torch.load(args.checkpoint, map_location="cpu", weights_only=True)
    pos_flops = flops_per_position(state)
    print(f"forward cost / position: {pos_flops / 1e9:.3f} GFLOPs")

    move_flops = pos_flops * args.simulations
    print(f"\nper move ({args.simulations} simulations, ~1 NN eval/simulation, upper bound):")
    print(f"  {move_flops / 1e9:.1f} GFLOPs = {move_flops / 1e12:.4f} TFLOPs")

    if args.avg_plies:
        plies, n = args.avg_plies, None
    else:
        plies, n = avg_plies_per_game(args.log_glob)

    if plies:
        game_flops = move_flops * plies
        src = f"measured over {n} games in {args.log_glob}" if n else "override"
        print(f"\nper game (avg {plies:.1f} plies, {src}):")
        print(f"  {game_flops / 1e12:.2f} TFLOPs")

        if args.games_per_hour:
            hourly_flops = game_flops * args.games_per_hour
            sustained = hourly_flops / 3600  # FLOP/s
            print(f"\nat {args.games_per_hour:g} games/hour:")
            print(f"  {hourly_flops / 1e15:.2f} PFLOPs/hour")
            print(f"  {sustained / 1e12:.2f} TFLOP/s sustained, aggregate across all self-play GPUs")
    else:
        print(f"\nno games found matching {args.log_glob} to measure average length; "
              f"pass --avg-plies, or --games-per-hour will be unavailable without it")


if __name__ == "__main__":
    main()
