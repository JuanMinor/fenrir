# Project: Fenrir Codebase Refactoring

## Architecture
Fenrir Chess Engine is written in C++ and structured as follows:
- `include/core/core.h` & `include/utils/utils.h`: Global constants, bitboard helpers, and notation converters.
- `include/abstract/board.h` & `include/chess/board.h`: Concrete board representation (bitboards, state).
- `include/chess/moves.h`: Moves generator.
- `include/eval/nn_eval.h`: ONNX Runtime-based evaluator loading `onnx/fenrir.onnx`.
- `include/search/mcts.h`: MCTS-based chess engine search.
- `include/uci/uci.h`: Universal Chess Interface.

## Milestones

| # | Name | Scope | Dependencies | Status |
|---|---|---|---|---|
| 1 | M1: Utilities & Hardware | `core/core.h`, `chrono/*`, `logger/*`, `modifier/*`, `utils/*`, `pgn/*`, `hardware/*`, `tuner/tuning_parameters.*` | None | DONE |
| 2 | M2: Rules & Board Representation | `abstract/board.h`, `chess/attacks.*`, `chess/move.*`, `chess/fen.*`, `chess/board.*`, `chess/moves.*` | M1 | PLANNED |
| 3 | M3: Engine & Tuning | `engine/*`, `abi/*`, `tuner/auto_tuner.*` | M1, M2 | PLANNED |
| 4 | M4: NN Search & UCI Loop | `eval/*`, `search/*`, `uci/*`, `selfplay/*` | M1, M2, M3 | PLANNED |

## Interface Contracts
- All APIs preserve existing types. No namespace adjustments are introduced.
- Function signatures are updated to use `snake_case` instead of `camelCase`.
- Javadoc block comments are added above every function in both `.h` and `.cpp` files.
- Functions are alphabetized in `.h` and `.cpp` files, keeping constructors/destructors at the top, and inline functions in headers.

## Code Layout
- Headers: `include/`
- Sources: `src/`
- Tests: `tests/`
