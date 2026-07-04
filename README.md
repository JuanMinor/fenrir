# Fenrir - Chess Rules Engine

Fenrir is a C++ shared library (`libfenrir.so`) that serves as the **rules and validation layer** for a three-tier chess ecosystem. It handles every hard chess problem - legal move generation, FEN/PGN, board state, move validation - so that agents, UIs, and applications built on top of it never have to.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Build](https://img.shields.io/badge/Build-CMake-blue.svg)](https://cmake.org/)
[![Testing](https://img.shields.io/badge/Testing-Google%20Test-red.svg)](https://github.com/google/googletest)
[![Coverage](https://img.shields.io/badge/Coverage-100%25-brightgreen.svg)]()
[![Tests](https://img.shields.io/badge/Tests-359%20passing-success.svg)]()

> **Architecture**: Fenrir is the base of a three-layer system: **Fenrir** (rules, validation, move generation) -> **Agents** (Python, JS, or any language - they search and decide) -> **UIs** (mobile, web, desktop - they present and explain). Fenrir's job is to be correct, fast, and stable so everything built on top of it can be trusted.

> **Current Status**: Move generation is pseudo-legal (complete piece movement logic including en passant). Full legal move enforcement (check detection, castling, promotion, checkmate) is in progress as v0.3.0.

> **🔧 Build System**: CMake is the supported build system. See [Build System](#build-system) for details.

> **🐳 Development**: Uses dev containers for consistent environments. VS Code recommended. See [Development Environment](#development-environment).

## ⚡ Quick Start

```bash
# Clone and build the library
git clone <repository-url>
cd fenrir
make

# Run the example program
./scripts/run.sh

# Run all 300 tests
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
| ♟️ **Pawn** | ✅ Complete | Single/double push, diagonal captures, **en passant** — ❌ no promotion yet |
| ♜ **Rook** | ✅ Complete | Horizontal/vertical sliding, blocking detection |
| ♞ **Knight** | ✅ Complete | L-shaped jumps, can jump over pieces |
| ♗ **Bishop** | ✅ Complete | Diagonal sliding, blocking detection |
| ♛ **Queen** | ✅ Complete | Combined rook + bishop (8 directions) |
| ♚ **King** | ✅ Complete | Single-square movement in all 8 directions — ❌ no castling yet |

**Other Working Features:**
- 📋 **FEN Support**: Parse and generate Forsyth-Edwards Notation
- 🎯 **Move Generation**: Generate all pseudo-legal moves for any piece
- 📦 **Shared Library**: `libfenrir.so` for integration
- 🪵 **Logging System**: Production-ready with rotation and levels
- 📝 **PGN Recording**: Basic game notation support
- ✅ **100% Test Coverage**: 300 unit tests, all passing
- ✅ **Memory Safety**: `unique_ptr<Piece>` ownership, Rule of Five enforced on `Board`
- ✅ **Const Correctness**: All move-generation methods are `const`

### ❌ Not Yet Implemented

**v0.3.0 target — game rule enforcement:**
- 🚫 **No Check Detection**: Doesn't know when a king is in check *(needed first — unlocks everything below)*
- 🚫 **No Legal Move Filtering**: `generateMoves()` is pseudo-legal (may leave king in check)
- 🚫 **No Castling**: `CASTLE_KINGSIDE`/`CASTLE_QUEENSIDE` move types exist but are not generated
- 🚫 **No Pawn Promotion**: `PROMOTION` move type exists but pawns reaching rank 1/8 don't trigger it
- 🚫 **No Checkmate/Stalemate**: No game-ending condition detection

**Future work (beyond v0.3.0):**
- 🚫 **No AI**: No computer opponent or position evaluation
- 🚫 **No UCI Protocol**: Cannot interface with chess GUIs
- 🚫 **No Turn Enforcement**: Can move either side's pieces

> **⚠️ Important**: `generateMoves()` returns **pseudo-legal moves** — moves that follow piece movement rules but may leave the king in check. Legal move filtering (check validation) is planned for v0.3.0.

## The Ecosystem Fenrir Enables

Fenrir is not a standalone chess program. It is the foundation of a larger system:

```
Layer 3 - UIs
  Mobile apps (React Native, Swift)
  Web apps (JS/WASM)
  Desktop apps (Electron)
  Purpose: present, visualize, explain moves to humans

Layer 2 - Agents
  Python bots, JS agents, RL training loops
  Purpose: search, evaluate, decide which moves to make
  They ask Fenrir what is legal; they decide what is best

Layer 1 - Fenrir (this library)
  C++ shared library: libfenrir.so
  Purpose: rules, validation, move generation, FEN, PGN
  Agents and UIs trust Fenrir completely for correctness
```

**Why this matters for Fenrir's design:**
- **Correctness is non-negotiable.** Agents learn from the moves Fenrir says are legal. A wrong answer corrupts training data and every layer above it.
- **Performance is structural.** An agent doing reinforcement learning plays millions of games during training. A minimax search at depth 5 makes roughly 24 million `generateMoves()` calls. Fenrir must handle that workload efficiently.
- **The API must be stable.** Every language binding (Python ctypes, JS WASM, Swift FFI) breaks when the API changes. Breaking changes have cascading costs across all consumers.
- **Fenrir owns the hard chess problems.** FEN, PGN, algebraic notation, legal move generation, move validation - none of this should be reimplemented in agents or UIs.

## API Overview

### Basic Usage

```cpp
#include "include/engine/engine.h"
#include "include/chess/move.h"

// Initialize with default starting position
fenrir::Engine engine;

// Make moves using the Move class
engine.makeMove(fenrir::Move("e2", "e4"));  // Pawn e2 to e4
engine.makeMove(fenrir::Move("e7", "e5"));  // Pawn e7 to e5
engine.makeMove(fenrir::Move("g1", "f3"));  // Knight g1 to f3
engine.makeMove(fenrir::Move("b8", "c6"));  // Knight b8 to c6

// Specify move types for special moves
engine.makeMove(fenrir::Move("e4", "d5", fenrir::MoveType::CAPTURE));  // Capture
engine.makeMove(fenrir::Move("e7", "e8", fenrir::MoveType::PROMOTION, 'Q'));  // Promotion

// Generate legal moves for any piece type (returns vector<Move>)
const std::vector<fenrir::Move> pawn_moves = engine.generateMoves("e4");  // Pawn moves
const std::vector<fenrir::Move> knight_moves = engine.generateMoves("f3");  // Knight moves
const std::vector<fenrir::Move> all_moves = engine.generateMoves("d1");  // Queen moves

// Access move information
for (const auto& move : pawn_moves) {
    std::cout << move.getFrom() << " -> " << move.getTo() << std::endl;
}

// Display current board state
engine.printBoard();

// Reset to starting position
engine.reset();
```

### Key Classes and Methods

- `fenrir::Engine`: Main interface for chess operations
  - `engine.makeMove(const Move&)`: Execute a move using Move object
  - `engine.generateMoves(square)`: Get legal moves (returns `vector<Move>`)
  - `engine.getFen()`: Get current position in FEN notation
  - `engine.printBoard()`: Display current position
  - `engine.reset()`: Return to starting position

- `fenrir::Move`: Represents a chess move
  - Constructor: `Move(from, to, moveType=NORMAL, promotionPiece='\0')`
  - `move.getFrom()`: Get source square (e.g., "e2")
  - `move.getTo()`: Get destination square (e.g., "e4")
  - `move.getMoveType()`: Get move type (NORMAL, CAPTURE, EN_PASSANT, etc.)
  - `move.toUCINotation()`: Convert to UCI format (e.g., "e2e4")

- `fenrir::MoveType`: Enum for move types
  - `NORMAL`: Regular move
  - `CAPTURE`: Capturing move
  - `EN_PASSANT`: En passant capture *(generated)*
  - `CASTLE_KINGSIDE`: Kingside castling *(data model ready — not yet generated)*
  - `CASTLE_QUEENSIDE`: Queenside castling *(data model ready — not yet generated)*
  - `PROMOTION`: Pawn promotion *(data model ready — not yet generated)*

**⚠️ Important Notes:**
- Moves are **pseudo-legal** (follow piece rules but may leave king in check)
- No turn validation (can move opponent's pieces)
- No game-over detection
- See `AI_CONTEXT.md` for complete technical details

## Build System

**Fenrir uses CMake as the supported build system.**

### Building the Library

```bash
# Configure the build system (defaults to Debug mode)
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build the project (libraries, tests, main executable)
cmake --build build

# Build optimized release version
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# The shared library will be created at:
# bin/lib/libfenrir.so
```

### Run Tests and Coverage

```bash
# Compile and run all unit tests, then generate coverage report (requires 100% coverage)
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target coverage
```

## Using the Library

### Step-by-Step Integration Guide

1. **Build the Fenrir library**:

   ```bash
   git clone <repository-url>
   cd fenrir
   cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
   ```

2. **Create your application** (e.g., `my_chess_app.cpp`):

   ```cpp
   #include <iostream>
   #include <vector>
   #include "include/engine/engine.h"
   #include "include/chess/move.h"

   int main() {
	   try {
		   // Initialize the chess engine
		   fenrir::Engine engine;

		   // Display the initial board
		   std::cout << "Initial position:" << std::endl;
		   engine.printBoard();

		   // Demonstrate movement for different piece types using Move class
		   engine.makeMove(fenrir::Move("e2", "e4"));  // White pawn to e4
		   engine.makeMove(fenrir::Move("e7", "e5"));  // Black pawn to e5
		   engine.makeMove(fenrir::Move("g1", "f3"));  // White knight to f3
		   engine.makeMove(fenrir::Move("b8", "c6"));  // Black knight to c6
		   engine.makeMove(fenrir::Move("f1", "c4"));  // White bishop to c4

		   std::cout << "\nAfter multiple piece movements:" << std::endl;
		   engine.printBoard();

		   // Generate legal moves for different piece types (returns vector<Move>)
		   const std::vector<fenrir::Move> knight_moves = engine.generateMoves("f3");
		   std::cout << "\nLegal moves for white knight on f3:" << std::endl;
		   for (const auto& move : knight_moves) {
			   std::cout << move.getFrom() << " -> " << move.getTo() << std::endl;
		   }

		   const std::vector<fenrir::Move> bishop_moves = engine.generateMoves("c4");
		   std::cout << "\nLegal moves for white bishop on c4:" << std::endl;
		   for (const auto& move : bishop_moves) {
			   std::cout << move.getFrom() << " -> " << move.getTo() << std::endl;
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
│   │   ├── move.h       # Move class (replaces pair<string,string>)
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
make test      # Run all 300 tests
make coverage  # Generate coverage report (fails if < 100%)
```

**Test Structure:**
- Framework: Google Test
- Location: `tests/unit/*.test.cpp`
- Count: 300 tests across 11 test suites (9 skipped in CI environment)
- Coverage: 100% line coverage enforced

### Build System

**CMake is the supported build system.**

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build      # Build debug version (default)
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build    # Build optimized release
rm -rf build                                                        # Clean build artifacts
```

## 📚 Documentation

- **README.md** (this file): User-facing documentation and API reference
- **AI_CONTEXT.md**: Technical documentation for AI assistants (architecture, move support table, next steps)
- **RELEASE_NOTES.md**: Version history and changelog

## Roadmap

**Current Version: 0.2.0-dev** (Foundation + Memory Safety Complete)

**v0.3.0 - Full Legal Move Enforcement** *(in progress)*
1. ✅ ~~All piece movement logic~~ (DONE)
2. ✅ ~~Memory safety (unique_ptr, Rule of Five)~~ (DONE)
3. 🚧 AbstractBoard extensions (`getCastlingRights`, `getColor`)
4. 🚧 Check detection
5. 🚧 Legal move filtering (strip moves leaving king in check)
6. 🚧 Castling (kingside + queenside)
7. 🚧 Pawn promotion
8. 🚧 Checkmate / stalemate detection
9. 🚧 Make/unmake + `Engine::undoMove()` (zero-allocation search support)

**v0.4.0 - Reference Search Agent** *(C++, links libfenrir.so directly)*
- Minimax with alpha-beta pruning
- Basic position evaluation (material, mobility)
- Required before UCI is meaningful - UCI needs a `go` command response

**v0.5.0 - UCI Protocol** *(C++ wrapper around reference agent)*
- Standard UCI stdin/stdout interface
- Compatible with chess.com analysis, Lichess bots, Arena, Fritz, Chessbase
- Benchmark against Stockfish
- Agents written by others can wrap themselves in UCI to join the same ecosystem

**v0.6.0 - Language Bindings**
- Python bindings (ctypes or pybind11) - unlocks Python RL agents
- WebAssembly build - unlocks browser-based agents and UIs
- Python UCI wrapper for Python agents to play on Lichess/chess.com

## 🤝 Contributing

**Before contributing:**
1. Read `AI_CONTEXT.md` to understand the architecture
2. Write tests first (TDD approach)
3. Ensure 100% coverage (`make coverage`)
4. Follow existing naming conventions (camelCase methods, PascalCase classes)
5. Update documentation for API changes

## 📄 License

MIT License - see [LICENSE](LICENSE) file.

**Copyright (c) 2025 Juan Minor**

## Disclaimer

Fenrir currently generates **pseudo-legal moves** - move generation follows piece movement rules but does not yet filter moves that leave the king in check. Full legal move enforcement is the v0.3.0 target. Do not use the current version in production agents or applications that require complete rule correctness.
