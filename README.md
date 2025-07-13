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

## Features

- **Chess Logic**: Board representation, move generation, and validation
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

// Alternative: coordinate-based moves (rank, file - 0-indexed)
engine.make_move(1, 4, 3, 4);  // Same as e2-e4

// Generate legal moves for a square
auto moves = engine.generate_moves("e4");

// Display current board state
engine.print_board();

// Reset to starting position
engine.reset();
```

### Key Classes and Methods

- `fenrir::Engine`: Main interface for chess operations
- `engine.make_move(from, to)`: Execute a move
- `engine.generate_moves(square)`: Get legal moves for a piece
- `engine.get_fen()`: Get current position in FEN notation
- `engine.print_board()`: Display current position
- `engine.reset()`: Return to starting position

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

           // Make some moves using algebraic notation
           engine.make_move("e2", "e4");  // White pawn to e4
           engine.make_move("e7", "e5");  // Black pawn to e5
           engine.make_move("g1", "f3");  // White knight to f3

           std::cout << "\nAfter moves e2-e4, e7-e5, Ng1-f3:" << std::endl;
           engine.print_board();

           // Generate legal moves for a piece
           auto moves = engine.generate_moves("f3");
           std::cout << "\nLegal moves for knight on f3:" << std::endl;
           for (const auto& move : moves) {
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
│   ├── chess/        # Chess logic (board, pieces, moves, FEN)
│   ├── engine/       # Main engine interface
│   └── utils/        # Utilities and logging
├── src/              # Implementation files
├── tests/unit/       # Google Test unit tests
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

- GCC/G++ compiler (C++17 support required)
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
