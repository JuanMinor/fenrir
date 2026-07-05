# Perft (Performance Test) Plan & Context

## 1. Goal for Next Session
Build a **Perft (Performance Test)** suite for the Fenrir engine. 
The objective is to absolutely guarantee 100% rule correctness (including edge cases like en passant and castling rights) and to benchmark the engine's Nodes Per Second (NPS) speed.

## 2. What is Perft?
A Perft test works by:
1. Loading a specific, complex FEN string (e.g., the famous "Kiwipete" position).
2. Recursively generating every single legal move out to a specific depth (e.g., depth 4, 5, or 6).
3. Counting the exact number of nodes (board states) generated.
4. Comparing that exact number against mathematically proven answers (from established engines like Stockfish).

If Fenrir's node count matches the known correct counts at every depth, we can guarantee that the move generator and rule enforcer are completely flawless.

## 3. Hardware & Environment Context
*These details were analyzed from `server.specs` and container limits to understand the benchmarking environment.*

**Host Machine:** 
- **Model:** Early 2015 13-inch Apple MacBook Pro (Running Linux).
- **CPU:** Intel Core i7-5557U @ 3.10GHz (Broadwell, 2 Cores / 4 Threads).
- **Memory:** 16 GiB System RAM.

**Docker Container Limits:**
- **CPU:** Unrestricted (`cpu.max = max`). The container has full access to all 4 logical threads.
- **Memory:** Unrestricted. Full access to the host's 15-16 GiB of RAM.
- **Impact on Benchmarking:** Since Fenrir's move generation is strictly single-threaded, the Perft suite will utilize exactly 1 thread, allowing the i7 core to reach its maximum boost clock. There are no Docker-imposed quotas that will artificially throttle the benchmark.

## 4. Current Engine State
- **Strict Rule Enforcement:** `engine.make_move()` now explicitly throws `std::invalid_argument` if an illegal or out-of-turn move is attempted. There is no longer a permissive fallback.
- **C ABI:** The C wrapper (`fenrir_c.cpp`) is now built directly into `libfenrir.so`, allowing easy integration with Python or testing frameworks without needing external wrappers.
- **Thread Safety:** Move generation is highly performant and single-threaded. Mutexes only guard the logger and PGN writers.

## 5. Next Steps
1. Create a C++ Perft binary (or Python script using the C ABI) that can recursively traverse the move tree.
2. Feed it the standard Perft test suites (Start Position, Kiwipete, Position 3, etc.).
3. Record timings and node counts to compare with established baselines.
