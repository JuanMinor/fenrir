#!/usr/bin/env python3
#   Copyright (c) 2026 Juan Minor
#   GPL-3.0-or-later (see repository LICENSE)
"""
Find a weight_decay (and optionally an LR schedule) that actually arrests
policy_fc.weight's magnitude growth -- offline, against real archived
self-play data, without spending live pod time on trial and error.

Background: a long run with plain Adam (lr=0.001, no weight_decay, no
grad clipping) let policy_fc.weight's absmax climb unbounded (33 -> 45
across many hours) until value calibration broke (a position up a whole
queen scored negative). AdamW with weight_decay=1e-4 was deployed as a
fix but two live checks after restart showed absmax still climbing
(43.1 -> 44.1 -> 44.2), just maybe slower -- inconclusive from two
expensive, hours-apart live samples. This script runs the same kind of
step cheaply and repeatably: load a real checkpoint, replay real archived
data through several candidate configs in parallel-but-isolated runs, and
watch the magnitude trend directly, in minutes instead of hours.

Read-only: never touches data/selfplay/ or your training data. Loads a
fixed sample of already-archived JSONL files into memory once and reuses
them; ChessDataset (in train.py) is NOT reused here because it deletes
consumed files -- fine for live training, dangerous for a diagnostic run
pointed at your archive directory.

Usage:
  python3 scripts/tune_weight_decay.py \
      --checkpoint checkpoints/fenrir_20260720_0751.pth \
      --data-dir data/selfplay/archive \
      --steps 2000 --weight-decay 0 1e-4 3e-4 1e-3
"""

import argparse
import glob
import json
import os
import random
import sys
import time

import torch
import torch.nn.functional as F

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "training"))
from model import AlphaZeroNet, infer_value_channels  # noqa: E402
from train import uci_to_index  # noqa: E402

BATCH_SIZE = 4096


def load_samples_readonly(data_dir, max_files=None):
    """Pure read: globs *.jsonl, parses lines, never deletes or moves
    anything. Mirrors ChessDataset's parsing exactly, minus the
    file-consumption side effects."""
    import chess

    files = sorted(glob.glob(os.path.join(data_dir, "*.jsonl")))
    if not files:
        sys.exit(f"no .jsonl files found in {data_dir}")
    if max_files:
        files = files[-max_files:]

    samples = []
    for path in files:
        with open(path) as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                try:
                    data = json.loads(line)
                    chess.Board(data["fen"])  # validate
                    samples.append(data)
                except (json.JSONDecodeError, ValueError, KeyError):
                    pass
    print(f"loaded {len(samples)} positions (read-only) from {len(files)} files in {data_dir}")
    return samples


def fen_to_tensor(fen):
    import chess

    board = chess.Board(fen)
    tensor = torch.zeros((14, 8, 8), dtype=torch.float32)
    for square, piece in board.piece_map().items():
        color_offset = 0 if piece.color == chess.WHITE else 6
        channel = color_offset + (piece.piece_type - 1)
        tensor[channel, chess.square_rank(square), chess.square_file(square)] = 1.0
    tensor[12, :, :] = 1.0 if board.turn == chess.WHITE else -1.0
    castling_rights = fen.split(" ")[2]
    tensor[13, :, :] = float(ord(castling_rights[0])) if castling_rights else 0.0
    return tensor


def precompute_tensors(samples):
    """Parse every sample's FEN/policy exactly once. The same finite pool
    gets resampled thousands of times across steps x configs; paying the
    python-chess parsing cost per-batch instead of once turns a one-time
    cost into tens of millions of redundant re-parses -- slow enough on a
    single CPU thread to leave the GPU sitting idle for the whole sweep."""
    tensors, policies, values = [], [], []
    for sample in samples:
        tensors.append(fen_to_tensor(sample["fen"]))
        turn = sample["fen"].split(" ")[1]
        policy = torch.zeros(4672, dtype=torch.float32)
        for move, prob in sample["policy"].items():
            policy[uci_to_index(move, turn)] = prob
        policies.append(policy)
        values.append((sample["result"] * 2) - 1)
    return (torch.stack(tensors),
            torch.stack(policies),
            torch.tensor(values, dtype=torch.float32).unsqueeze(1))


def make_batch(precomputed, batch_size, device):
    all_tensors, all_policies, all_values = precomputed
    n = all_tensors.shape[0]
    idx = torch.randint(0, n, (min(batch_size, n),))
    return all_tensors[idx].to(device), all_policies[idx].to(device), all_values[idx].to(device)


def run_config(checkpoint_state, value_channels, precomputed, device, steps,
                weight_decay, lr_decay, log_every):
    torch.manual_seed(0)
    model = AlphaZeroNet(value_channels=value_channels).to(device)
    model.load_state_dict(checkpoint_state)
    model.train()

    optimizer = torch.optim.AdamW(model.parameters(), lr=0.001, weight_decay=weight_decay)
    scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=steps) if lr_decay else None

    trend = []
    for step in range(1, steps + 1):
        tensors, policies, values = make_batch(precomputed, BATCH_SIZE, device)
        optimizer.zero_grad()
        pred_policies, pred_values = model(tensors)
        loss = F.cross_entropy(pred_policies, policies) + F.mse_loss(pred_values, values)
        loss.backward()
        torch.nn.utils.clip_grad_norm_(model.parameters(), max_norm=5.0)
        optimizer.step()
        if scheduler:
            scheduler.step()

        if step % log_every == 0 or step == steps:
            absmax = model.policy_fc.weight.abs().max().item()
            trend.append((step, absmax, loss.item()))

    return trend


def main():
    parser = argparse.ArgumentParser(description="Offline sweep to find a weight_decay that arrests magnitude growth")
    parser.add_argument("--checkpoint", required=True)
    parser.add_argument("--data-dir", default="data/selfplay/archive")
    parser.add_argument("--max-files", type=int, default=400, help="cap files loaded, for speed")
    parser.add_argument("--steps", type=int, default=2000)
    parser.add_argument("--log-every", type=int, default=200)
    parser.add_argument("--weight-decay", type=float, nargs="+", default=[0.0, 1e-4, 3e-4, 1e-3])
    parser.add_argument("--lr-decay", action="store_true", help="also test a cosine LR decay over --steps")
    args = parser.parse_args()

    device = "cuda" if torch.cuda.is_available() else "cpu"
    print(f"device: {device}")

    state = torch.load(args.checkpoint, map_location="cpu", weights_only=True)
    value_channels = infer_value_channels(state)
    start_absmax = state["policy_fc.weight"].abs().max().item()
    print(f"checkpoint: {args.checkpoint}  (value head width {value_channels})")
    print(f"starting policy_fc.weight absmax: {start_absmax:.3f}\n")

    samples = load_samples_readonly(args.data_dir, args.max_files)
    t0 = time.time()
    precomputed = precompute_tensors(samples)
    print(f"precomputed {precomputed[0].shape[0]} tensors once ({time.time() - t0:.0f}s) -- "
          f"every config below reuses these instead of re-parsing FEN/policy per batch\n")

    results = {}
    for wd in args.weight_decay:
        for lr_decay in ([False, True] if args.lr_decay else [False]):
            label = f"wd={wd:g}" + (" +cosine_lr" if lr_decay else "")
            print(f"=== {label} ===")
            t0 = time.time()
            trend = run_config(state, value_channels, precomputed, device, args.steps, wd, lr_decay, args.log_every)
            for step, absmax, loss in trend:
                print(f"  step {step:>5}  absmax {absmax:>8.3f}  loss {loss:.4f}")
            print(f"  ({time.time() - t0:.0f}s)\n")
            results[label] = trend

    print("=" * 60)
    print("SUMMARY")
    print("=" * 60)
    print(f"{'config':<20} {'start':>8} {'end':>8} {'delta':>8}  verdict")
    print("-" * 60)
    for label, trend in results.items():
        end_absmax = trend[-1][1]
        delta = end_absmax - start_absmax
        verdict = "GROWING" if delta > 0.5 else "flat/declining"
        print(f"{label:<20} {start_absmax:>8.3f} {end_absmax:>8.3f} {delta:>+8.3f}  {verdict}")


if __name__ == "__main__":
    main()
