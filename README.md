# Fenrir Chess Engine

A modular C++ chess engine with FEN support, move generation, and comprehensive testing.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Build](https://img.shields.io/badge/Build-Make-green.svg)](https://www.gnu.org/software/make/)
[![Testing](https://img.shields.io/badge/Testing-Google%20Test-red.svg)](https://github.com/google/googletest)

## Quick Start

```bash
# Build the shared library
make

# Run the example program
./scripts/run.sh

# Run tests with coverage
make test coverage
```

## Features

- **Chess Logic**: Board representation, move generation, and validation
- **FEN Support**: Parse and generate Forsyth-Edwards Notation strings
- **Shared Library**: `libfenrir.so` for integration into other projects
- **PGN Handling**: Game recording and replay functionality
- **Comprehensive Testing**: Unit tests with 100% code coverage requirement
- **Development Tools**: VS Code integration, debugging support, and automation scripts

## API Usage

```cpp
#include "include/engine/engine.h"

// Initialize with default starting position
fenrir::Engine engine;

// Make moves (rank, file coordinates)
engine.make_move(1, 1, 3, 1);  // Move pawn from b2 to b4

// Generate legal moves for a square
auto moves = engine.generate_moves("a2");

// Display board state
engine.print_board();

// Reset to starting position
engine.reset();
```

## Build System

| Target | Description |
|--------|-------------|
| `make` | Build shared library (`bin/lib/libfenrir.so`) |
| `make test` | Run unit tests with Google Test |
| `make coverage` | Generate coverage report (requires 100% coverage) |
| `make clean` | Clean build artifacts |

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

### Prerequisites
- GCC/G++ compiler
- GNU Make
- Google Test (for testing)
- lcov (for coverage reporting)

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
