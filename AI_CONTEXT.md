# 🤖 AI Context Document - Fenrir Chess Engine

> **Purpose**: This document provides AI assistants with complete context about the Fenrir chess engine project. Read this first to understand the codebase, architecture, and development goals.

---

## 📋 Project Overview

**Fenrir** is a C++ chess engine library that provides:
- Complete chess board representation and piece management
- FEN (Forsyth-Edwards Notation) parsing and generation
- Move generation for all 6 chess piece types
- Shared library (`libfenrir.so`) for integration into other projects
- Comprehensive logging and debugging infrastructure

**Current Status**: Foundation complete with all piece movement logic implemented. Ready for game state management and AI search algorithms.

---

## 🎯 Project Goals & Roadmap

### ✅ Completed (Current State)
1. **Chess Board Representation**: 8x8 board with proper piece management
2. **FEN Parser**: Complete FEN string parsing and generation
3. **All Piece Movement Logic**:
   - ♟️ Pawns: Single/double moves, diagonal captures, en passant
   - ♜ Rooks: Horizontal/vertical sliding with blocking
   - ♞ Knights: L-shaped jumps (can jump over pieces)
   - ♗ Bishops: Diagonal sliding with blocking
   - ♛ Queens: Combined rook + bishop movement
   - ♚ Kings: Single-square movement in all 8 directions
4. **Move Generation**: Legal move generation for all pieces
5. **Logging System**: Production-ready logging with rotation and levels
6. **PGN Recording**: Basic PGN file support
7. **Build System**: Makefile with debug/release modes
8. **Testing**: 262 unit tests with 100% coverage requirement

### 🚧 Not Yet Implemented (Future Work)
1. **Castling**: King-rook castling moves (kingside/queenside)
2. **Check Detection**: Determining if king is in check
3. **Checkmate Detection**: Game-ending conditions
4. **Move Validation**: Preventing moves that leave king in check
5. **Stalemate Detection**: Draw conditions
6. **Promotion**: Pawn promotion to queen/rook/bishop/knight
7. **AI Search**: Minimax, alpha-beta pruning, position evaluation
8. **UCI Protocol**: Universal Chess Interface for GUI integration
9. **Opening Book**: Database of opening moves
10. **Endgame Tablebases**: Perfect play in endgames

---

## 🏗️ Architecture

### Core Components

```
fenrir/
├── include/              # Public API headers
│   ├── chess/           # Chess-specific logic
│   │   ├── board.h      # Board representation and management
│   │   ├── piece.h      # Piece class (position, color, type)
│   │   ├── moves.h      # Move generation (singleton pattern)
│   │   └── fen.h        # FEN parsing and generation
│   ├── engine/          # Main engine interface
│   │   └── engine.h     # High-level API for users
│   ├── logger/          # Logging system
│   ├── pgn/             # PGN file handling
│   ├── chrono/          # Timestamp utilities
│   ├── modifier/        # String modifiers
│   └── utils/           # Utility functions
├── src/                 # Implementation files (mirrors include/)
├── tests/unit/          # Google Test unit tests (262 tests)
├── bin/lib/             # Build output (libfenrir.so)
└── scripts/             # Build automation scripts
```

### Key Design Patterns

1. **Singleton Pattern**: `Moves` class uses singleton for move generation
2. **RAII**: Proper resource management throughout
3. **Shared Library**: Compiled as `.so` for reusability
4. **Separation of Concerns**: Clear boundaries between board, pieces, moves, engine

### Class Responsibilities

| Class | Responsibility | Key Methods |
|-------|---------------|-------------|
| `Engine` | High-level API facade | `make_move()`, `generate_moves()`, `get_fen()`, `reset()` |
| `Board` | Board state management | `get_piece()`, `move()`, `print()`, `get_fen()` |
| `Piece` | Individual piece data | `get_rank()`, `get_file()`, `get_color()`, `get_alias()` |
| `Moves` | Move generation logic | `generate_moves()` (delegates to piece-specific methods) |
| `Fen` | FEN parsing/generation | `parse()`, `get_placement()`, `get_color()`, etc. |

---

## 🔧 Technical Details

### Build System
- **Primary**: GNU Make (only supported build system)
- **Compiler**: GCC/G++ with C++20 standard
- **Flags**: `-std=c++20 -fPIC -DFENRIR_BUILD_DLL`
- **Debug Mode**: `-g` (default)
- **Release Mode**: `-O2 -DNDEBUG`

### Build Commands
```bash
make              # Build debug version (default)
make release      # Build optimized release
make test         # Run 262 unit tests
make coverage     # Generate coverage report (requires 100%)
make clean        # Remove all build artifacts
make help         # Show all available targets
```

### Testing Requirements
- **Framework**: Google Test
- **Coverage**: 100% line coverage REQUIRED (build fails if < 100%)
- **Test Count**: 262 tests across 10 test suites
- **Test Files**: `tests/unit/*.test.cpp`

### Move Generation Algorithm

**Sliding Pieces** (Rook, Bishop, Queen, King):
- Uses shared `__slide__()` algorithm with direction vectors
- Rook: 4 directions (horizontal/vertical)
- Bishop: 4 directions (diagonals)
- Queen: 8 directions (rook + bishop)
- King: 8 directions with `single_depth=true` (1 square only)

**Knight**:
- 8 L-shaped moves from current position
- Can jump over other pieces
- Validates boundaries and captures

**Pawn**:
- Direction based on color (white=+1, black=-1)
- Single move forward (if not blocked)
- Double move from starting position (if not moved)
- Diagonal captures (left and right)
- En passant capture (special case)

---

## 📝 Code Conventions

### Naming Conventions
- **Private methods**: `__method_name__()` (double underscore prefix/suffix)
- **Parameters**: `__parameter_name` (double underscore prefix)
- **Classes**: PascalCase (`Board`, `Piece`, `Engine`)
- **Constants**: UPPER_SNAKE_CASE (`BOARD_SIZE`, `WHITE`, `BLACK`)
- **Files**: lowercase with extension (`.h`, `.cpp`)

### Piece Representation
- **Aliases**: `P/p` (pawn), `R/r` (rook), `N/n` (knight), `B/b` (bishop), `Q/q` (queen), `K/k` (king)
- **Color**: Uppercase = White, Lowercase = Black
- **Color Constants**: `WHITE = 0`, `BLACK = 1`

### Coordinate System
- **Ranks**: 0-7 (0 = rank 1, 7 = rank 8)
- **Files**: 0-7 (0 = a-file, 7 = h-file)
- **Algebraic Notation**: "e4", "a1", "h8" (file + rank)

---

## 🔍 What Works vs What Doesn't

### ✅ What Currently Works
1. **Board Setup**: Initialize from FEN string
2. **Piece Movement**: All 6 piece types can move according to chess rules
3. **Move Generation**: Generate all pseudo-legal moves for any piece
4. **Captures**: Pieces can capture opponent pieces
5. **En Passant**: Pawn en passant captures work
6. **FEN Export**: Export current position to FEN string
7. **Board Display**: Print ASCII board to console
8. **Logging**: Comprehensive debug/info/warn/error logging

### ❌ What Doesn't Work Yet
1. **Castling Moves**: Cannot castle king and rook
2. **Check Validation**: Doesn't prevent moving into check
3. **Checkmate Detection**: Cannot detect game-ending positions
4. **Pawn Promotion**: Pawns reaching end rank don't promote
5. **Stalemate**: No draw detection
6. **Move Validation**: Generates pseudo-legal moves (may leave king in check)
7. **Game State**: No turn management or game-over detection
8. **AI**: No computer opponent or position evaluation

---

## 🚀 How to Extend the Project

### Adding Castling Support
1. Modify `Moves::__king__()` to check castling rights
2. Verify king and rook haven't moved
3. Ensure squares between are empty
4. Validate king doesn't castle through check
5. Add tests in `tests/unit/moves.test.cpp`

### Adding Check Detection
1. Create `Board::is_in_check(color)` method
2. For each opponent piece, generate moves
3. Check if any move targets the king
4. Add tests for various check scenarios

### Adding Move Validation
1. Before making a move, simulate it on a copy of the board
2. Check if the moving side's king is in check
3. If yes, reject the move
4. Update `Engine::make_move()` to validate

### Adding Pawn Promotion
1. In `Board::move()`, detect pawn reaching rank 0 or 7
2. Prompt for promotion piece (Q/R/B/N)
3. Replace pawn with chosen piece
4. Update FEN generation to handle promoted pieces

---

## 📊 Current Metrics

- **Lines of Code**: ~4,240 (test files)
- **Test Count**: 262 unit tests
- **Test Suites**: 10 (FenTest, BoardTest, PieceTest, EngineTest, MovesTest, etc.)
- **Coverage**: 100% required (enforced by build system)
- **Build Artifacts**: `libfenrir.so` (shared library)
- **Dependencies**: Google Test, lcov (for coverage)

---

## 🛠️ Development Workflow

### Making Changes
1. **Read this document first** to understand the architecture
2. **Check existing tests** in `tests/unit/` to understand expected behavior
3. **Write tests first** for new functionality (TDD approach)
4. **Implement changes** in `src/` and `include/`
5. **Run tests**: `make test` (must pass all 262 tests)
6. **Check coverage**: `make coverage` (must be 100%)
7. **Update documentation** if API changes

### Common Tasks

**Add a new piece type** (hypothetical):
1. Add alias to `PIECE_NAMES` map
2. Create `__new_piece__()` method in `Moves` class
3. Add case in `Moves::generate_moves()` switch statement
4. Write comprehensive tests
5. Update documentation

**Fix a bug**:
1. Write a failing test that reproduces the bug
2. Fix the implementation
3. Verify test passes
4. Check coverage remains 100%
5. Document the fix in commit message

**Add a feature**:
1. Update `AI_CONTEXT.md` roadmap (move from "Not Yet Implemented" to "In Progress")
2. Write tests for the feature
3. Implement the feature
4. Update `README.md` with new API
5. Update `RELEASE_NOTES.md` for next version

---

## 🎓 Key Concepts for AI Assistants

### When Asked to "Add Castling"
- Castling is a special king move that also moves the rook
- Kingside: King moves 2 squares toward h-file, rook jumps over
- Queenside: King moves 2 squares toward a-file, rook jumps over
- Requirements: Neither piece has moved, no pieces between, king not in check, doesn't castle through check
- FEN castling rights: "KQkq" (White kingside, White queenside, Black kingside, Black queenside)

### When Asked to "Add Check Detection"
- Check = opponent can capture the king on next move
- Iterate through all opponent pieces, generate their moves
- If any move targets the king's square, it's check
- This is needed before implementing move validation

### When Asked to "Add AI"
- Start with minimax algorithm (recursive search)
- Add alpha-beta pruning for efficiency
- Implement position evaluation function (material, position, mobility)
- Consider iterative deepening for time management
- This requires move validation and check detection first

### When Asked to "Fix Tests"
- Tests are in `tests/unit/*.test.cpp`
- Use Google Test framework (`TEST_F`, `EXPECT_EQ`, `ASSERT_NE`)
- Run specific test: `./bint/unit/tests --gtest_filter=TestSuite.TestName`
- Coverage report: `.coverage/report/index.html`

---

## 📚 Important Files to Know

| File | Purpose | When to Modify |
|------|---------|----------------|
| `include/engine/engine.h` | Public API | Adding new high-level features |
| `src/chess/moves.cpp` | Move generation | Fixing/adding piece movement logic |
| `include/chess/board.h` | Board interface | Adding board state features |
| `src/chess/board.cpp` | Board implementation | Modifying board behavior |
| `tests/unit/moves.test.cpp` | Move tests | Testing piece movement |
| `Makefile` | Build configuration | Changing build process |
| `README.md` | User documentation | Documenting API changes |
| `AI_CONTEXT.md` | This file | Updating project context |

---

## ⚠️ Common Pitfalls

1. **Don't break 100% coverage**: Every new line must be tested
2. **Don't modify build system**: Stick with Make (no CMake, etc.)
3. **Don't change naming conventions**: Follow `__private__()` pattern
4. **Don't skip tests**: Write tests before implementation
5. **Don't assume move validation**: Current moves are pseudo-legal only
6. **Don't forget en passant**: It's implemented but easy to overlook
7. **Don't hardcode board size**: Use `BOARD_SIZE` constant (8)

---

## 🎯 Quick Start for AI Assistants

**Scenario: User asks to add a feature**

1. ✅ Read this document to understand current state
2. ✅ Check if feature is in "Not Yet Implemented" section
3. ✅ Identify which files need modification
4. ✅ Write tests first in appropriate `tests/unit/*.test.cpp`
5. ✅ Implement in `src/` and `include/`
6. ✅ Run `make test` to verify
7. ✅ Run `make coverage` to ensure 100%
8. ✅ Update documentation (README.md, this file, RELEASE_NOTES.md)

**Scenario: User asks to fix a bug**

1. ✅ Reproduce the bug with a test
2. ✅ Identify the buggy code in `src/`
3. ✅ Fix the implementation
4. ✅ Verify test passes
5. ✅ Check coverage remains 100%

**Scenario: User asks "What does this project do?"**

1. ✅ Refer to "Project Overview" section
2. ✅ Explain current capabilities (all piece movement)
3. ✅ Clarify limitations (no check detection, castling, AI, etc.)
4. ✅ Reference the roadmap for future plans

---

## 📞 Summary for AI

**In one sentence**: Fenrir is a C++ chess library with complete piece movement logic but no game state validation, check detection, or AI - it's a foundation ready for advanced chess features.

**Key takeaway**: The engine can move pieces legally but doesn't enforce chess rules like preventing check, detecting checkmate, or handling castling/promotion. It's a building block, not a complete chess game.

**Next logical steps**: Implement check detection → move validation → castling → checkmate detection → AI search algorithms → UCI protocol.

---

*Last Updated: 2025-11-08*  
*Project Version: 0.2.0-dev (post-foundation, pre-game-rules)*  
*License: MIT (see LICENSE file)*
