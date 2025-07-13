# Fenrir Chess Engine

A modular C++ chess engine with FEN support, move generation, and comprehensive testing.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Build](https://img.shields.io/badge/Build-Make-green.svg)](https://www.gnu.org/software/make/)
[![Testing](https://img.shields.io/badge/Testing-Google%20Test-red.svg)](https://github.com/google/googletest)
[![VS Code](https://img.shields.io/badge/VS%20Code-Recommended-007ACC.svg)](https://code.visualstudio.com/)
[![Dev Container](https://img.shields.io/badge/Dev%20Container-Ready-brightgreen.svg)](https://containers.dev/)

> **Build System**: Fenrir uses **GNU Make** as the primary and currently supported build system. See the [Build System](#build-system) section for details.

> **Development Environment**: This project uses a **dev container** for consistent development across different machines. **VS Code is highly recommended** as it provides seamless integration with the dev container, debugging tools, and project-specific extensions. See the [Development Environment](#development-environment) section for setup instructions.

## Quick Start

```bash
# Clone and build the library (using GNU Make - the supported build system)
git clone <repository-url>
cd fenrir
make

# Run the example program
./scripts/run.sh

# Run tests with coverage
make test coverage

# For integration into your project, see "Using the Library" section below
```

## ✨ Complete Chess Implementation

**Fenrir now features complete movement logic for all chess pieces!**

All six chess piece types are fully implemented with comprehensive rule compliance:

| Piece | Implementation Status | Features |
|-------|---------------------|----------|
| ♟️ **Pawn** | ✅ **Complete** | Single/double moves, diagonal captures, en passant |
| ♜ **Rook** | ✅ **Complete** | Horizontal/vertical sliding, capture & blocking logic |
| ♞ **Knight** | ✅ **Complete** | L-shaped jumps, obstacle jumping |
| ♗ **Bishop** | ✅ **Complete** | Diagonal sliding, capture & blocking logic |
| ♛ **Queen** | ✅ **Complete** | Combined rook + bishop movement |
| ♚ **King** | ✅ **Complete** | Single-square movement in all 8 directions |

**Key Technical Achievements:**
- **Elegant Architecture**: Shared `__slide__` algorithm for rook, bishop, queen, and king
- **Smart King Implementation**: King leverages queen logic with single-square depth limitation
- **100% Test Coverage**: Comprehensive test suite with 216+ unit tests covering all edge cases
- **Performance Optimized**: Efficient move generation with proper boundary checking
- **Rule Compliant**: Full adherence to chess movement rules and capture mechanics

## Features

- **Complete Chess Logic**: Full board representation with **all piece movement implementations**
  - ♟️ **Pawns**: Single/double moves, captures, en passant
  - ♜ **Rooks**: Horizontal and vertical sliding movements
  - ♞ **Knights**: L-shaped jumping movements
  - ♗ **Bishops**: Diagonal sliding movements
  - ♛ **Queens**: Combined rook and bishop movements
  - ♚ **Kings**: Single-square movement in all directions
- **Advanced Move Generation**: Complete legal move validation for all piece types
- **FEN Support**: Parse and generate Forsyth-Edwards Notation strings
- **Shared Library**: `libfenrir.so` for integration into other projects
- **PGN Handling**: Game recording and replay functionality
- **Comprehensive Testing**: Unit tests with 100% code coverage requirement
- **Development Tools**: VS Code integration, debugging support, and automation scripts

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
auto pawn_moves = engine.generate_moves("e4");    // Pawn moves
auto knight_moves = engine.generate_moves("f3");  // Knight moves
auto all_moves = engine.generate_moves("d1");     // Queen moves

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

_Complete piece movement logic is implemented for all 6 chess piece types with full rule compliance._

_For complete examples and compilation instructions, see the "Using the Library" section below._

## Build System

**Fenrir uses GNU Make as the primary and currently supported build system.**

| Target          | Description                                       |
| --------------- | ------------------------------------------------- |
| `make`          | Build shared library (`bin/lib/libfenrir.so`)     |
| `make debug`    | Explicitly build in debug mode (default)          |
| `make release`  | Build optimized release version                   |
| `make test`     | Run unit tests with Google Test                   |
| `make coverage` | Generate coverage report (requires 100% coverage) |
| `make clean`    | Clean build artifacts                             |
| `make help`     | Show detailed help with all available targets     |

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

## Project Structure

```
├── include/          # Header files
│   ├── chess/        # Complete chess logic (board, all 6 piece types, moves, FEN)
│   ├── engine/       # Main engine interface
│   └── utils/        # Utilities and logging
├── src/              # Implementation files
├── tests/unit/       # Google Test unit tests (216+ tests, 100% coverage)
├── scripts/          # Automation scripts (run.sh, test.sh)
└── bin/              # Build artifacts
```

## Development

### Development Environment

**Fenrir uses a dev container for consistent development across different machines.**

#### Recommended Setup (VS Code + Dev Container)

1. **Install VS Code** with the following extensions:
   - Dev Containers extension (`ms-vscode-remote.remote-containers`)

2. **Open in Dev Container**:
   ```bash
   # Clone the repository
   git clone <repository-url>
   cd fenrir
   
   # Open in VS Code
   code .
   
   # VS Code will prompt to "Reopen in Container" - click Yes
   # Or use Command Palette: "Dev Containers: Reopen in Container"
   ```

3. **Why VS Code + Dev Container?**
   - **Consistent Environment**: Same development environment across all machines
   - **Pre-configured Tools**: All dependencies, compilers, and tools are pre-installed
   - **Integrated Debugging**: GDB integration with breakpoints and variable inspection
   - **Extension Integration**: Project-specific extensions are automatically installed
   - **No Local Setup**: No need to install GCC, Make, Google Test, or lcov locally

#### Alternative Development (Not Recommended)

If you cannot use the dev container, you'll need to manually install:

### Prerequisites

- GCC/G++ compiler (C++20 support required)
- **GNU Make** (primary build system)
- Google Test (for testing)
- lcov (for coverage reporting)

### Build System Support

- **Supported**: GNU Make (recommended and actively maintained)
- **Not Supported**: CMake, Autotools, or other build systems
- **Future**: Other build systems may be added based on community needs

### VS Code Setup

Install recommended extensions via the task runner:

```bash
# In VS Code Command Palette (Ctrl+Shift+P)
Tasks: Run Task → extensions
```

### Testing

All code must maintain 100% test coverage:

```bash
make coverage  # Fails if coverage < 100%
```

### Debugging

Use the predefined VS Code tasks for debugging with GDB.

## License

GNU General Public License v3.0 - see [LICENSE](LICENSE) file.

**Author**: Juan Minor
