# AI Context

Technical context for an AI coding assistant working in this repo, so
you can get a contributor productive fast without re-deriving everything
from scratch. Read this before README.md if you're an AI — README.md is
written for humans and skips implementation detail this file covers.

## What This Project Is

Fenrir is a C++20 UCI chess engine: an AlphaZero-style Monte Carlo Tree
Search (MCTS) guided by a neural network, plus a full self-play +
PyTorch training pipeline to produce and improve that network. Two
layers:

1. **Rules core** (`src/chess/`, `src/engine/`, `include/chess/`,
   `include/engine/`) — legal move generation, FEN, PGN, check/pin/
   castling/promotion logic. Fully deterministic, no neural network
   involved. Validated by `tests/unit/` (Google Test, 100% coverage
   enforced by `scripts/build/generate_coverage.sh`/CI), and by
   `perft.cpp` (root-level, its own `bin/perft` executable target —
   see `CMakeLists.txt`, always built alongside `fenrir`) which recursive
   leaf-node-counts via `Engine::generate_all_moves()` /
   `make_move_fast()` / `undo_move()`. `tools/perft.py` drives that
   binary against 5 standard reference positions and writes
   `PERFT_RESULTS.md`; all pass at every tested depth as of the last
   run. `perft` is a standalone binary/script pair though — not part of
   the Google Test suite, not run by `scripts/test.sh`, and not enforced
   in CI, so it needs to be run manually to get a current result.
   **History note**: this exact gap (script present, no binary to run
   it against) existed in this repo before — `perft.cpp` was deleted at
   some point and only re-added when this was investigated. If
   `bin/perft` is ever missing again, that's the fix, not writing a new
   tool.
2. **AI engine** (`src/mcts/`, `src/nn/`, `src/uci/`, `src/selfplay/`,
   `src/tuner/`) — MCTS search guided by an ONNX-loaded neural network,
   a UCI protocol loop, self-play game generation, and a hardware
   auto-tuner. This is the part that "learns."

The two layers are cleanly separated: the rules core has zero knowledge
of search/NN/UCI; the AI engine consumes the rules core through
`include/engine/engine.h` and `include/abstract/board.h`.

## The One Thing You Must Know Before Touching Anything Runtime-Related

**`onnx/fenrir.onnx` is not checked into the repo** (`.gitignore`
excludes `*.onnx`; only `onnx/.gitkeep` is tracked). This does **not**
make the engine refuse to start — that would actually be the easy case.
`NN::try_reload_model()` (`src/nn/nn.cpp`) does a plain
`if (!std::filesystem::exists(onnx_file_path)) { return; }`, called from
the constructor, so `NN` construction always succeeds even with no file
present, leaving its ONNX Runtime `session` null. Every subsequent
`evaluate_batch()` call then hits the `!session` branch and silently
serves uniform fake evaluations (`value = 0.5`, flat policy) instead of
real inference — printed exactly **once** to `stderr`
(`WARNING: NN has no model session loaded...`), which a GUI-launched
process (Arena etc.) may never surface at all. Both the UCI loop and
self-play (`src/selfplay/selfplay.cpp`) construct `NN` the same way, so
both are affected identically: they run, they produce output, and
without watching for that one warning line nothing tells you the output
is meaningless. Self-play in this state would generate junk training
data indefinitely without erroring.

There is a real chicken-and-egg problem here that shapes the tooling:
`tools/export_onnx.py` converts an *existing* `.pth` checkpoint to
`.onnx`, but a fresh clone has no checkpoint either. `training/train.py`
only exports a new `.onnx` after it has consumed self-play data — which
itself needs `onnx/fenrir.onnx` to generate in the first place.

**`tools/init_model.py` breaks that loop.** It constructs a fresh
`AlphaZeroNet` (random PyTorch initialization, matching `training/model.py`)
and exports it directly to ONNX with no checkpoint involved. This is the
first command to run in any fresh environment, before anything else that
touches `onnx/`:

```bash
pip install -r training/requirements.txt   # needed for this + any training work
python3 tools/init_model.py                # writes onnx/fenrir.onnx and onnx/fenrir.pth
```

The resulting network is legal-but-weak (near-uniform policy, value near
0 everywhere) — it hasn't learned anything yet. That's expected; self-play
+ training is what improves it from there. If you're asked to "get the
engine running" or "set up the project" and skip this step, nothing will
error — the engine and self-play will both run and produce output that
looks plausible but is meaningless (uniform fallback evaluations, see
above). If UCI output or self-play games look suspiciously undirected,
or you see the "NN has no model session loaded" warning anywhere, check
for `onnx/fenrir.onnx` first.

`onnx/fenrir.pth` is the matching PyTorch state dict — `train.py` looks
for exactly this path to resume from (see `training/train.py`'s
`checkpoint_path = "onnx/fenrir.pth"`). Both files are written together
by `init_model.py` so a subsequent `train.py` run resumes from the exact
same random init it exported, rather than silently constructing a
*different* random init of its own.

## Build System

- **CMake 3.19+** required (uses `string(JSON ...)`, needs 3.19+).
- **C++20 compiler gate, enforced at configure time**: GCC 13+, Clang
  17+, or MSVC 19.29+ — `CMakeLists.txt` checks `CMAKE_CXX_COMPILER_ID`/
  `CMAKE_CXX_COMPILER_VERSION` right after `project()` and hard-fails
  with `FATAL_ERROR` on anything older, because `<format>` isn't
  reliably available before these versions. If a build fails with a
  compiler-version message, that's this check working as intended, not
  a bug.
- **ONNX Runtime is fetched automatically** via `FetchContent` — no
  manual install, and the version is *not* pinned: CMake queries
  GitHub's releases API for the latest tag at configure time and
  pattern-matches the actual returned asset names (not a constructed
  filename), so it survives upstream renames. On Linux, GPU vendor is
  detected via `lspci` (not `nvidia-smi`, so it doesn't require the
  vendor's own driver/tooling to already be installed): NVIDIA gets a
  CUDA build, no GPU gets a CPU-only build, and **AMD gets a hard
  configure failure** (Microsoft has never published a prebuilt
  ROCm-enabled ONNX Runtime binary — this is a deliberate fail-fast, not
  a bug to "fix" by silently falling back). WSL+AMD gets a distinct
  error message from native-Linux+AMD (different underlying cause).
  Windows always fetches the DirectML build via NuGet (also
  version-unpinned), since DirectML already works with either GPU vendor.
- Build artifacts: `bin/fenrir` (the executable), `bin/lib/libfenrir_core.a`
  (static library, linked into the executable — **there is no shared
  library target**; despite some earlier project history, nothing here
  produces `libfenrir.so`).
- `-DENABLE_TESTING=ON` adds the `tests` target (Google Test) and the
  `coverage` custom target (runs `scripts/build/generate_coverage.sh`,
  enforces 100% line coverage).
- `-DENABLE_AUTOTUNE=ON` (default ON) compiles `--auto-tune` support.

## Tools Reference (`tools/`)

All Python, all run from the repo root (they resolve `bin/fenrir`,
`fenrir.cfg`, `onnx/` relative to CWD, matching the engine itself).
`training/requirements.txt` covers their dependencies (PyTorch,
python-chess, onnx/onnxscript).

**Bootstrap:**
- `init_model.py` — see above. Run first, always.

**Model health & diagnosis:**
- `inspect_model.py` — the main checkpoint-health tool. Parameter stats,
  BatchNorm health, policy/value head spectrum analysis, behavioral
  value probes on canned positions, checkpoint-diffing, ONNX graph
  inspection, loss-curve parsing. Reach for this first for "is this
  checkpoint okay?"
- `data_health.py` — self-play data quality (decisive-game rate, game
  length, mate-vs-adjudication ratio, policy-target entropy). Reads
  worker logs (`logs/gpu*_inst*.log`) and archived JSONL
  (`data/selfplay/archive/`).
- `diagnose_position.py` — why did the engine play *this* move at *this*
  position. `--pth` = raw network, no search. `--model --nodes N...` =
  real engine via UCI at increasing node budgets, to separate "network
  doesn't understand" from "insufficient search depth."
- `tune_weight_decay.py` — offline hyperparameter sweep against real
  archived data, to validate a training-hyperparameter fix in minutes
  instead of burning live training time.

**Strength measurement:**
- `arena.py` — two checkpoints head-to-head, relative score only.
- `gauntlet_greedy.py` / `tactics.py` — absolute, fixed-difficulty skill
  rungs (comparable across checkpoints, unlike arena).
- `elo_stockfish.py` — real absolute Elo estimate against
  `UCI_LimitStrength`-capped Stockfish.

**Operational:**
- `game_rate.py` — self-play games/hour and $/game from worker logs.
- `selfplay_compute.py` — estimated GPU FLOPs for self-play generation.

**Utilities:**
- `export_onnx.py` — `.pth` -> `.onnx` for an *existing* checkpoint.
- `perft.py` — move-generator correctness (not network-related at all).

Two related shell scripts live in `scripts/`, not `tools/`, since
they're bash: `scripts/run_selfplay.sh` (production self-play + training
launcher, configured via env vars — see its header comments) and
`scripts/shakedown.sh` (20-minute end-to-end smoke test for new hardware).

## Self-Play & Training Loop

```
self-play (bin/fenrir --selfplay) --> JSONL game data (data/selfplay/)
    --> training/train.py consumes it --> exports onnx/fenrir.onnx + onnx/fenrir.pth
    --> self-play workers hot-reload the new .onnx --> repeat
```

- `train.py` only exports after processing a training cycle (needs
  `len(dataset) >= 4096` positions buffered) — it doesn't export
  immediately on startup, hence the `init_model.py` bootstrap requirement.
- `ChessDataset` in `train.py` **deletes/archives consumed JSONL files**
  as a side effect (archives 1-in-N via `ARCHIVE_EVERY`, default 20).
  Any tool that needs to read training data non-destructively (e.g.
  `tune_weight_decay.py`) must NOT reuse `ChessDataset` directly against
  live `data/selfplay/` — it needs its own read-only loader.
  `tune_weight_decay.py`'s `load_samples_readonly()` is the existing
  pattern to follow if you build something similar.
- Optimizer is `AdamW(lr=0.001, weight_decay=0.03)` — the weight_decay
  value was empirically derived (see the comment block right above it in
  `train.py`) after a real production incident where plain `Adam` with
  no `weight_decay` let parameter magnitudes drift unbounded until value
  calibration broke. Don't casually change this without re-deriving via
  `tune_weight_decay.py` against real data.
- `--auto-tune` benchmarks batch size / thread count / pipeline depth
  for the current machine and writes `fenrir.cfg`. Supports
  `--gpu-usage`/`--cpu-usage` caps (percent) so it can be run on a
  machine you don't want to fully monopolize — it measures real
  utilization via NVML (GPU, NVIDIA-only, dynamically loaded so it
  doesn't hard-fail on other vendors) and process CPU time, and rejects
  configs that exceed the caps rather than just chasing peak throughput.
- There is no `--auto-tune` equivalent for AMD/DirectML GPU utilization
  measurement (no NVML equivalent exists) — the CPU cap still works
  there, the GPU cap silently can't be enforced (a warning is printed).

## UCI Protocol Support

Commands handled (`src/uci/uci.cpp`): `uci`, `isready`, `ucinewgame`,
`position`, `go`, `quit`. **Not implemented**: async `stop` (a `go`
command runs to completion), `setoption` (all tuning is via `fenrir.cfg`,
not UCI options), `debug`. If asked to add UCI features, check this list
first — GUIs that assume `stop`/`setoption` support may behave
unexpectedly.

`go` accepts `movetime`, `wtime`/`btime`/`winc`/`binc` (clock-based, 1/30
of remaining time + 3/4 increment, with a 200ms safety reserve), and
`nodes` (used as a pure simulation cap with no time bound). Priority:
explicit `movetime` wins outright; otherwise clock time if present;
otherwise `nodes`.

## Testing

- Framework: Google Test, `tests/unit/*.test.cpp`.
- 381 tests currently registered in `CMakeLists.txt`'s `add_executable(tests ...)`
  list. **Known issue**: `tests/unit/mcts_fallback.test.cpp` exists on
  disk with 1 test but is *not* in that list — it never compiles or
  runs. Worth fixing if you're touching the test target, but it's a
  pre-existing gap, not something introduced by other changes.
- 6 of the 381 registered tests are environment-conditional stress tests
  that call `GTEST_SKIP()` rather than being disabled — search for
  `GTEST_SKIP()` in `tests/unit/` if coverage numbers look off from what
  you expect.
- 100% line coverage is enforced by `scripts/build/generate_coverage.sh`.

## Known Inconsistencies (not yet resolved, don't silently pick a side)

- **License — resolved.** The root `LICENSE` file used to be MIT
  (`Copyright (c) 2023 IkeHammons`) while every source file's header
  declared GPL-3.0-or-later (`Copyright (c) 2026 Juan Minor`) — a real
  conflict. Resolved by replacing `LICENSE` with the verbatim, official
  GPLv3 text (fetched from `gnu.org/licenses/gpl-3.0.txt`, unmodified),
  matching what every source file header already claimed. If you ever
  find MIT referenced anywhere in this repo again, that's stale and
  should be corrected to GPL, not the other way around.
- Git commit author history uses "Ike"/"IkeHammons"
  (`juanfminor@gmail.com`); source file copyright headers say "Juan
  Minor" (same email). Likely the same person under different display
  names — not necessarily a conflict, just worth knowing both forms
  appear in the repo's history.

## Conventions

- snake_case methods/functions, PascalCase classes, matching existing
  code — this is a C++ project with Python tooling bolted on, and both
  sides follow their respective language's idiomatic style already
  present in the codebase.
- Python scripts in `tools/` use the full GPL boilerplate header (not
  the shorter SPDX-only form) — match whichever files already use, but
  the long form is now the established convention for `tools/`.
- Relative paths in both `tools/*.py` and `scripts/*.sh` are meant to be
  run from the repo root. Python scripts locate `training/` via
  `os.path.dirname(__file__)` math (one level up from `tools/`); shell
  scripts in `scripts/` use a `SCRIPT_DIR`/`PROJECT_ROOT` `BASH_SOURCE`
  pattern so they work regardless of the caller's CWD. If you move any
  of these files, both path schemes need updating for their new depth —
  this has bitten past reorganizations in this repo.
