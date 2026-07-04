# AI Context Document - Fenrir Chess Engine

> **Purpose**: This document provides AI assistants with complete context about the Fenrir chess engine project. Read this first before making any changes.

---

## Project Overview

Fenrir is the **rules and validation layer** of a three-tier chess ecosystem:

```
Layer 3 - UIs (mobile, web, desktop)
  React Native / Swift / JS / Electron
  Present, visualize, and explain chess to humans.

Layer 2 - Agents (Python, JS, RL training loops, minimax bots)
  Consume Fenrir via FFI. Search, evaluate, decide.
  They ask Fenrir what is legal. They decide what is best.

Layer 1 - Fenrir (this library)
  C++ shared library: libfenrir.so
  Rules, validation, legal move generation, FEN, PGN.
  Agents and UIs trust Fenrir completely for correctness.
```

**Fenrir's contract with its consumers:**
- Every move returned by `generate_moves()` / `generate_all_moves()` is fully legal (leaves no check conditions, handles correct side to move).
- `make_move()` **validates the move internally** — it matches the input against the generated legal moves list. In case of mock testing or out-of-turn moves (common in unit tests), a warning is logged and a permissive fallback applies the move.
- FEN and PGN are correct - consumers never need to parse raw chess notation.
- The API is stable - language bindings (Python ctypes, WASM, Swift FFI) do not break on updates.

**Performance Constraints:**
Agents doing reinforcement learning play millions of games during training. A minimax search at depth 5 with a branching factor of ~30 makes roughly 24 million `generate_all_moves()` calls. Fenrir must handle that load. 
* Avoid heap allocations inside critical paths (`generate_moves`, `apply_move`, `undo_move`).
* Keep `UndoState` as a pure POD struct (zero string objects, byte-level properties) to allow $O(1)$ memory copies during tree searches.

**Current Version**: 0.3.0
**Last Updated**: 2026-07-03
**License**: GPL-3.0

---

## Project Goals & Roadmap

### Completed (v0.1.0 - v0.3.0)

| Feature | Status | Notes |
|---------|--------|-------|
| Bitboard board representation (8x8) | Done | 12 `uint64_t` bitboards + 3 occupancy bitboards (white, black, all). |
| Precomputed attack tables | Done | Precomputed Knight, King, Pawn attacks + sliding ray tables in `attacks.h/cpp`. |
| FEN parsing and generation | Done | Full FEN string support in `fen.h/cpp`. |
| `Move` class with `MoveType` enum | Done | Holds coordinates, promotion piece, and precomputed `MoveType` flag. |
| `AbstractBoard` interface | Done | Decouples `Moves` from `Board` representation. |
| Pawn moves (push, captures, EP) | Done | Includes double push, en passant, and promotion generation. |
| Slider generation (Rook, Bishop, Queen) | Done | Ray-blocker masks. |
| Legal move filtering | Done | Filters out moves putting or leaving king in check. |
| Make / Unmake | Done | `Board::apply_move` and `Board::undo_move` using POD `UndoState` (zero allocations). |
| Castling & Promotion | Done | Handled correctly in move generation and state updates. |
| Checkmate / Stalemate | Done | Detected via legal move count and check states. |
| Turn enforcement | Done | Normal gameplay order checks with permissive test fallbacks. |
| Logging & PGN singletons | Done | Thread-safe, non-copyable singletons (`Logger`, `Pgn`). |
| Standard snake_case conventions | Done | Google C++ Style Guide naming adjustments. |
| 342 unit tests, 100% coverage | Done | Google Test, coverage targets, benchmark runs. |
| `is_draw()` | Done | Stalemate, fifty-move rule, and threefold repetition. |
| Library versioning | Done | Milestone 0 items complete (`VERSION` file, soname, `version()`, `install` target, `CHANGELOG.md`). |

### Planned (v0.4.0 - v0.6.0)

| Version | Feature | Notes |
|---------|---------|-------|
| v0.4.0 | Reference search agent (C++) | Minimax + alpha-beta, links `libfenrir.so` directly. Required before UCI. |
| v0.5.0 | UCI protocol (C++) | Wraps reference agent. Unlocks chess.com, Lichess, Arena, Stockfish benchmarking. |
| v0.6.0 | Python bindings | ctypes or pybind11. Unlocks Python RL agents. |
| v0.6.0 | WebAssembly build | Browser agents and web UIs. |

---

## Architecture

### Folder Structure

```
fenrir/
├── include/              # Public API headers
│   ├── abstract/
│   │   └── board.h      # AbstractBoard interface (Moves <-> Board decoupling seam)
│   ├── chess/
│   │   ├── board.h      # Board: 12 bitboards (uint64_t bitboards[12]) + occupancy
│   │   ├── attacks.h    # Attack tables + ray tables
│   │   ├── move.h       # Move: from/to coordinates, promotion_piece, move_type
│   │   ├── moves.h      # Moves: static legal move generation dispatcher
│   │   └── fen.h        # Fen: FEN string parsing and generation
│   ├── core/
│   │   └── core.h       # Constants (BOARD_SIZE, WHITE, BLACK), MoveType enum
│   ├── engine/
│   │   └── engine.h     # Engine: public facade class
│   ├── logger/          # Logging singleton (rotating, thread-safe, macro-wrapped)
│   ├── pgn/             # PGN file writing singleton
│   ├── chrono/          # Timestamp utilities
│   ├── modifier/        # Terminal color modifiers
│   └── utils/           # Algebraic notation parsing helpers
├── src/                 # Implementations (mirror include/)
├── tests/unit/          # Google Test suites (342 tests)
├── bin/lib/             # libfenrir.so build output
└── scripts/             # build targets, test coverage sh files, etc.
```

### Key Design Decisions

1. **Bitboard representation**: `Board` stores state as 12 `uint64_t` (one per piece-type/color). Bit n = square n (a1=0, h8=63). `get_piece(rank, file)` checks color occupancy bitboards first to bypass search loops on empty squares in $O(1)$ time.
2. **Precomputed attack tables** (`attacks.h/cpp`): Precomputed arrays for Knight, King, Pawn pushes, and Ray directions initialized once at startup.
3. **`AbstractBoard` interface**: Decouples board representation from move generation.
4. **Make/unmake**: POD `UndoState` stores byte-level data (`castling_rights` bitmask, `en_passant_square`) instead of `std::string` objects. Keeps search execution free from heap allocations. String formatting for FEN or output is evaluated lazily only when requested.
5. **`Moves` utility**: Exclusively uses `static` helper methods with no internal state, keeping call sites organized under the `Moves::` namespace prefix.
6. **`Engine`** is the public facade. External consumers only need `#include "include/engine/engine.h"`.

### Class Responsibilities

| Class / Struct | Responsibility | Key Methods |
|----------------|---------------|-------------|
| `Engine` | Public API facade | `generate_moves()`, `generate_all_moves()`, `make_move()`, `undo_move()`, `get_fen()`, `reset()`, `get_board_view()`, `is_checkmate()`, `is_stalemate()` |
| `Board` | Bitboards, state updates | `get_piece()`, `apply_move()`, `undo_move()`, `is_in_check()`, `reset()`, `print()` |
| `Moves` | Legal move generation | `generate_moves()`, `generate_bishop_moves()`, `slide_in_directions()`, etc. |
| `Fen` | FEN parsing/generation | `get_placement()`, `get_castling()`, `get_en_passant()`, `generate_fen()` |
| `Move` | Move coordinates, promotion | `get_from()`, `get_to()`, `get_move_type()`, `get_promotion_piece()`, `to_uci_notation()` |
| `AbstractBoard` | Decoupling interface | `get_piece()`, `get_en_passant()`, `get_castling_rights()`, `get_occupancy()` |
| `UndoState` | Lightweight POD state snapshot | Reverses changes of `apply_move()`. Copied in 1 CPU cycle. |

---

## Technical Details

### Build

```bash
make              # Debug build of libfenrir.so
make release      # Optimized build (-O2 -DNDEBUG, dynamic logs compiled out)
make test         # Run all 342 unit tests
make coverage     # Generate HTML coverage report (must remain 100%)
make clean        # Remove all build artifacts
```

### Coordinate & Naming Conventions

* **Coordinates**: `from` and `to` are unified parameters across constructors. Ranks 0-7 (a1=0, h8=63).
* **Naming**: `snake_case` for methods, functions, parameters, and local variables. `CamelCase` for classes, structs, and enums. `ALL_CAPS` for constants and macros.

---

## Common Pitfalls

1. **Do not break 100% coverage** - every new code path needs a unit test.
2. **Do not add CMake** - GNU Make only.
3. **`Board` is not copyable** - Rule of Five, copy/move deleted to ensure strict reference-based engine pass-downs.
4. **Avoid String Allocations in Search** - Never query `get_en_passant()` or `get_castling_rights()` during search loops (e.g. `apply_move`, `generate_moves`). Use byte-level equivalents `en_passant_square` and `castling_rights` directly.
