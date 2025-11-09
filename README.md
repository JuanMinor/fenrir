# 🐺 Fenrir Chess Engine

A foundational C++ chess library providing board representation, FEN parsing, and complete piece movement logic for all 6 chess piece types.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Build](https://img.shields.io/badge/Build-Make-green.svg)](https://www.gnu.org/software/make/)
[![Testing](https://img.shields.io/badge/Testing-Google%20Test-red.svg)](https://github.com/google/googletest)
[![Coverage](https://img.shields.io/badge/Coverage-100%25-brightgreen.svg)]()
[![Tests](https://img.shields.io/badge/Tests-262%20passing-success.svg)]()

> **⚠️ Current Status**: This is a **foundation library** with complete piece movement logic but **no game rule enforcement** (no check detection, castling, or checkmate). It's a building block for chess applications, not a playable chess game.

> **🔧 Build System**: GNU Make is the only supported build system. See [Build System](#build-system) for details.

> **🐳 Development**: Uses dev containers for consistent environments. VS Code recommended. See [Development Environment](#development-environment).

## ⚡ Quick Start

```bash
# Clone and build the library
git clone <repository-url>
cd fenrir
make

# Run the example program
./scripts/run.sh

# Run all 262 tests
make test

# Generate coverage report (requires 100%)
make coverage

# For integration, see "Using the Library" section below
```

## ✨ What Fenrir Does (And Doesn't Do)

### ✅ Implemented Features

**Complete Piece Movement Logic** - All 6 chess piece types can generate pseudo-legal moves:

| Piece | Movement Implementation | Special Features |
|-------|------------------------|------------------|
| ♟️ **Pawn** | ✅ Complete | Single/double moves, diagonal captures, **en passant** |
| ♜ **Rook** | ✅ Complete | Horizontal/vertical sliding, blocking detection |
| ♞ **Knight** | ✅ Complete | L-shaped jumps, can jump over pieces |
| ♗ **Bishop** | ✅ Complete | Diagonal sliding, blocking detection |
| ♛ **Queen** | ✅ Complete | Combined rook + bishop (8 directions) |
| ♚ **King** | ✅ Complete | Single-square movement in all 8 directions |

**Other Working Features:**
- 📋 **FEN Support**: Parse and generate Forsyth-Edwards Notation
- 🎯 **Move Generation**: Generate all pseudo-legal moves for any piece
- 📦 **Shared Library**: `libfenrir.so` for integration
- 🪵 **Logging System**: Production-ready with rotation and levels
- 📝 **PGN Recording**: Basic game notation support
- ✅ **100% Test Coverage**: 262 unit tests, all passing

### ❌ Not Yet Implemented

**Critical Missing Features** (prevents this from being a playable chess game):
- 🚫 **No Castling**: King-rook castling moves not implemented
- 🚫 **No Check Detection**: Doesn't know when king is in check
- 🚫 **No Move Validation**: Allows moves that leave king in check (pseudo-legal only)
- 🚫 **No Checkmate/Stalemate**: Cannot detect game-ending conditions
- 🚫 **No Pawn Promotion**: Pawns reaching end rank don't promote
- 🚫 **No AI**: No computer opponent or position evaluation
- 🚫 **No UCI Protocol**: Cannot interface with chess GUIs

> **⚠️ Important**: This library generates **pseudo-legal moves** (moves that follow piece movement rules but may leave the king in check). It does NOT enforce chess game rules.

## 🎯 Use Cases

**What You Can Build With Fenrir:**
- 🎓 **Learning Tool**: Study chess piece movement algorithms
- 🧩 **Chess Puzzle Solver**: Generate moves for puzzle positions
- 📊 **Position Analyzer**: Analyze FEN positions and mobility
- 🔧 **Chess App Foundation**: Build upon this library to create a full chess game
- 🧪 **Testing Framework**: Test chess-related algorithms

**What You Cannot Do (Yet):**
- ❌ Play a complete chess game (no rule enforcement)
- ❌ Detect illegal moves (no check validation)
- ❌ Use as a chess engine (no AI or evaluation)
- ❌ Connect to chess GUIs (no UCI protocol)

## API Overview

### Basic Usage

```cpp
#include "include/engine/engine.h"

// Initialize with default starting position
fenrir::Engine engine;

// Make moves using algebraic notation (recommended)
engine.make_move("e2", "e4");  // Pawn e2 to e4
engine.make_move("e7", "e5");  // Pawn e7 to e5
engine.make_move("g1", "f3");  // Knight g1 to f3
engine.make_move("b8", "c6");  // Knight b8 to c6

// Alternative: coordinate-based moves (rank, file - 0-indexed)
engine.make_move(1, 4, 3, 4);  // Same as e2-e4

// Generate legal moves for any piece type
auto pawn_moves = engine.generate_moves("e4");	// Pawn moves
auto knight_moves = engine.generate_moves("f3");  // Knight moves
auto all_moves = engine.generate_moves("d1");	 // Queen moves

// Display current board state
engine.print_board();

// Reset to starting position
engine.reset();
```

### Key Classes and Methods

- `fenrir::Engine`: Main interface for chess operations
- `engine.make_move(from, to)`: Execute a move for any piece type
- `engine.generate_moves(square)`: Get legal moves for any piece (pawns, rooks, knights, bishops, queens, kings)
- `engine.get_fen()`: Get current position in FEN notation
- `engine.print_board()`: Display current position
- `engine.reset()`: Return to starting position

**⚠️ Important Notes:**
- Moves are **pseudo-legal** (follow piece rules but may leave king in check)
- No turn validation (can move opponent's pieces)
- No game-over detection
- See `AI_CONTEXT.md` for complete technical details

## Build System

**Fenrir uses GNU Make as the primary and currently supported build system.**

| Target		  | Description									   |
| --------------- | ------------------------------------------------- |
| `make`		  | Build shared library (`bin/lib/libfenrir.so`)	 |
| `make debug`	| Explicitly build in debug mode (default)		  |
| `make release`  | Build optimized release version				   |
| `make test`	 | Run unit tests with Google Test				   |
| `make coverage` | Generate coverage report (requires 100% coverage) |
| `make clean`	| Clean build artifacts							 |
| `make help`	 | Show detailed help with all available targets	 |

### Building the Library

```bash
# Build debug version (default)
make

# Build optimized release version
make release

# The shared library will be created at:
# bin/lib/libfenrir.so
```

## Using the Library

### Step-by-Step Integration Guide

1. **Build the Fenrir library**:

   ```bash
   git clone <repository-url>
   cd fenrir
   make release  # or just 'make' for debug version
   ```

2. **Create your application** (e.g., `my_chess_app.cpp`):

   ```cpp
   #include <iostream>
   #include <vector>
   #include "include/engine/engine.h"

   int main() {
	   try {
		   // Initialize the chess engine
		   fenrir::Engine engine;

		   // Display the initial board
		   std::cout << "Initial position:" << std::endl;
		   engine.print_board();

		   // Demonstrate movement for different piece types
		   engine.make_move("e2", "e4");  // White pawn to e4
		   engine.make_move("e7", "e5");  // Black pawn to e5
		   engine.make_move("g1", "f3");  // White knight to f3
		   engine.make_move("b8", "c6");  // Black knight to c6
		   engine.make_move("f1", "c4");  // White bishop to c4

		   std::cout << "\nAfter multiple piece movements:" << std::endl;
		   engine.print_board();

		   // Generate legal moves for different piece types
		   auto knight_moves = engine.generate_moves("f3");
		   std::cout << "\nLegal moves for white knight on f3:" << std::endl;
		   for (const auto& move : knight_moves) {
			   std::cout << move.first << " -> " << move.second << std::endl;
		   }

		   auto bishop_moves = engine.generate_moves("c4");
		   std::cout << "\nLegal moves for white bishop on c4:" << std::endl;
		   for (const auto& move : bishop_moves) {
			   std::cout << move.first << " -> " << move.second << std::endl;
		   }

		   // Reset to starting position
		   engine.reset();
		   std::cout << "\nBoard reset to starting position." << std::endl;

	   } catch (const std::exception& e) {
		   std::cerr << "Error: " << e.what() << std::endl;
		   return 1;
	   }

	   return 0;
   }
   ```

3. **Compile and link against the library**:

   ```bash
   # Method 1: Direct linking (recommended)
   g++ -I./fenrir my_chess_app.cpp -L./fenrir/bin/lib -lfenrir -o my_chess_app

   # Method 2: With runtime library path
   g++ -I./fenrir my_chess_app.cpp -L./fenrir/bin/lib -lfenrir -Wl,-rpath,./fenrir/bin/lib -o my_chess_app

   # Method 3: Using pkg-config style (if you set LD_LIBRARY_PATH)
   export LD_LIBRARY_PATH=./fenrir/bin/lib:$LD_LIBRARY_PATH
   g++ -I./fenrir my_chess_app.cpp -L./fenrir/bin/lib -lfenrir -o my_chess_app
   ```

4. **Run your application**:

   ```bash
   # If you used Method 1 or 2
   ./my_chess_app

   # If you used Method 3 (ensure LD_LIBRARY_PATH is set)
   LD_LIBRARY_PATH=./fenrir/bin/lib:$LD_LIBRARY_PATH ./my_chess_app
   ```

### Compilation Flags Explained

- `-I./fenrir`: Include directory for Fenrir headers
- `-L./fenrir/bin/lib`: Library search directory
- `-lfenrir`: Link against libfenrir.so
- `-Wl,-rpath,./fenrir/bin/lib`: Embed library path in executable (optional)

### Example Makefile for Your Project

```makefile
# Assuming fenrir is in a subdirectory or alongside your project
FENRIR_DIR = ./fenrir
INCLUDES = -I$(FENRIR_DIR)
LIBS = -L$(FENRIR_DIR)/bin/lib -lfenrir
RPATH = -Wl,-rpath,$(FENRIR_DIR)/bin/lib

my_chess_app: my_chess_app.cpp
	g++ $(INCLUDES) $< $(LIBS) $(RPATH) -o $@

clean:
	rm -f my_chess_app

.PHONY: clean
```

## 📁 Project Structure

```
fenrir/
├── include/              # Public API headers
│   ├── chess/           # Board, pieces, moves, FEN
│   │   ├── board.h      # Board representation
│   │   ├── piece.h      # Piece class
│   │   ├── moves.h      # Move generation (singleton)
│   │   └── fen.h        # FEN parser
│   ├── engine/          # Main engine interface
│   │   └── engine.h     # High-level API
│   ├── logger/          # Logging system
│   ├── pgn/             # PGN file handling
│   ├── chrono/          # Timestamp utilities
│   ├── modifier/        # String modifiers
│   └── utils/           # Utility functions
├── src/                 # Implementation files (mirrors include/)
├── tests/unit/          # Google Test suite (262 tests)
├── bin/lib/             # Build output (libfenrir.so)
├── scripts/             # Build automation
├── Makefile             # Build system
├── README.md            # This file
└── AI_CONTEXT.md        # Technical documentation for AI assistants
```

## 🛠️ Development

### Prerequisites

**Required:**
- GCC/G++ compiler with C++20 support
- GNU Make
- Google Test framework
- lcov (for coverage reporting)

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
- ✅ All tools pre-installed (GCC, Make, GTest, lcov)
- ✅ Integrated debugging with GDB
- ✅ No local setup required

**Option 2: Local Development**

Manually install all prerequisites, then:

```bash
git clone <repository-url>
cd fenrir
make
```

### Testing Requirements

**100% code coverage is REQUIRED:**

```bash
make test      # Run all 262 tests
make coverage  # Generate coverage report (fails if < 100%)
```

**Test Structure:**
- Framework: Google Test
- Location: `tests/unit/*.test.cpp`
- Count: 262 tests across 10 test suites
- Coverage: 100% line coverage enforced

### Build System

**Only GNU Make is supported.** Do not use CMake, Autotools, or other build systems.

```bash
make           # Build debug version (default)
make release   # Build optimized release
make clean     # Remove all build artifacts
make help      # Show all available targets
```

## 📚 Documentation

- **README.md** (this file): User-facing documentation and API reference
- **AI_CONTEXT.md**: Technical documentation for AI assistants (read this first!)
- **RELEASE_NOTES.md**: Version history and changelog

## 🗺️ Roadmap

**Current Version: 0.2.0-dev** (Foundation Complete)

**Next Steps:**
1. ✅ ~~Complete piece movement logic~~ (DONE)
2. 🚧 Implement castling moves
3. 🚧 Add check detection
4. 🚧 Add move validation (prevent illegal moves)
5. 🚧 Implement checkmate/stalemate detection
6. 🚧 Add pawn promotion
7. 🔮 Basic AI (minimax with alpha-beta pruning)
8. 🔮 UCI protocol support
9. 🔮 Opening book and endgame tablebases

## 🤝 Contributing

**Before contributing:**
1. Read `AI_CONTEXT.md` to understand the architecture
2. Write tests first (TDD approach)
3. Ensure 100% coverage (`make coverage`)
4. Follow existing naming conventions (`__private__()` pattern)
5. Update documentation for API changes

## 📄 License

MIT License - see [LICENSE](LICENSE) file.

**Copyright (c) 2025 Juan Minor**

## ⚠️ Disclaimer

This is a **foundation library**, not a complete chess game. It provides piece movement logic but does not enforce chess rules like check, checkmate, or castling. Use it as a building block for chess applications.
