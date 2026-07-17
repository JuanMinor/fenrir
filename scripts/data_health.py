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
Training-run health report for Fenrir self-play.

Two data sources, two lifetimes:
  - Worker logs (logs/gpu*_inst*.log): every game ever played this run —
    results, lengths, and trends survive here even after training data
    is consumed.
  - Policy JSONL (data/selfplay/READY_* plus data/selfplay/archive/*):
    policy-target quality (entropy, sharpness, diversity). train.py
    archives a 1-in-N sample of consumed files (ARCHIVE_EVERY, default
    20) so this section can show trends over the whole run, ordered by
    file modification time.

Usage: python3 scripts/data_health.py           # from the repo root
       python3 scripts/data_health.py --recent 500
"""

import argparse
import glob
import json
import math
import os
import re
import time

GAME_RE = re.compile(r"Game \d+ finished in (\d+) moves\. Result: ([\d.]+)")


def analyze_logs(log_glob, recent):
    games = []  # (moves, result) in per-file order
    for path in sorted(glob.glob(log_glob)):
        with open(path, errors="replace") as f:
            for line in f:
                m = GAME_RE.search(line)
                if m:
                    games.append((int(m.group(1)), float(m.group(2))))

    print("=" * 62)
    print("GAMES (from worker logs — full run history)")
    print("=" * 62)
    if not games:
        print("no finished games found\n")
        return

    def summarize(label, subset):
        n = len(subset)
        moves = [g[0] for g in subset]
        wins = sum(1 for g in subset if g[1] == 1.0)
        losses = sum(1 for g in subset if g[1] == 0.0)
        draws = n - wins - losses
        capped = sum(1 for mv in moves if mv >= 200)
        print(f"{label:<16} {n:>6} games | decisive {100.0*(wins+losses)/n:5.1f}% "
              f"(W {wins} / D {draws} / L {losses}) | avg len {sum(moves)/n:6.1f} "
              f"| at 200-cap {100.0*capped/n:5.1f}%")

    summarize("all", games)
    if len(games) > 2 * recent:
        summarize(f"first {recent}", games[:recent])
    if len(games) > recent:
        summarize(f"last {recent}", games[-recent:])
    print()


def analyze_jsonl(data_dir, per_file_limit):
    pending = glob.glob(os.path.join(data_dir, "READY_*.jsonl"))
    archived = glob.glob(os.path.join(data_dir, "archive", "*.jsonl"))
    files = sorted(pending + archived, key=os.path.getmtime)

    print("=" * 62)
    print(f"POLICY TARGETS ({len(archived)} archived + {len(pending)} pending files)")
    print("=" * 62)
    if not files:
        print("no JSONL available (archive appears after train.py consumes ~20 files)\n")
        return

    print(f"{'file time':<16} {'positions':>9} {'uniq%':>6} {'entropy':>8} {'maxp':>6} {'draw%':>6}")
    grand_ent, grand_n = 0.0, 0
    for path in files[-per_file_limit:]:
        ents, maxps, fens, draws, n = [], [], set(), 0, 0
        with open(path, errors="replace") as f:
            for line in f:
                try:
                    d = json.loads(line)
                except json.JSONDecodeError:
                    continue
                ps = list(d["policy"].values())
                if not ps:
                    continue
                n += 1
                ents.append(-sum(p * math.log(p + 1e-12) for p in ps))
                maxps.append(max(ps))
                fens.add(d["fen"].split(" ")[0])
                if d.get("result") == 0.5:
                    draws += 1
        if n == 0:
            continue
        stamp = time.strftime("%m-%d %H:%M", time.localtime(os.path.getmtime(path)))
        print(f"{stamp:<16} {n:>9} {100.0*len(fens)/n:>5.0f}% "
              f"{sum(ents)/n:>8.3f} {sum(maxps)/n:>6.3f} {100.0*draws/n:>5.1f}%")
        grand_ent += sum(ents)
        grand_n += n

    if grand_n:
        print(f"\noverall mean target entropy: {grand_ent/grand_n:.3f} over {grand_n} positions")
        print("(healthy: entropy >= 1.5 and drifting DOWN slowly as the net sharpens;")
        print(" a sudden collapse toward 0, or uniq% collapsing, is the alarm)")
    print()


def main():
    parser = argparse.ArgumentParser(description="Fenrir training-run health report")
    parser.add_argument("--logs", default="logs/gpu*_inst*.log", help="worker log glob")
    parser.add_argument("--data-dir", default="data/selfplay", help="self-play data directory")
    parser.add_argument("--recent", type=int, default=500, help="window for first/last game trends")
    parser.add_argument("--files", type=int, default=30, help="max JSONL files listed (most recent)")
    args = parser.parse_args()

    analyze_logs(args.logs, args.recent)
    analyze_jsonl(args.data_dir, args.files)


if __name__ == "__main__":
    main()
