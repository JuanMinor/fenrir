# 🐺 Fenrir Chess Engine - Release Notes

## Version 0.2.0 - "Complete Movement" (November 8, 2025)

🎉 **All Piece Movement Implemented!** - Complete pseudo-legal move generation for all 6 chess piece types.

### 🌟 What's New

This release completes the foundational piece movement logic, implementing all remaining chess pieces (Rook, Knight, Bishop, Queen, King) with elegant algorithms and comprehensive testing.

**Major Achievement**: Fenrir can now generate pseudo-legal moves for every piece type in chess, making it a complete foundation for building chess applications.

### ✨ New Features in v0.2.0

**Complete Piece Movement Logic:**
- ♜ **Rook**: Horizontal and vertical sliding with blocking detection
- ♞ **Knight**: L-shaped jumps that can leap over other pieces
- ♗ **Bishop**: Diagonal sliding with blocking detection
- ♛ **Queen**: Combined rook + bishop movement (8 directions)
- ♚ **King**: Single-square movement in all 8 directions

**Technical Highlights:**
- **Elegant `__slide__()` Algorithm**: Shared sliding logic for Rook, Bishop, Queen, and King
- **Smart King Implementation**: Reuses Queen logic with single-depth limitation
- **Comprehensive Testing**: Expanded to 262 unit tests (up from 157)
- **100% Coverage Maintained**: All new code fully tested
- **Performance Optimized**: Efficient move generation with proper boundary checking

### ✅ All Features (Cumulative)

**Chess Logic:**
- ♟️ **Pawns**: Single/double moves, diagonal captures, en passant (v0.1.0)
- ♜ **Rooks**: Horizontal/vertical sliding (v0.2.0)
- ♞ **Knights**: L-shaped jumps (v0.2.0)
- ♗ **Bishops**: Diagonal sliding (v0.2.0)
- ♛ **Queens**: Combined rook + bishop (v0.2.0)
- ♚ **Kings**: Single-square movement (v0.2.0)

**Infrastructure:**
- **FEN Parser**: Complete Forsyth-Edwards Notation support
- **Board Representation**: 8x8 board with piece management
- **Move Generation**: Pseudo-legal moves for all pieces
- **Logging System**: Production-ready with rotation and levels
- **PGN Recording**: Basic game notation support
- **Build System**: Make with debug/release modes
- **Testing**: 262 tests with 100% coverage requirement

### 🔧 Technical Details

**Code Quality:**
- **C++20 Standard**: Modern C++ with proper OOP design
- **262 Unit Tests**: Up from 157 in v0.1.0
- **100% Code Coverage**: Strictly enforced by build system
- **Singleton Pattern**: Used for `Moves` class
- **RAII Throughout**: Proper resource management

**Build System:**
- GNU Make with debug/release modes
- Coverage reporting with lcov
- Shared library output (`libfenrir.so`)
- Comprehensive test suite integration

### 📦 Build Artifacts
- `bin/lib/libfenrir.so` - Shared library for integration
- `include/` - Complete public API headers
- `tests/unit/` - 262 comprehensive unit tests
- `AI_CONTEXT.md` - Technical documentation for AI assistants

### 🚧 Current State & Limitations

**✅ What Works:**
- ✅ All 6 piece types can generate pseudo-legal moves
- ✅ Complete FEN parsing and generation
- ✅ Board representation and piece management
- ✅ En passant captures for pawns
- ✅ Move generation for any position
- ✅ Comprehensive logging and debugging

**❌ What's Missing (planned for future releases):**
- ❌ **Castling**: King-rook castling not implemented
- ❌ **Check Detection**: Cannot detect if king is in check
- ❌ **Move Validation**: Generates pseudo-legal moves only (may leave king in check)
- ❌ **Checkmate/Stalemate**: No game-ending condition detection
- ❌ **Pawn Promotion**: Pawns reaching end rank don't promote
- ❌ **AI Search**: No minimax, alpha-beta, or position evaluation
- ❌ **UCI Protocol**: Cannot interface with chess GUIs
- ❌ **Game State**: No turn management or rule enforcement

> **⚠️ Critical**: This library generates **pseudo-legal moves** (follows piece movement rules but doesn't validate chess game rules). It's a foundation, not a playable chess game.

### 🛠️ Building from Source

**Prerequisites:**
- GCC/G++ with C++20 support
- GNU Make
- Google Test framework
- lcov (for coverage)

**Quick Start:**
```bash
# Clone and build
git clone <repository-url>
cd fenrir
make

# Run all 262 tests
make test

# Generate coverage report
make coverage

# Build optimized release
make release
```

### 🎯 Development Roadmap

**Completed:**
- ✅ v0.1.0: Foundation (board, FEN, pawn movement)
- ✅ v0.2.0: Complete piece movement logic

**Upcoming:**
- 🚧 v0.3.0: Castling, check detection, move validation
- 🚧 v0.4.0: Checkmate/stalemate, pawn promotion
- 🔮 v0.5.0: Basic AI (minimax with alpha-beta)
- 🔮 v0.6.0: UCI protocol implementation
- 🔮 v1.0.0: Full-featured chess engine

### 🐛 Known Issues
- None in implemented features
- See roadmap for missing features

### 🏆 Quality Metrics
- **Test Coverage**: 100% line coverage (enforced)
- **Test Count**: 262 comprehensive unit tests
- **Test Suites**: 10 (FenTest, BoardTest, PieceTest, MovesTest, etc.)
- **Memory Safety**: Zero memory leaks (RAII throughout)
- **Build System**: Make with debug/release modes
- **Documentation**: README.md, AI_CONTEXT.md, inline docs

### 📞 Getting Started

**What you can do with v0.2.0:**
- ✅ Generate pseudo-legal moves for any piece type
- ✅ Parse and generate FEN positions
- ✅ Build chess puzzles and position analyzers
- ✅ Study piece movement algorithms
- ✅ Use as foundation for chess applications

**What you cannot do (yet):**
- ❌ Play a complete chess game (no rule enforcement)
- ❌ Detect illegal moves (no check validation)
- ❌ Use as chess engine (no AI)

### 🙏 Acknowledgments

This project demonstrates modern C++ development practices and serves as a foundation for chess programming education and development.

---

## Version 0.1.0 - "Foundation" (June 22, 2025)

🎉 **First Release!** - The foundation of Fenrir Chess Engine

### 🌟 What's New
Initial release establishing core architecture and infrastructure.

### ✨ Features Implemented
- **Chess Board Representation**: 8x8 board with piece management
- **FEN Parser**: Complete FEN string parsing and generation
- **Pawn Movement**: Single/double moves, captures, en passant
- **Logging System**: Production-ready with rotation and levels
- **PGN Recording**: Basic game notation support
- **Build System**: Make with debug/release modes
- **Testing**: 157 unit tests with 100% coverage

### 🏆 Quality Metrics
- 157 unit tests
- 100% code coverage
- Zero memory leaks
- Modern C++20

---

**Note**: Fenrir is a **foundation library** for chess applications. It provides piece movement logic but does not enforce chess game rules (no check detection, castling, or checkmate). Use it as a building block, not a complete chess game.