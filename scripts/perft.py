import subprocess
import time

tests = [
    {
        "name": "Standard Start Position",
        "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "expected": {
            1: 20,
            2: 400,
            3: 8902,
            4: 197281,
            5: 4865609,
            6: 119060324
        }
    },
    {
        "name": "Kiwipete",
        "fen": "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "expected": {
            1: 48,
            2: 2039,
            3: 97862,
            4: 4085603,
            5: 193690690
        }
    },
    {
        "name": "Position 3",
        "fen": "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "expected": {
            1: 14,
            2: 191,
            3: 2812,
            4: 43238,
            5: 674624,
            6: 11030083
        }
    },
    {
        "name": "Position 4",
        "fen": "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "expected": {
            1: 6,
            2: 264,
            3: 9467,
            4: 422333,
            5: 15833292
        }
    },
    {
        "name": "Position 5",
        "fen": "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "expected": {
            1: 44,
            2: 1486,
            3: 62379,
            4: 2103487,
            5: 89941194
        }
    }
]

import os
import sys

def run_perft(depth, fen):
    exe_name = "perft.exe" if sys.platform.startswith("win") else "perft"
    exe_path = os.path.join(".", "bin", exe_name)
    
    # Fallback to current directory if not in bin
    if not os.path.exists(exe_path):
        exe_path = os.path.join(".", "build", "Release", exe_name)
        if not os.path.exists(exe_path):
            exe_path = os.path.join(".", exe_name)
            
    cmd = [exe_path, str(depth), fen]
    try:
        output = subprocess.check_output(cmd, universal_newlines=True)
        nodes = 0
        nps = 0
        t = 0.0
        for line in output.split('\n'):
            if line.startswith('Nodes generated:'):
                nodes = int(line.split(':')[1].strip())
            elif line.startswith('Time taken:'):
                t = float(line.split(':')[1].strip().split()[0])
            elif line.startswith('NPS:'):
                nps = int(line.split(':')[1].strip())
        return nodes, nps, t
    except Exception as e:
        print(f"Error running perft: {e}")
        return -1, -1, -1

with open("PERFT_RESULTS.md", "w") as f:
    f.write("# Perft Benchmark Results\n\n")
    for t in tests:
        f.write(f"## {t['name']}\n")
        f.write(f"`FEN: {t['fen']}`\n\n")
        f.write("| Depth | Expected Nodes | Fenrir Nodes | Status | NPS | Time (s) |\n")
        f.write("|-------|----------------|--------------|--------|-----|----------|\n")
        
        for depth, expected in sorted(t["expected"].items()):
            print(f"Running {t['name']} Depth {depth}...")
            nodes, nps, time_s = run_perft(depth, t["fen"])
            status = "✅ PASS" if nodes == expected else "❌ FAIL"
            f.write(f"| {depth} | {expected:,} | {nodes:,} | {status} | {nps:,} | {time_s:.4f} |\n")
            f.flush()
        f.write("\n")
