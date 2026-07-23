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
Measure live self-play throughput (games/hour) and $/game, from the same
worker-log game counter data_health.py uses.

Two modes:
  --interval N   live sample: count games now, sleep N seconds, count
                 again, compute the rate from the delta (matches the
                 two-snapshot method run_selfplay.sh's own header
                 comments already recommend).
  --since T G    historical sample: rate between a past (timestamp,
                 game count) you already have -- e.g. from an earlier
                 data_health.py run -- and right now. Skips the wait.

Usage:
  python3 tools/game_rate.py --interval 300 --cost-per-hour 2.776
  python3 tools/game_rate.py --since "2026-07-22 18:20:48" 129727 --cost-per-hour 2.776
"""

import argparse
import datetime as dt
import glob
import re
import sys
import time

GAME_RE = re.compile(r"Game \d+ finished in (\d+) moves\. Result: ([\d.]+)(?: \((\w+)\))?")


def count_games(log_glob):
    n = 0
    for path in glob.glob(log_glob):
        with open(path, errors="replace") as f:
            for line in f:
                if GAME_RE.search(line):
                    n += 1
    return n


def report(dgames, hours, cost_per_hour):
    rate = dgames / hours if hours > 0 else float("nan")
    print(f"\n{dgames} games over {hours:.4f}h = {rate:.1f} games/hour")
    if cost_per_hour and rate > 0:
        cost_per_game = cost_per_hour / rate
        games_per_dollar = 1.0 / cost_per_game
        print(f"at ${cost_per_hour:g}/hour: ${cost_per_game:.6f}/game "
              f"(~{games_per_dollar:.0f} games per dollar)")


def main():
    parser = argparse.ArgumentParser(description="Self-play games/hour and $/game")
    parser.add_argument("--log-glob", default="logs/gpu*_inst*.log")
    parser.add_argument("--interval", type=int, help="live sample: seconds to wait between snapshots")
    parser.add_argument("--since", nargs=2, metavar=("TIMESTAMP", "GAME_COUNT"),
                         help="historical sample: a past 'YYYY-MM-DD HH:MM:SS' and the game count at that time")
    parser.add_argument("--cost-per-hour", type=float, default=2.776)
    args = parser.parse_args()

    if not args.interval and not args.since:
        sys.exit("pass --interval (live) or --since TIMESTAMP GAME_COUNT (historical)")

    if args.since:
        ts, prior_count = args.since
        prior_dt = dt.datetime.strptime(ts, "%Y-%m-%d %H:%M:%S")
        now_count = count_games(args.log_glob)
        hours = (dt.datetime.now() - prior_dt).total_seconds() / 3600
        print(f"then: {prior_count} games @ {ts}")
        print(f"now:  {now_count} games @ {dt.datetime.now():%Y-%m-%d %H:%M:%S}")
        report(now_count - int(prior_count), hours, args.cost_per_hour)

    if args.interval:
        n1 = count_games(args.log_glob)
        print(f"snapshot 1: {n1} games, waiting {args.interval}s...")
        time.sleep(args.interval)
        n2 = count_games(args.log_glob)
        print(f"snapshot 2: {n2} games")
        report(n2 - n1, args.interval / 3600, args.cost_per_hour)


if __name__ == "__main__":
    main()
