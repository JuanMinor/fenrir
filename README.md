# Loki

Loki is an Atom Chess Engine designed to provide a robust and efficient chess-playing experience. It is built with modular components to ensure flexibility and maintainability. The project is written in C++ and adheres to modern software development practices.

## Features

- **Chess Board Representation**: Implements a flexible board representation using FEN (Forsyth-Edwards Notation).
- **Move Generation**: Supports move generation and validation.
- **PGN Support**: Handles Portable Game Notation (PGN) for recording and replaying games.
- **Logging**: Provides detailed logging for debugging and analysis.
- **Customizable Modifiers**: Includes modifiers for visual and functional customization.

## Project Structure

```
LICENSE
main.cpp
Makefile
README.md
bin/
    loki.elf
    build/
        chess/
            board.o
            fen.o
            moves.o
            piece.o
        chrono/
            chrono.o
        logger/
            logger.o
        modifier/
            modifier.o
        pgn/
            pgn.o
include/
    chess/
        board.h
        fen.h
        moves.h
        piece.h
    chrono/
        chrono.h
    core/
        core.h
    logger/
        logger.h
    modifier/
        modifier.h
    pgn/
        pgn.h
logs/
    loki.log
pgn/
    loki.pgn
    loki.store.txt
scripts/
    loki.sh
src/
    chess/
        board.cpp
        fen.cpp
        moves.cpp
        piece.cpp
    chrono/
        chrono.cpp
    logger/
        logger.cpp
    modifier/
        modifier.cpp
    pgn/
        pgn.cpp
```

## Build Instructions

1. Ensure you have `make` and a C++ compiler installed on your system.
2. Run the following command to build the project:
   ```bash
   make
   ```
3. The compiled binary will be available in the `bin/` directory as `loki.elf`.

## Usage

To run the chess engine, execute the following command:

```bash
./bin/loki.elf
```

## Logging

Logs are stored in the `logs/` directory. The primary log file is `loki.log`.

## PGN Support

PGN files are stored in the `pgn/` directory. The engine supports reading and writing PGN files for game analysis and replay.

## Scripts

Utility scripts are available in the `scripts/` directory. For example, `loki.sh` can be used for automated tasks.

## Development

### Prerequisites

- C++ Compiler
- GNU Make
- Recommended VS Code Extensions:
  - Code Spell Checker
  - C/C++ (ms-vscode.cpptools)
  - Makefile Tools
  - Prettier - Code Formatter
  - TODO Tree
  - GitLens

### Debugging

Use the `gdb` task for debugging. Ensure the project is built with debug symbols.

### Testing

Unit tests can be added to validate individual components. Ensure all tests pass before committing changes.

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your changes.

## Author

Juan Minor

## Acknowledgments

Special thanks to the open-source community for providing tools and libraries that make this project possible.

## Default FEN String

The Loki engine uses the following default FEN string to initialize the chessboard:

```
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
```

This represents the standard starting position of a chess game.
