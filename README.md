# Fenrir - Neural Network Chess Engine

*Named after [Fenrir](https://en.wikipedia.org/wiki/Fenrir), the giant, prophesied-to-be-unstoppable wolf of Norse mythology.*

Fenrir is a C++20 [UCI](https://en.wikipedia.org/wiki/Universal_Chess_Interface)
chess engine: an AlphaZero-style Monte Carlo Tree Search (MCTS) guided by
a neural network, with a full self-play and PyTorch training pipeline to
produce and improve that network. It works with any standard chess GUI
(Arena, Cute Chess, etc.) over the UCI protocol, and ships its own
tooling for generating training data, training the network, and
diagnosing/benchmarking the result.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Build](https://img.shields.io/badge/Build-CMake-blue.svg)](https://cmake.org/)
[![Testing](https://img.shields.io/badge/Testing-Google%20Test-red.svg)](https://github.com/google/googletest)
[![Coverage](https://img.shields.io/badge/Coverage-100%25-brightgreen.svg)]()
[![Tests](https://img.shields.io/badge/Tests-381%20registered-success.svg)]()

> **Current Status**: Version 0.3.2. Full legal move enforcement (check
> detection, castling, promotion, pinned pieces, checkmate/stalemate) is
> complete and mathematically validated via deep Perft tests. On top of
> that rules core, Fenrir has a built-in AlphaZero-style MCTS + neural
> network search, UCI support, a self-play training pipeline, and a
> hardware auto-tuner.

> **🔧 Build System**: CMake is the supported build system. See [Build System](#build-system) for details.

> **🐳 Development**: Uses dev containers for consistent environments. VS Code recommended. See [Development Environment](#development-environment).

## ⚡ Quick Start

```bash
# Clone and build (CMake resolves ONNX Runtime automatically, matching
# your platform and GPU vendor -- see Build System below)
git clone <repository-url>
cd fenrir
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j

# Run the engine directly (drops into the UCI loop -- type "uci" then
# "isready", or point a GUI like Arena at ./bin/fenrir). A fresh clone
# has no network yet -- see "Playing Against Fenrir" below before this
# does anything useful.
./bin/fenrir

# Or use the build+run helper
./scripts/run.sh

# Run all tests
./scripts/test.sh

# Generate a coverage report (requires 100%)
./scripts/build/generate_coverage.sh
```

To actually play against Fenrir, or to generate training data / train a
new network, see [Playing Against Fenrir](#-playing-against-fenrir) and
[Self-Play & Training](#-self-play--training) below.

## ✨ What Fenrir Does

Fenrir has two layers: a chess rules core, and the AI engine built on top of it.

The **rules core** generates fully legal moves for all 6 piece types
(including en passant, castling, and promotion), with complete check
detection and pin resolution — `generate_moves` guarantees mathematical
correctness, backed by 381 unit tests and a multi-million node Perft
suite. It also handles FEN parsing/generation, checkmate/stalemate
detection, PGN recording, and logging.

The **AI engine** built on that core is what makes Fenrir a chess
engine rather than just a rules library:
- **Search**: AlphaZero-style MCTS (PUCT, virtual loss, pipelined
  batching) guided by an ONNX neural network evaluator, with hot model
  reload so a running engine picks up a freshly-trained network without
  restarting.
- **UCI protocol**: `position`, `go` (movetime / nodes / clock-based
  time management) — works with any standard chess GUI. Async `stop`
  support isn't implemented yet.
- **Self-play & training**: `--selfplay` generates training data
  (JSONL); the `training/` PyTorch pipeline consumes it and exports new
  ONNX models, closing the loop.
- **Auto-tuner**: `--auto-tune` benchmarks batch size, thread count,
  and pipeline depth for whatever hardware it's run on and persists
  them to `fenrir.cfg`, optionally capped by `--gpu-usage`/`--cpu-usage`
  so it doesn't take over the whole machine.

## ♟️ Playing Against Fenrir

Fenrir speaks standard UCI, so any UCI-compatible GUI works. A fresh
clone has no network yet, so there's one bootstrap step first — skip it
and the engine still runs, it just plays on meaningless fake evaluations
instead of anything real (see step 2):

1. Build the engine (see Quick Start above) — the binary is `bin/fenrir`.
2. **Generate a starting network.** `onnx/fenrir.onnx` isn't checked into the repo. The engine does *not* refuse to run without it — it starts up fine and plays *something*, which is worse: it silently falls back to serving uniform, made-up evaluations (one warning printed once, easy to miss, especially under a GUI that doesn't surface the engine's stderr) instead of actually failing. Nothing has been trained yet on a fresh clone, so create a randomly-initialized network to start from before relying on any output from the engine:
   ```bash
   pip install -r training/requirements.txt   # one-time, needed for this step
   python3 tools/init_model.py                # writes onnx/fenrir.onnx (+ onnx/fenrir.pth)
   ```
   This network knows nothing about chess yet — it plays legal moves but weakly, exactly like any freshly-initialized AlphaZero-style net. See [Self-Play & Training](#-self-play--training) below to actually make it stronger.
3. Point your GUI (Arena, Cute Chess, En Croissant, etc.) at `bin/fenrir` as a UCI engine. It looks for `onnx/fenrir.onnx` and `fenrir.cfg` relative to its working directory, so the GUI should launch it with the repo root as the working directory (or you can copy `bin/fenrir`, `onnx/`, and `fenrir.cfg` together elsewhere).
4. Run `./bin/fenrir --auto-tune` once on the target machine first (see Auto-Tuner above) so search parameters are actually tuned for that hardware — an un-tuned config still works, just slower.

## 🧬 Self-Play & Training

The training loop is: self-play generates games -> `training/train.py`
learns from them -> a new ONNX export replaces the network self-play
uses -> repeat.

```bash
# One-time: build a Release binary, then tune search parameters for
# this machine (writes fenrir.cfg)
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
CUDA_VISIBLE_DEVICES=0 ./bin/fenrir --auto-tune

# Install training dependencies (PyTorch, python-chess, ONNX export tooling)
pip install -r training/requirements.txt

# Launch self-play workers (one per GPU) plus the trainer, production-shaped
./scripts/run_selfplay.sh
```

`scripts/run_selfplay.sh` is configured entirely through environment
variables (`SIMULATIONS`, `INSTANCES_PER_GPU`, `GPUS`, `SEARCH_THREADS`,
`TRAIN`, `CHECKPOINT_INTERVAL` — see the comments at the top of the
script for details). Once it's running, `tools/` has everything needed
to check on it — see [Debugging & Diagnostic Tools](#-debugging--diagnostic-tools)
below, and `./scripts/shakedown.sh` for a one-time smoke test on new
hardware before trusting it with a long unattended run.

## 🚀 Performance Benchmarks

Fenrir is highly optimized for performance and strict mathematical correctness. Move-generation correctness at depth is checked via the standard chess-engine "perft" technique: count leaf nodes at fixed search depths from known positions, and compare against known-correct reference counts. `perft.cpp` builds as its own `bin/perft` binary (always built alongside `fenrir`, in both Debug and Release — see `CMakeLists.txt`) that exercises only the rules core, no search/NN/UCI involved; `tools/perft.py` drives it against 5 standard reference positions and writes `PERFT_RESULTS.md`. It's a standalone script, not part of the enforced Google Test suite or CI, so run it yourself if you want to verify correctness on your own build:
```bash
python3 tools/perft.py   # writes PERFT_RESULTS.md
```

*Note: The following results are from an actual run of `tools/perft.py` against this build, inside a Linux Docker container on an Early 2015 13-inch laptop (Dual-Core i7-5557U, 16GB RAM), single-threaded. Engine compiled with `-O3 -flto -march=native`. All 5 standard reference positions (Start Position, Kiwipete, and Positions 3-5) pass at every tested depth — see `PERFT_RESULTS.md` for the full table.*

### Standard Start Position
`rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1`
* **Depth 5**: 4,865,609 nodes (✅ Perfect Match) | ~8.2 Million NPS
* **Depth 6**: 119,060,324 nodes (✅ Perfect Match) | ~7.7 Million NPS

### Kiwipete (Aggressive Stress Test)
`r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1`
* **Depth 4**: 4,085,603 nodes (✅ Perfect Match) | ~7.7 Million NPS
* **Depth 5**: 193,690,690 nodes (✅ Perfect Match) | ~9.1 Million NPS

On modern desktop processors (e.g., AMD Zen 3 or newer) natively compiled with MSVC/MinGW, single-threaded throughput scales significantly higher. These are raw move-generation (`perft`) numbers, not search speed against the neural network — see `tools/selfplay_compute.py` and `tools/game_rate.py` for measuring actual self-play throughput on your hardware.

## Build System

**Fenrir uses CMake as the supported build system.**

### Requirements

- A C++20 compiler: **GCC 13+**, **Clang 17+**, or **MSVC 19.29+** (Visual Studio 2019 16.11+ / 2022). CMake hard-fails configure with a clear message on older compilers, since `<format>` isn't reliably available before these versions.
- **CMake 3.19+** (used to resolve the latest ONNX Runtime release/version at configure time).
- No manual ONNX Runtime install needed — CMake's `FetchContent` downloads it automatically: on Linux it detects your GPU vendor (`lspci`) and fetches a CUDA build for NVIDIA, a CPU-only build otherwise, and fails clearly (rather than silently falling back) if it detects an AMD GPU, since no prebuilt ROCm ONNX Runtime binary exists. On Windows it fetches the DirectML build (works with either NVIDIA or AMD).

### Building

```bash
# Configure the build system (defaults to Debug mode)
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build the project (static core library, tests, main executable)
cmake --build build

# Build optimized release version
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# The engine binary will be created at:
# bin/fenrir
```

### Run Tests and Coverage

```bash
# Compile and run all unit tests, then generate coverage report (requires 100% coverage)
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target coverage
```

## 🔍 Debugging & Diagnostic Tools

Every Python script that inspects, probes, or benchmarks the network or
engine lives in `tools/` at the repo root. All of them are run from the
repo root (they resolve `bin/fenrir`, `fenrir.cfg`, and `onnx/` relative
to your working directory, matching how the engine itself is normally
launched):

```bash
python3 tools/<tool>.py --help
```

They fall into four groups, depending on what question you're actually
asking.

### Model health & diagnosis

**`inspect_model.py`** — the first thing to reach for. Deep inspection of
a `.pth` checkpoint: parameter statistics and outlier flags, BatchNorm
health (dead channels, exploding/vanishing running stats), policy/value
head detail (singular-value spectrum, effective rank), behavioral value
probes on canned positions (no search), and — if you pass 2+ checkpoints
or `--onnx`/`--train-log` — per-layer drift, ONNX graph structure, and
parsed loss history. Use it any time you want a general "is this
checkpoint okay?" answer, or after every training restart.
```bash
python3 tools/inspect_model.py onnx/fenrir.pth
python3 tools/inspect_model.py checkpoints/*.pth --train-log logs/train.log
```

**`data_health.py`** — checks the *data*, not the network: decisive-game
rate, average game length, how often games hit the move cap, mate vs.
material-adjudication ratio (from worker logs), and self-play policy
target quality — entropy, uniqueness, sharpness (from the archived
JSONL). Use it to tell whether self-play itself is healthy and trending
the right way, independent of what the checkpoint's weights look like.
```bash
python3 tools/data_health.py
```

**`diagnose_position.py`** — answers "why did the engine play *that*
move, right here?" for one specific position. `--pth` dumps the raw
policy/value for every legal move with no search, so you can see whether
the network itself recognizes a threat; `--model --nodes N1 N2 ...`
runs the real engine via UCI at each node budget, so you can see whether
the problem is the network or just insufficient search depth. Use it
whenever a specific game or move looks wrong and you want to know why.
```bash
python3 tools/diagnose_position.py --fen "<FEN>" --pth onnx/fenrir.pth
python3 tools/diagnose_position.py --fen "<FEN>" --model onnx/fenrir.onnx --nodes 800 5000 20000
```

**`tune_weight_decay.py`** — an offline sweep that replays real archived
self-play data through several candidate hyperparameter values (built
for `weight_decay`, but the pattern generalizes) against a fixed
checkpoint, so you can find a fix and see the trend in minutes instead of
burning hours of live training time on trial and error. Use it when
you suspect a training hyperparameter is wrong and need to verify a
fix before deploying it.
```bash
python3 tools/tune_weight_decay.py --checkpoint checkpoints/<ckpt>.pth \
    --data-dir data/selfplay/archive --weight-decay 0 1e-4 3e-4 1e-3
```

### Strength measurement

**`arena.py`** — plays two checkpoints against each other and reports
the score with a confidence interval. This is a *relative* measurement:
it tells you whether a new checkpoint is stronger than an old one, not
how strong either is in absolute terms. Use it after any training
milestone to confirm you actually improved.
```bash
python3 tools/arena.py --model-a checkpoints/A.onnx --model-b checkpoints/B.onnx \
    --games 100 --nodes 800 --pgn games.pgn
```

**`gauntlet_greedy.py`** and **`tactics.py`** — *absolute* skill rungs
with fixed difficulty, so scores are comparable across checkpoints (unlike
arena's relative score). The gauntlet is a scripted greedy-material bot
(reliably beating it means not hanging pieces to a pure materialist);
tactics is a set of unambiguous, known-answer puzzles (mate-in-1,
win-the-hanging-piece), checkable either via the full engine (`--model`)
or the raw policy net alone (`--pth`, no search — "is the right answer in
the top-1/top-3 prior?"). Use these for an objective learning curve, not
just "better than last week."
```bash
python3 tools/gauntlet_greedy.py --model onnx/fenrir.onnx --games 20 --nodes 800
python3 tools/tactics.py --model onnx/fenrir.onnx --nodes 800
```

**`elo_stockfish.py`** — the real absolute-strength check: plays against
Stockfish at one or more `UCI_LimitStrength`/`UCI_Elo` settings and finds
the crossover where the score is ~50%. Use it when you need to know
actual playing strength, not just "better than the last checkpoint" —
relative arena wins do not imply absolute Elo gains.
```bash
python3 tools/elo_stockfish.py --model onnx/fenrir.onnx --nodes 800 \
    --elo 1400 1600 1800 2000 --games 20
```

### Operational / throughput

**`game_rate.py`** — measures live self-play throughput (games/hour)
from worker logs, either by sampling live (waiting an interval and
diffing the game count) or instantly against a past count you already
have. Pass `--cost-per-hour` to get $/game. Use it to know what your
compute is actually costing per unit of training data.
```bash
python3 tools/game_rate.py --interval 300 --cost-per-hour 2.776
```

**`selfplay_compute.py`** — estimates the GPU compute (FLOPs) self-play
game generation consumes, from the network's own architecture, the
simulation budget, and real average game length. Use it alongside
`game_rate.py` to understand hardware requirements or estimate throughput
on different hardware.
```bash
python3 tools/selfplay_compute.py onnx/fenrir.pth --simulations 800 --games-per-hour 3764
```

### Utilities & correctness

**`init_model.py`** — creates a fresh, randomly-initialized network and
exports it straight to ONNX. This is the one bootstrap step nothing
else here can do on its own (see [Playing Against Fenrir](#-playing-against-fenrir)):
`export_onnx.py` needs an *existing* `.pth`, and `train.py` only exports
after consuming self-play data — which itself needs `onnx/fenrir.onnx`
to generate. Run this once on a fresh clone, before anything else.
```bash
python3 tools/init_model.py
```

**`export_onnx.py`** — converts a `.pth` checkpoint to the `.onnx` format
the engine actually loads. Not a diagnostic tool itself, but a
prerequisite for nearly everything above: `arena.py`, `diagnose_position.py`
(`--model`), `elo_stockfish.py`, `gauntlet_greedy.py`, and `tactics.py`
(`--model`) all need a current `.onnx` export to test the latest weights.
```bash
python3 tools/export_onnx.py checkpoints/fenrir_20260718_0400.pth onnx/fenrir.onnx
```

**`perft.py`** — pure move-generator correctness check (the standard
chess-engine "perft" technique: count leaf nodes at fixed search depths
from known positions and compare against known-correct counts). Unlike
everything else in this folder, it has nothing to do with the trained
network — it validates the engine's chess rules themselves. Use it after
touching any move-generation code, not after a training change.
```bash
python3 tools/perft.py
```

### Operational scripts (shell)

Two shell scripts round out the operational side and live in `scripts/`
alongside the build helpers, since they're bash rather than Python:

- **`scripts/run_selfplay.sh`** — the production self-play + training
  launcher described in [Self-Play & Training](#-self-play--training) above.
- **`scripts/shakedown.sh`** — a 20-minute production-shaped end-to-end
  smoke test for a fresh multi-GPU box: launches real self-play plus
  training exactly as production would, then reports whether the whole
  pipeline actually worked. Use it once after setting up new hardware.
  ```bash
  ./scripts/shakedown.sh                 # 20 minutes, default layout
  DURATION=600 ./scripts/shakedown.sh    # shorter run
  ```

## 🛠️ Development

### Prerequisites

**Required (engine):**
- C++20 compiler: GCC 13+, Clang 17+, or MSVC 19.29+ (see [Build System](#build-system) for why)
- CMake 3.19+
- Google Test framework (for `-DENABLE_TESTING=ON`)
- lcov (for coverage reporting)

**Required (training pipeline, optional if you're only using the engine):**
- Python 3 with the packages in `training/requirements.txt` (PyTorch, python-chess, onnx/onnxscript)

**Recommended:**
- VS Code with Dev Containers extension
- Docker (for dev container)

### Development Environment

**Option 1: Dev Container (Recommended)**

```bash
# Clone and open in VS Code
git clone <repository-url>
cd fenrir
code .

# VS Code will prompt to "Reopen in Container"
# All dependencies are pre-installed in the container
```

**Benefits:**
- ✅ Consistent environment across machines
- ✅ All tools pre-installed (GCC, CMake, GTest, lcov)
- ✅ Integrated debugging with GDB
- ✅ No local setup required

**Option 2: Local Development**

Manually install all prerequisites, then:

```bash
git clone <repository-url>
cd fenrir
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
```

### Testing Requirements

**100% code coverage is REQUIRED:**

```bash
./scripts/test.sh      # Run all tests
./scripts/build/generate_coverage.sh  # Generate coverage report (fails if < 100%)
```

**Test Structure:**
- Framework: Google Test
- Location: `tests/unit/*.test.cpp`
- Count: 381 tests registered in the build, across 15 test suites (6 are environment-conditional stress tests that call `GTEST_SKIP()` outside suitable hardware, not disabled)
- Coverage: 100% line coverage enforced

### Build System

**CMake is the supported build system.**

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build      # Build debug version (default)
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build    # Build optimized release
rm -rf build                                                        # Clean build artifacts
```

## 📚 Documentation

- **README.md** (this file): User-facing documentation
- **[ai/AI_CONTEXT.md](ai/AI_CONTEXT.md)**: Technical documentation for AI coding assistants working in this repo — architecture, tools, and the same first-run bootstrap step covered above
- **CONTRIBUTORS.md**: Who's worked on this project

## 🤝 Contributing

**Before contributing:**
1. Read [ai/AI_CONTEXT.md](ai/AI_CONTEXT.md) to understand the architecture
2. Write tests first (TDD approach)
3. Ensure 100% coverage (`./scripts/build/generate_coverage.sh`)
4. Follow existing naming conventions (snake_case methods, PascalCase classes)
5. Update documentation for API changes

## 📄 License

MIT License - see [LICENSE](LICENSE) file.
