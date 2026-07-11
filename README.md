# Fenrir - Chess Rules Engine

Fenrir is a high-performance C++ **Neural Network Chess Engine**. It also exports its core rules and move generation logic as a shared library (`libfenrir.so`). This dual-architecture allows Fenrir to operate both as a standalone AI engine (via UCI) and as the blazing-fast foundation for custom chess ecosystems.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Build](https://img.shields.io/badge/Build-CMake-blue.svg)](https://cmake.org/)
[![Testing](https://img.shields.io/badge/Testing-Google%20Test-red.svg)](https://github.com/google/googletest)
[![Coverage](https://img.shields.io/badge/Coverage-100%25-brightgreen.svg)]()
[![Tests](https://img.shields.io/badge/Tests-359%20passing-success.svg)]()

> **Architecture**: Fenrir is the base of a three-layer system: **Fenrir** (rules, validation, move generation) -> **Agents** (Python, JS, or any language - they search and decide) -> **UIs** (mobile, web, desktop - they present and explain). Fenrir's job is to be correct, fast, and stable so everything built on top of it can be trusted.

> **Current Status**: Version 0.3.0 (Feature Complete). Full legal move enforcement (check detection, castling, promotion, pinned pieces, checkmate/stalemate) is 100% complete and mathematically validated via deep Perft tests.

> **🔧 Build System**: CMake is the supported build system. See [Build System](#build-system) for details.

> **🐳 Development**: Uses dev containers for consistent environments. VS Code recommended. See [Development Environment](#development-environment).

## ⚡ Quick Start

```bash
# Clone and build the library
git clone <repository-url>
cd fenrir
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j

# Run the example program
./scripts/run.sh

# Run all 300+ tests
./scripts/test.sh

# Generate coverage report (requires 100%)
./scripts/build/generate_coverage.sh

# For integration, see "Using the Library" section below
```

## ✨ What Fenrir Does (And Doesn't Do)

### ✅ Implemented Features

**Complete Piece Movement Logic** - All 6 chess piece types can generate pseudo-legal moves:

| Piece | Movement Implementation | Special Features |
|-------|------------------------|------------------|
| ♟️ **Pawn** | ✅ Complete | Single/double push, diagonal captures, **en passant**, promotion |
| ♜ **Rook** | ✅ Complete | Horizontal/vertical sliding, blocking detection |
| ♞ **Knight** | ✅ Complete | L-shaped jumps, can jump over pieces |
| ♗ **Bishop** | ✅ Complete | Diagonal sliding, blocking detection |
| ♛ **Queen** | ✅ Complete | Combined rook + bishop (8 directions) |
| ♚ **King** | ✅ Complete | Single-square movement in all 8 directions, castling (kingside/queenside) |

**Other Working Features:**
- 📋 **FEN Support**: Parse and generate full Forsyth-Edwards Notation
- 🛡️ **Absolute Legal Move Enforcement**: Complete check detection, pin resolution, safe castling, and pawn promotion. `generate_moves` guarantees mathematical correctness.
- 🎯 **Game State Detection**: Native `is_checkmate()` and `is_stalemate()` support.
- 📦 **Shared Library**: `libfenrir.so` (`libfenrir.dll`) for cross-platform integration
- 🪵 **Logging System**: Production-ready with rotation and levels
- 📝 **PGN Recording**: Basic game notation support
- ✅ **100% Test Coverage**: 300+ unit tests, all passing, plus multi-million node Perft suite validation
- ✅ **Memory Safety**: `unique_ptr<Piece>` ownership, Rule of Five enforced on `Board`

### ❌ Out of Scope / Not Implemented

- 🚫 **No Built-in AI**: Fenrir is purely a rules and validation engine. Building search agents, bots, or position evaluators is the responsibility of client applications linking against `libfenrir.so`.
- 🚫 **No UCI Protocol (Yet)**: Cannot interface directly with chess GUIs. UCI standard I/O is planned for v0.4.0.

## 🚀 Performance Benchmarks

Fenrir is highly optimized for performance and strict mathematical correctness. The engine undergoes rigorous `perft` (Performance Test) validations against standard FENs to ensure absolute rule enforcement at deep traversal depths.

*Note: The following metrics were captured during development inside a Linux Docker container on an Early 2015 13-inch laptop (Dual-Core i7-5557U, 16GB RAM) running single-threaded. Engine compiled with `-O3 -flto -march=native`.*

### Standard Start Position
`rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1`
* **Depth 5**: 4,865,609 nodes (✅ Perfect Match) | ~5.3 Million NPS
* **Depth 6**: 119,060,324 nodes (✅ Perfect Match) | ~4.7 Million NPS

### Kiwipete (Aggressive Stress Test)
`r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1`
* **Depth 4**: 4,085,603 nodes (✅ Perfect Match) | ~6.5 Million NPS
* **Depth 5**: 193,690,690 nodes (✅ Perfect Match) | ~6.3 Million NPS

On modern desktop processors (e.g., AMD Zen 3 or newer) natively compiled with MSVC/MinGW, single-threaded throughput scales significantly higher.

## The Ecosystem Fenrir Enables

Fenrir is not a standalone chess program. It is the foundation of a larger system:

```
Layer 3 - UIs
  Mobile apps (React Native, Swift)
  Web apps (JS/WASM)
  Desktop apps (Electron)
  Purpose: present, visualize, explain moves to humans

Layer 2 - Fenrir Executable (The Engine)
  Native C++ application (MCTS + Neural Network)
  Purpose: Search, evaluate, and output the best move via UCI

Layer 1 - libfenrir.so (The Library)
  C++ shared library embedded in the engine
  Purpose: Rules, move generation, validation, FEN, PGN
  Also exported for Python RL training pipelines
```

**Why this matters for Fenrir's design:**
- **Correctness is non-negotiable.** Agents learn from the moves Fenrir says are legal. A wrong answer corrupts training data and every layer above it.
- **Performance is structural.** An agent doing reinforcement learning plays millions of games during training. A minimax search at depth 5 makes roughly 24 million `generate_moves()` calls. Fenrir must handle that workload efficiently.
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
engine.make_move(fenrir::Move("e2", "e4"));  // Pawn e2 to e4
engine.make_move(fenrir::Move("e7", "e5"));  // Pawn e7 to e5
engine.make_move(fenrir::Move("g1", "f3"));  // Knight g1 to f3
engine.make_move(fenrir::Move("b8", "c6"));  // Knight b8 to c6

// Specify move types for special moves
engine.make_move(fenrir::Move("e4", "d5", fenrir::MoveType::CAPTURE));  // Capture
engine.make_move(fenrir::Move("e7", "e8", fenrir::MoveType::PROMOTION, 'Q'));  // Promotion

// Generate legal moves for any piece type (returns vector<Move>)
const std::vector<fenrir::Move> pawn_moves = engine.generate_moves("e4");  // Pawn moves
const std::vector<fenrir::Move> knight_moves = engine.generate_moves("f3");  // Knight moves
const std::vector<fenrir::Move> all_moves = engine.generate_moves("d1");  // Queen moves

// Access move information
for (const auto& move : pawn_moves) {
    std::cout << move.get_from() << " -> " << move.get_to() << std::endl;
}

// Display current board state
engine.print_board();

// Reset to starting position
engine.reset();
```

### Key Classes and Methods

- `fenrir::Engine`: Main interface for chess operations
  - `engine.make_move(const Move&)`: Execute a move using Move object
  - `engine.generate_moves(square)`: Get legal moves (returns `vector<Move>`)
  - `engine.get_fen()`: Get current position in FEN notation
  - `engine.print_board()`: Display current position
  - `engine.reset()`: Return to starting position

- `fenrir::Move`: Represents a chess move
  - Constructor: `Move(from, to, moveType=NORMAL, promotionPiece='\0')`
  - `move.get_from()`: Get source square (e.g., "e2")
  - `move.get_to()`: Get destination square (e.g., "e4")
  - `move.get_move_type()`: Get move type (NORMAL, CAPTURE, EN_PASSANT, etc.)
  - `move.to_uci_notation()`: Convert to UCI format (e.g., "e2e4")

- `fenrir::MoveType`: Enum for move types
  - `NORMAL`: Regular move
  - `CAPTURE`: Capturing move
  - `EN_PASSANT`: En passant capture
  - `CASTLE_KINGSIDE`: Kingside castling
  - `CASTLE_QUEENSIDE`: Queenside castling
  - `PROMOTION`: Pawn promotion

**⚠️ Important Notes:**
- Moves are **strictly legal** (enforces check, pins, turn order, and castling rules)
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
           engine.print_board();

           // Demonstrate movement for different piece types using Move class
           engine.make_move(fenrir::Move("e2", "e4"));  // White pawn to e4
           engine.make_move(fenrir::Move("e7", "e5"));  // Black pawn to e5
           engine.make_move(fenrir::Move("g1", "f3"));  // White knight to f3
           engine.make_move(fenrir::Move("b8", "c6"));  // Black knight to c6
           engine.make_move(fenrir::Move("f1", "c4"));  // White bishop to c4

           std::cout << "\nAfter multiple piece movements:" << std::endl;
           engine.print_board();

           // Generate legal moves for different piece types (returns vector<Move>)
           const std::vector<fenrir::Move> knight_moves = engine.generate_moves("f3");
           std::cout << "\nLegal moves for white knight on f3:" << std::endl;
           for (const auto& move : knight_moves) {
               std::cout << move.get_from() << " -> " << move.get_to() << std::endl;
           }

           const std::vector<fenrir::Move> bishop_moves = engine.generate_moves("c4");
           std::cout << "\nLegal moves for white bishop on c4:" << std::endl;
           for (const auto& move : bishop_moves) {
               std::cout << move.get_from() << " -> " << move.get_to() << std::endl;
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
├── CMakeLists.txt       # Build system
├── README.md            # This file
└── AI_CONTEXT.md        # Technical documentation for AI assistants
```

## 🛠️ Development

### Prerequisites

**Required:**
- GCC/G++ compiler with C++20 support
- CMake 3.15+
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
./scripts/test.sh      # Run all 300+ tests
./scripts/build/generate_coverage.sh  # Generate coverage report (fails if < 100%)
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

**Current Version: 0.3.0** (Rules & Game Logic Complete)

**v0.3.0 - Full Legal Move Enforcement** *(COMPLETE)*
- ✅ AbstractBoard extensions (`get_castling_rights`, `get_color`)
- ✅ Check detection and pinned piece resolution
- ✅ Strict legal move filtering (100% Perft verified)
- ✅ Castling (kingside + queenside) with attack validation
- ✅ Pawn promotion
- ✅ Checkmate / stalemate detection
- ✅ Fast Make/unmake (`apply_move`/`undo_move`)

**v0.4.0 - Monte Carlo Tree Search (MCTS) Engine**
- Native C++ MCTS implementation.
- `src/search/` directory established.
- Tree policy, playout policy, and backpropagation mechanics.

**v0.5.0 - Neural Network Inference (NNUE/ONNX)**
- Loading pre-trained neural network weights (`.weights` or `.onnx` files) in C++.
- Replacing random playouts with neural network evaluation.

**v0.6.0 - UCI Protocol & Executable**
- Standard UCI stdin/stdout interface integration.
- Standalone `fenrir` executable built alongside `libfenrir.so`.
- Responds to `go` commands using the MCTS+NN engine.

**v0.7.0 - C++ Self-Play & Local Training Pipeline**
- Build a `--self-play` mode into the `fenrir` executable to generate training games at max speed.
- Output raw game data to disk for asynchronous consumption by an external PyTorch optimization loop.
- Support **Warm-Starting**: initialize training with a basic pre-trained weights file (e.g., trained on human grandmaster games) to bootstrap competency and drastically reduce local compute time.

## 🤝 Contributing

**Before contributing:**
1. Read `AI_CONTEXT.md` to understand the architecture
2. Write tests first (TDD approach)
3. Ensure 100% coverage (`./scripts/build/generate_coverage.sh`)
4. Follow existing naming conventions (snake_case methods, PascalCase classes)
5. Update documentation for API changes

## 📄 License

MIT License - see [LICENSE](LICENSE) file.

**Copyright (c) 2025 Juan Minor**

## Disclaimer

Fenrir is a legal-move engine built for speed and correctness, but it is provided "as is" under the MIT license without any warranties.
