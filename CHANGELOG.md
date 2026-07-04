# Changelog

All notable changes to the Fenrir Chess Engine project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.0] - 2026-07-04

### Added
- Precomputed attack tables for pawns, knights, kings, and sliding pieces (rays)
- 12-bitboard representation + occupancy tables replacing old mailbox system
- Fully legal move generation filtering out moves that put or leave the king in check
- Special moves support (castling, promotion, en passant)
- Checkmate and stalemate detection methods
- Singleton `Logger` and Pgn classes for cleaner execution and debug output tracking
- `Engine::is_draw()` method supporting stalemate, 50-move rule, and threefold repetition
- Static `Engine::version()` to query the compile-time library version
- Complete Google Test suite with 100% code coverage rule in Makefile

### Changed
- Refactored all method, parameter, and variable names from camelCase to snake_case to match `AI_CONTEXT.md` guidelines
- Decoupled move generation logic from board representation via `AbstractBoard`
- Made `UndoState` lightweight POD to achieve $O(1)$ memory copies on search trees

### Fixed
- Fixed redundant ternary expression and unused parameters in pawn promotion move generation
- Resolved UB risks with `const_cast` in Engine's move generation dispatchers by correctly declaring mutating helpers non-const
