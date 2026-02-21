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
- Every move returned by `generateMoves()` is legal (no illegal moves in the list)
- `makeMove()` **validates the move internally** — it calls `generateMoves()` and rejects any move not in the legal set. Agents cannot bypass move validation by constructing a `Move` directly. Passing an illegal or absurd move returns an error; the board state is unchanged.
- FEN and PGN are correct - consumers never reimplement chess notation
- The API is stable - language bindings (Python ctypes, WASM, Swift FFI) do not break on updates

> **Current limitation (v0.2.0-dev):** `makeMove()` validates against **pseudo-legal** moves only (piece movement rules, not legality). Full legal validation (move cannot leave king in check) is added in v0.3.0 once `generateMoves()` produces legal moves.

**Why performance matters here:** Agents doing reinforcement learning play millions of games during training. A minimax search at depth 5 with a branching factor of ~30 makes roughly 24 million `generateMoves()` calls. Fenrir must handle that load. Every allocation and serialization inside `generateMoves()` is multiplied by that volume.

**Current Version**: 0.2.0-dev
**Last Updated**: 2026-02-18
**License**: GPL-3.0

---

## Project Goals & Roadmap

### Completed (v0.1.0 - v0.2.0)

| Feature | Status | Notes |
|---------|--------|-------|
| Chess board representation (8x8) | Done | Mailbox (`unique_ptr<Piece>`) — **being replaced by bitboards in v0.3.0** |
| FEN parsing and generation | Done | Full FEN string support |
| `Move` class with `MoveType` enum | Done | Replaces old `pair<string,string>` |
| `AbstractBoard` interface | Done | Decouples `Moves` from `Board` |
| Pawn moves (single + double push) | Done | Direction based on color |
| Pawn diagonal captures | Done | Left and right |
| En passant capture | Done | Detected from FEN `getEnPassant()` |
| Rook sliding moves | Done | 4 directions, blocking detection |
| Knight L-shaped jumps | Done | Can jump over pieces |
| Bishop diagonal sliding | Done | 4 directions, blocking detection |
| Queen moves (rook + bishop) | Done | 8 directions |
| King single-square moves | Done | 8 directions, depth=1 |
| Logging system (rotating, leveled) | Done | Production-ready |
| PGN recording (basic) | Done | `pgn/fenrir.pgn` |
| Shared library build | Done | `bin/lib/libfenrir.so` |
| 300 unit tests, 100% coverage | Done | Google Test, enforced |
| Memory safety (`unique_ptr`) | Done | No raw owning pointers, Rule of Five |
| Const correctness | Done | All `Moves` methods are `const` |

### Not Yet Implemented (v0.3.0 target)

| Feature | Priority | Prerequisite |
|---------|----------|-------------|
| **Bitboard foundation rewrite** | Critical | Phase 0 — must complete before all below. Replaces `unique_ptr<Piece>` grid with 12 `uint64_t` bitboards. New `attacks.h/cpp` with precomputed knight, king, pawn, and ray tables. All 300 existing tests must pass after. |
| **AbstractBoard extensions** (`getCastlingRights` as bitmask, `getColor`, `getBB`, `getOccupancy`, `getEnPassantBB`) | Critical | Bitboard rewrite |
| **Check detection** | Critical | Attack tables + AbstractBoard extensions. With bitboards: O(1) via `attackers & king_bb`. |
| **Make/unmake** (`Board::applyMove()` / `Board::undoMove()` with `UndoState`) | Critical | Check detection. With bitboards: save 12 integers + ~10 bytes. Zero allocation. |
| **Legal move filtering** (strip pseudo-legal moves leaving king in check) | Critical | Make/unmake (apply -> isInCheck -> undo, no copies) |
| **`Engine::undoMove()`** (public undo, pops internal `vector<UndoState>`) | Critical | Make/unmake |
| **Castling move generation** | High | Check detection |
| **Pawn promotion detection** | High | Legal move filtering |
| **Checkmate detection** | High | Legal move filtering |
| **Stalemate detection** | High | Legal move filtering |
| Turn enforcement | Medium | None |
| Fifty-move rule | Medium | None |
| Threefold repetition | Medium | None |

### Planned (v0.4.0 - v0.6.0)

| Version | Feature | Notes |
|---------|---------|-------|
| v0.4.0 | Reference search agent (C++) | Minimax + alpha-beta, links libfenrir.so directly. Required before UCI. |
| v0.5.0 | UCI protocol (C++) | Wraps reference agent. Unlocks chess.com, Lichess, Arena, Stockfish benchmarking. |
| v0.6.0 | Python bindings | ctypes or pybind11. Unlocks Python RL agents. |
| v0.6.0 | WebAssembly build | Browser agents and web UIs. |

> **Current limitation**: `generateMoves()` returns **pseudo-legal moves** - moves that follow piece movement rules but may leave the king in check. External consumers must be aware of this.

---

## Architecture

### Folder Structure

```
fenrir/
├── include/              # Public API headers
│   ├── abstract/
│   │   └── board.h      # AbstractBoard interface (Moves <-> Board decoupling seam)
│   ├── chess/
│   │   ├── board.h      # Board: 12 bitboards (uint64_t bb[12]) — v0.3.0 rewrite in progress
│   │   ├── attacks.h    # Attack tables + slider generation (new in v0.3.0)
│   │   ├── piece.h      # Piece: value type, reconstructed from bitboards on demand
│   │   ├── move.h       # Move: from/to/MoveType/promotionPiece
│   │   ├── moves.h      # Moves singleton: pseudo-legal move generation
│   │   └── fen.h        # Fen: FEN string parsing and generation
│   ├── core/
│   │   └── core.h       # Constants, MoveType enum, GameMode enum
│   ├── engine/
│   │   └── engine.h     # Engine: high-level public API facade
│   ├── logger/          # Logging (levels, rotation)
│   ├── pgn/             # PGN file writing
│   ├── chrono/          # Timestamp utilities
│   ├── modifier/        # String modifiers
│   └── utils/           # Algebraic notation helpers
├── src/                 # Implementations (mirror include/)
├── tests/unit/          # Google Test suites (300 tests)
├── bin/lib/             # libfenrir.so build output
└── scripts/             # build_mode.sh, test.sh, etc.
```

### Key Design Decisions

1. **Bitboard representation** (v0.3.0): `Board` stores state as 12 `uint64_t` (one per piece-type/color). Bit n = square n (a1=0, h8=63). `getPiece(rank, file)` reconstructs piece info on demand by probing the bitboards. Before v0.3.0 this was `vector<vector<unique_ptr<Piece>>>`.
2. **Precomputed attack tables** (`attacks.h/cpp`): knight, king, pawn push/attack tables indexed by square. Slider attacks (rook, bishop, queen) use classical ray blocker algorithm. Initialized once at startup.
3. **`AbstractBoard` interface** (`include/abstract/board.h`) exposes `getPiece()`, `getEnPassant()`, and (v0.3.0) `getCastlingRights()`, `getColor()`, `getBB()`, `getOccupancy()`, `getEnPassantBB()`. `Moves` depends on this interface only.
4. **Make/unmake** (`Board::applyMove()` / `Board::undoMove()`): saves a delta `UndoState` (changed castling rights, en passant, captured piece index + square). Zero allocation. Used internally for legal move filtering and externally via `Engine::undoMove()`.
5. **`Moves` singleton** — one instance for the process lifetime. All methods are `const`.
6. **`Engine`** is the public facade. External consumers only need `#include "include/engine/engine.h"`.

### Class Responsibilities

| Class | Responsibility | Key Methods |
|-------|---------------|-------------|
| `Engine` | Public API facade | `generateMoves()`, `makeMove()`, `getFen()`, `reset()`, `getBoardView()` |
| `Board` | Board state, piece ownership | `getPiece()`, `move()`, `reset()`, `getFen()`, `print()` |
| `Piece` | Individual piece data | `getRank()`, `getFile()`, `getColor()`, `getAlias()`, `getMoved()` |
| `Moves` | Pseudo-legal move generation | `generateMoves()` (dispatches to piece-specific methods) |
| `Fen` | FEN parsing/generation | `parse()`, `getPlacement()`, `getColor()`, `getEnPassant()`, etc. |
| `Move` | Move data | `getFrom()`, `getTo()`, `getMoveType()`, `getPromotionPiece()`, `toUCINotation()` |
| `AbstractBoard` | Board interface for `Moves` | `getPiece()`, `getEnPassant()` |

---

## Move Rules - What Is and Is Not Supported

### `MoveType` Enum (defined in `include/core/core.h`)

| Value | Meaning | Generated? |
|-------|---------|-----------|
| `NORMAL` | Standard move to empty square | Yes |
| `CAPTURE` | Move to square occupied by opponent | Yes |
| `EN_PASSANT` | Pawn captures en passant via FEN square | Yes |
| `CASTLE_KINGSIDE` | King-side castling (O-O) | Not yet generated |
| `CASTLE_QUEENSIDE` | Queen-side castling (O-O-O) | Not yet generated |
| `PROMOTION` | Pawn reaching rank 1 or 8 | Not yet detected/generated |

### Per-Piece Move Generation

| Piece | Normal | Capture | Special |
|-------|--------|---------|---------|
| **Pawn** | single push, double push from start | diagonal | en passant (done), promotion (not yet) |
| **Rook** | slides 4 directions | stops on first enemy | none |
| **Knight** | 8 L-moves, jumps over pieces | yes | none |
| **Bishop** | slides 4 diagonals | stops on first enemy | none |
| **Queen** | slides 8 directions | stops on first enemy | none |
| **King** | 1 square, 8 directions | yes | castling (not yet) |

### What Pseudo-Legal Means

`generateMoves()` returns moves that are geometrically valid for the piece but does **not** validate:
- Whether the resulting position leaves the moving side's king in check
- Whether it is the correct side's turn
- Game-over conditions (checkmate, stalemate)

---

## Technical Details

### Build

```bash
make              # Debug build
make release      # Optimized build (-O2 -DNDEBUG)
make test         # Run all 300 unit tests
make coverage     # Generate HTML coverage (must be 100%)
make clean        # Remove all build artifacts
```

### Testing

- **Framework**: Google Test
- **Count**: 300 tests, 9 skipped in CI environment (env-gated)
- **Coverage**: 100% line coverage enforced - build fails below 100%
- **Suites**: `BoardTest`, `MovesTest`, `EngineTest`, `FenTest`, `PieceTest`, `MoveTest`, `UtilsTest`, `LoggerTest`, `ChronoTest`, `ModifierTest`, `PgnTest`
- Run single test: `./bint/unit/tests --gtest_filter=SuiteName.TestName`
- Coverage report: `.coverage/report/index.html`

### Coordinate System

- **Ranks**: 0-7 (0 = rank 1, 7 = rank 8)
- **Files**: 0-7 (0 = a-file, 7 = h-file)
- **Algebraic notation**: "a1" to "h8" (file letter + rank number, 1-based)

### Piece Aliases

| Alias (upper=White, lower=Black) | Piece |
|----------------------------------|-------|
| `P` / `p` | Pawn |
| `R` / `r` | Rook |
| `N` / `n` | Knight |
| `B` / `b` | Bishop |
| `Q` / `q` | Queen |
| `K` / `k` | King |

---

## Implementing the Next Steps (v0.3.0)

### 1. Check Detection (implement first - everything depends on it)

Create `Board::isInCheck(int color) const`:

- For every opponent piece on the board, call `Moves::generateMoves()`
- If any generated move targets the square occupied by the `color` king - it is in check
- Keep `Moves` decoupled from `Board` - check detection should go in `Board` or a new `Rules` class
- `AbstractBoard` may need `getCurrentColor()` added for downstream use

### 2. Legal Move Filtering

After check detection exists, filter in `Engine::generateMoves()`:
- For each pseudo-legal move, apply it to a board copy
- If the resulting position has the moving side's king in check - discard
- This makes `generateMoves()` return fully legal moves

### 3. Castling

- Prerequisite: check detection
- In `Moves::generateKingMoves()`: read castling rights from FEN via `board.getCastlingRights()`
  - `AbstractBoard` needs `getCastlingRights()` added
- Conditions: king not in check, path squares empty and not attacked, rook/king have not moved
- Emit `Move(kingFrom, kingTo, MoveType::CASTLE_KINGSIDE)` or `CASTLE_QUEENSIDE`
- `Board::move()` must also move the rook when handling a castling move type

### 4. Pawn Promotion

- In `Moves::generatePawnMoves()`: detect if destination rank is 0 (black) or 7 (white)
- Emit four `Move` objects with `MoveType::PROMOTION` and `promotionPiece` = `'Q'`, `'R'`, `'B'`, `'N'`
- `Board::move()` must handle promotion by replacing the pawn with the chosen piece
- `Move::promotionPiece` char field already exists in the data model - no struct changes needed

### 5. Checkmate / Stalemate

```cpp
bool Engine::isCheckmate() const  // In check AND no legal moves
bool Engine::isStalemate() const  // Not in check AND no legal moves
```

Both require legal move generation to already work.

---

## Code Conventions

- **Naming**: camelCase for methods/parameters, PascalCase for classes, UPPER_SNAKE_CASE for constants
- **Files**: `lowercase.cpp` / `lowercase.h`
- **Coverage**: every new line of code must be exercised by tests
- **No CMake**: GNU Make only

---

## Common Pitfalls

1. **Do not break 100% coverage** - every new code path needs a test
2. **Do not add CMake** - GNU Make only
3. **`Board` is not copyable** - Rule of Five, copy/move deleted
4. **All `Piece*` from `getPiece()` are non-owning** - never `delete` them
5. **Moves are pseudo-legal** - check validation not yet implemented
6. **`AbstractBoard`** is the interface `Moves` uses - do not let `Moves` depend on `Board` directly
7. **FEN castling rights** tracked as a string in `Fen` - "KQkq", "-", etc.

---

## Important Files

| File | Purpose | When to Modify |
|------|---------|----------------|
| `include/engine/engine.h` | Public API | Adding high-level features |
| `include/core/core.h` | Constants, MoveType, GameMode enums | Extending enums or constants |
| `src/chess/moves.cpp` | Pseudo-legal move generation | Fixing/adding movement logic |
| `include/abstract/board.h` | Board interface for Moves | Adding methods needed by move generation |
| `src/chess/board.cpp` | Board state management | Modifying board behavior |
| `tests/unit/moves.test.cpp` | Move generation tests | Every moves.cpp change |
| `tests/unit/board.test.cpp` | Board tests | Every board.cpp change |
| `README.md` | User documentation | API or feature changes |
| `AI_CONTEXT.md` | This file | After any significant change |

---

## Summary for AI

**What Fenrir is**: The rules and validation layer of a three-tier chess ecosystem. Layer 1 (Fenrir) serves Layer 2 (agents in Python/JS/any language doing search and learning) which serves Layer 3 (UIs - mobile, web, desktop). Fenrir owns correctness and performance so every layer above it can trust it completely.

**Key performance constraint**: Agents doing RL training or minimax search call `generateMoves()` tens of millions of times. Every allocation inside that call is multiplied by that volume. Make/unmake (zero-allocation move apply and undo) is not an optimization - it is a requirement for Fenrir to be useful as a search oracle.

**Key architectural constraint**: `Moves` depends on `AbstractBoard`, not `Board`. Keep this separation.

**Most important next step**: `AbstractBoard` extensions (`getCastlingRights`, `getColor`) - they unblock check detection, which unblocks everything else.
