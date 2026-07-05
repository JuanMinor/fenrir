# Fenrir — Roadmap & TODO

> **Current state**: v0.3.0 — legal move generation, bitboard representation, precomputed attack tables, C++ rules library complete.
> **Goal**: Production-grade chess rules library + C++ engine with UCI, versioned `.so`, and multi-language bindings.

Tasks within each milestone are ordered. Do not start a milestone until the previous one is complete — each builds directly on the last.

---

## Milestone 0 — Library Versioning  *(do this first, it costs nothing and enables everything)*

The library currently has no version. Consumers (bindings, engines) cannot pin to a stable ABI.

- [x] **0.1** Add a `VERSION` file at the repo root containing `0.3.0` (semver, single source of truth).
- [x] **0.2** Read `VERSION` in the `Makefile` (`VERSION := $(shell cat VERSION)`) and pass it as a compiler define (`-DFENRIR_VERSION=\"$(VERSION)\"`).
- [x] **0.3** Set the shared library soname: build as `libfenrir.so.0.3.0`, create symlinks `libfenrir.so.0` → `libfenrir.so.0.3.0` and `libfenrir.so` → `libfenrir.so.0`. Use `-Wl,-soname,libfenrir.so.0` in the linker flags.
- [x] **0.4** Expose `fenrir::version()` → `const char*` in `engine.h` (returns the compile-time `FENRIR_VERSION` string). Add a unit test for it.
- [x] **0.5** Add a `make install` target: installs `libfenrir.so*` to `$(PREFIX)/lib` (default `/usr/local/lib`) and all public headers to `$(PREFIX)/include/fenrir`. Generates a `fenrir.pc` pkg-config file.
- [x] **0.6** Add a `CHANGELOG.md` — one entry per release, format: `## [version] - date`, `### Added / Changed / Fixed`.

---

## Milestone 1 — Bitboard Foundation  *(replaces mailbox, unblocks everything else)*

**Why first**: the mailbox `vector<vector<unique_ptr<Piece>>>` is 10–50x slower than bitboards for move generation. Every feature built on top of it carries that penalty. Do this before legal moves, before the engine, before bindings.

### Phase 1A — New data types and attack tables

- [x] **1.1** Add `include/chess/attacks.h` and `src/chess/attacks.cpp`. Define:
  - `extern const uint64_t KNIGHT_ATTACKS[64]` — precomputed for all 64 squares
  - `extern const uint64_t KING_ATTACKS[64]`
  - `extern const uint64_t PAWN_ATTACKS[2][64]` — indexed by `[color][square]`
  - `extern const uint64_t RAY[64][8]` — 8 directional rays from each square (N/NE/E/SE/S/SW/W/NW)
  - `void initAttackTables()` — called once at startup in `Engine` constructor
- [x] **1.2** Define `PieceIndex` enum in `core.h`: `W_PAWN=0, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING` (12 values). Bit n of `bb[PieceIndex]` = piece of that type/color on square n (a1=0, h8=63).
- [x] **1.3** Add `Square` typedef (`uint8_t`) and helpers in `utils.h`: `squareFromRankFile(rank, file)`, `rankFromSquare(sq)`, `fileFromSquare(sq)`, `squareFromAlgebraic(str)`, `algebraicFromSquare(sq)`.

### Phase 1B — Rewrite `Board`

- [x] **1.4** Replace `vector<vector<unique_ptr<Piece>>>` in `Board` with `uint64_t bb[12]` plus `uint64_t occupancy[3]` (white, black, all). Remove the `buildBoard()` / heap allocation path entirely.
- [x] **1.5** Rewrite `Board::getPiece(rank, file)` — probe bitboards, reconstruct a `Piece` value on demand (return by value or via internal cache). Update `AbstractBoard` interface accordingly.
- [x] **1.6** Add `Board::getColor() const` → `uint8_t`. Add `Board::getCastlingRights() const` → `uint8_t` (bitmask: `KQ_BLACK=1, KQ_WHITE=2, KS_BLACK=4, KS_WHITE=8`). Expose both on `AbstractBoard`.
- [x] **1.7** Add `Board::getBB(PieceIndex) const` → `uint64_t` and `Board::getOccupancy(uint8_t color) const` → `uint64_t`. Expose on `AbstractBoard`.
- [x] **1.8** Add `Board::getEnPassantBB() const` → `uint64_t` (single bit set on the en passant target square, or 0). Expose on `AbstractBoard`. Keep existing `getEnPassant()` string method for FEN output.
- [x] **1.9** Rewrite `Board::move()` to flip bits in the affected bitboards instead of moving heap objects.
- [x] **1.10** Rewrite `Board::getFen()` to reconstruct the FEN placement string by iterating bitboards.
- [x] **1.11** Rewrite `Board::print()` to iterate bitboards.
- [x] **1.12** All 300 existing tests must pass. Run `make test` — fix anything broken. Do not delete tests; only fix the implementation.

### Phase 1C — Rewrite `Moves`

- [x] **1.13** Rewrite `Moves::generatePawnMoves()` using bitwise shifts and `PAWN_ATTACKS[color][sq]` from the attack tables. Single push: `(bb[W_PAWN] << 8) & ~occupancy[ALL]`. Double push: only from rank 2. Attacks: `PAWN_ATTACKS[W][sq] & occupancy[BLACK]`.
- [x] **1.14** Rewrite `Moves::generateKnightMoves()` using `KNIGHT_ATTACKS[sq] & ~occupancy[same_color]`.
- [x] **1.15** Rewrite `Moves::generateKingMoves()` using `KING_ATTACKS[sq] & ~occupancy[same_color]`.
- [x] **1.16** Rewrite sliding piece generation (rook, bishop, queen) using classical ray blocker: for each ray direction, mask against `occupancy[ALL]`, find first blocker with `__builtin_ctzll`, include that square if enemy.
- [x] **1.17** Rewrite `Moves::generateMoves()` dispatcher to use `PieceIndex`-based dispatch instead of `switch(tolower(alias))`.
- [x] **1.18** Add `Moves::generateAllMoves(const AbstractBoard& board, uint8_t color, std::vector<Move>& moves) const` — iterates all pieces of the given color and collects all pseudo-legal moves in one call.
- [x] **1.19** All 300 tests pass. Run `make test`. Measure: `make release` binary should show at minimum 2M `generateAllMoves()` calls/sec on a single thread (add a benchmark target `make bench`).

---

## Milestone 2 — Legal Move Generation  *(completes the rules layer)*

Depends on: Milestone 1 complete.

### Phase 2A — Check detection

- [x] **2.1** Add `Board::isSquareAttackedBy(uint8_t square, uint8_t color) const` — returns true if any piece of `color` attacks `square`. Implementation: probe each attack table against the corresponding bitboard. O(1) after bitboard rewrite.
- [x] **2.2** Add `Board::isInCheck(uint8_t color) const` — finds king square via `__builtin_ctzll(bb[K_INDEX])`, calls `isSquareAttackedBy(kingSquare, opponent)`.
- [x] **2.3** Write unit tests for `isInCheck`: known check positions from FEN (at least 10 positions covering all attacker types).

### Phase 2B — Make / Unmake

- [x] **2.4** Define `UndoState` struct in `include/chess/board.h`:
  ```cpp
  struct UndoState {
      uint64_t bb[12];        // full board snapshot (12 × 8 bytes = 96 bytes)
      uint64_t occupancy[3];
      uint8_t  castling;
      uint8_t  color;
      uint8_t  halfMoveClock;
      uint8_t  fullMoves;
      uint8_t  capturedPieceIndex;  // 255 = no capture
      uint8_t  enPassantSquare;     // 255 = none
  };
  ```
- [x] **2.5** Implement `Board::applyMove(const Move& move)` → `UndoState`. Handles all `MoveType` values: NORMAL, CAPTURE, EN_PASSANT, CASTLE_KINGSIDE, CASTLE_QUEENSIDE, PROMOTION. Updates `color`, `halfMoveClock`, `fullMoves`, `castling` rights, `enPassant` square. Returns `UndoState` before-snapshot.
- [x] **2.6** Implement `Board::undoMove(const UndoState& state)` — restores all 12 bitboards + metadata from the `UndoState`. Must be a perfect inverse of `applyMove`.
- [x] **2.7** Write unit tests: apply a move, undo it, assert FEN is unchanged. Cover all `MoveType` values.

### Phase 2C — Legal filtering

- [x] **2.8** Add `Moves::generateLegalMoves(Board& board, uint8_t color, std::vector<Move>& moves) const`: calls `generateAllMoves()`, then for each pseudo-legal move: `applyMove → isInCheck → undoMove`, discards moves leaving king in check.
- [x] **2.9** Update `Engine::generateMoves(algebraicAddress)` to call `generateLegalMoves` internally.
- [x] **2.10** Add `Engine::generateAllMoves()` → `std::vector<Move>` — returns all legal moves for the side to move. This is the primary search interface.

### Phase 2D — Special moves

- [x] **2.11** Add castling move generation in `Moves::generateKingMoves()`: check castling rights bitmask, verify king and rook haven't moved, verify squares between are empty (`~occupancy[ALL]`), verify king does not pass through or land on an attacked square.
- [x] **2.12** Add pawn promotion detection: any pawn reaching rank 8 (white) or rank 1 (black) generates 4 `PROMOTION` moves (Q/R/B/N). Update `generatePawnMoves()`.
- [x] **2.13** Add `Engine::isCheckmate() const` → `bool`: `generateAllMoves().empty() && isInCheck(color)`.
- [x] **2.14** Add `Engine::isStalemate() const` → `bool`: `generateAllMoves().empty() && !isInCheck(color)`.
- [x] **2.15** Add `Engine::is_draw()` → `bool`: covers stalemate, fifty-move rule (half_move_clock >= 100), threefold repetition (track position history in `Engine`).
- [x] **2.16** Enforce turn order in `Engine::makeMove()` — reject moves of the wrong color, return error/throw.
- [x] **2.17** Update all unit tests to expect legal moves only. The existing 300 tests were written against pseudo-legal output — audit each test suite and fix expected move counts/sets.
- [x] **2.18** `make test` must pass. `make coverage` must still report 100%.

---

## Milestone 3 — C++ Search Engine  *(the engine binary)*

Depends on: Milestone 2 complete.

### Phase 3A — New engine binary target

- [ ] **3.1** Create `engine/` directory at repo root (sibling of `src/`). This is the search engine, a separate binary that links `libfenrir.so`.
- [ ] **3.2** Add `engine` executable target in `CMakeLists.txt`: compiles `engine/*.cpp`, links `fenrir`, produces `bin/fenrir-engine`.
- [ ] **3.3** Add `engine/eval.cpp` + `include/engine/eval.h`: basic material evaluation.
  - Piece values: pawn=100, knight=320, bishop=330, rook=500, queen=900, king=20000 (centipawns).
  - `int evaluate(const AbstractBoard& board, uint8_t color)` — iterate all 64 bitboard squares, sum material differential from the perspective of `color`.

### Phase 3B — Search

- [ ] **3.4** Add `engine/search.cpp` + `include/engine/search.h`. Implement `Search` class with:
  - `int alphaBeta(Engine& engine, int depth, int alpha, int beta, uint8_t color)`
  - Calls `engine.generateAllMoves()`, `engine.applyMove()`, recurse, `engine.undoMove()`.
  - Base case: `depth == 0` → return `evaluate(engine.getBoardView(), color)`.
- [ ] **3.5** Add iterative deepening: `Search::findBestMove(Engine& engine, int maxDepth)` — runs `alphaBeta` at depth 1, 2, … maxDepth, returns best `Move` found at the deepest completed iteration.
- [ ] **3.6** Add move ordering in `Search`: score captures first (MVV-LVA: Most Valuable Victim – Least Valuable Attacker), then quiet moves. Sort before the alpha-beta loop.
- [ ] **3.7** Add quiescence search: at `depth == 0`, instead of returning static eval, search all captures until a quiet position (prevents horizon effect).
- [ ] **3.8** Add `Engine::applyMove(const Move&)` → `UndoState` and `Engine::undoMove(const UndoState&)` as public methods wrapping `Board::applyMove/undoMove`. These are what the search engine calls.

### Phase 3C — UCI protocol

- [ ] **3.9** Add `engine/uci.cpp` + `include/engine/uci.h`. Implement the UCI I/O loop:
  - `uci` → respond with `id name Fenrir`, `id author <name>`, `uciok`
  - `isready` → `readyok`
  - `ucinewgame` → `engine.reset()`
  - `position startpos moves e2e4 ...` → parse and replay moves via `engine.makeMove()`
  - `position fen <FEN>` → `engine.reset(FEN)`
  - `go depth <n>` → `search.findBestMove(engine, n)` → `bestmove e2e4`
  - `quit` → exit
- [ ] **3.10** Add `engine/main.cpp` — entry point: instantiates `Engine`, `Search`, `UCI`, runs the I/O loop.
- [ ] **3.11** Test the engine binary manually with Arena or any UCI-capable GUI. Play at least 10 games vs itself.
- [ ] **3.12** Add a `bench` custom target in `CMakeLists.txt`: runs a fixed perft (position + depth) and prints nodes/sec. Perft results must match known values (use `kiwipete` FEN as reference).

---

## Milestone 4 — Language Bindings

Depends on: Milestone 2 complete (Milestone 3 is parallel, not required for bindings).

Each binding wraps only the public `Engine` API: `generateAllMoves`, `makeMove`, `undoMove`, `getFen`, `reset`, `isCheckmate`, `isStalemate`, `isDraw`, `printBoard`.

### Python

- [ ] **4.1** Add `bindings/python/` directory.
- [ ] **4.2** Write a `ctypes`-based binding in `fenrir.py`: load `libfenrir.so` via `ctypes.CDLL`, wrap each public `Engine` method. Expose a `FenrirEngine` Python class.
- [ ] **4.3** Add a `pybind11` alternative in `bindings/python/pybind/`: provides a proper Python module (`import fenrir`), better type safety, no manual marshaling.
- [ ] **4.4** Add `python` target in `CMakeLists.txt` — builds the pybind11 extension `.so`.
- [ ] **4.5** Write `tests/python/test_engine.py`: at least 20 tests covering move generation, FEN round-trip, makeMove, checkmate detection.

### Java

- [ ] **4.6** Add `bindings/java/` directory.
- [ ] **4.7** Add a C JNI shim layer `bindings/java/jni/fenrir_jni.cpp` — thin wrappers that translate between JNI types and Fenrir C++ types. Expose as `libfenrir_jni.so`.
- [ ] **4.8** Add `bindings/java/src/io/fenrir/FenrirEngine.java` — Java class that loads `libfenrir_jni.so` via `System.loadLibrary` and wraps the native methods.
- [ ] **4.9** Add `java` target in `CMakeLists.txt`.
- [ ] **4.10** Write JUnit tests `bindings/java/tests/FenrirEngineTest.java`.

### Rust

- [ ] **4.11** Add `bindings/rust/` directory with a Cargo workspace.
- [ ] **4.12** Write `bindings/rust/src/lib.rs` using `unsafe` FFI blocks. Define a `FenrirEngine` Rust struct wrapping the C++ object pointer.
- [ ] **4.13** Add a `build.rs` that links against `libfenrir.so` via `cargo:rustc-link-lib=fenrir`.
- [ ] **4.14** Write Rust unit tests (`cargo test`).

### Go

- [ ] **4.15** Add `bindings/go/` directory.
- [ ] **4.16** Write `bindings/go/fenrir.go` using `cgo`: `// #include "include/engine/engine.h"` and CGo function wrappers.
- [ ] **4.17** Add `go test` test suite.

### C header (required by all bindings above)

- [ ] **4.18** Add `include/fenrir_c.h` — a pure C API (`extern "C"`) wrapping `Engine`. All methods take an opaque `FenrirHandle` (void pointer). This is the stable ABI that all language bindings actually call:
  ```c
  FenrirHandle fenrir_create(const char* fen);
  void         fenrir_destroy(FenrirHandle h);
  int          fenrir_generate_all_moves(FenrirHandle h, FenrirMove* out, int maxMoves);
  int          fenrir_make_move(FenrirHandle h, const char* uci_move);
  const char*  fenrir_get_fen(FenrirHandle h);
  void         fenrir_reset(FenrirHandle h, const char* fen);
  int          fenrir_is_checkmate(FenrirHandle h);
  int          fenrir_is_stalemate(FenrirHandle h);
  int          fenrir_is_draw(FenrirHandle h);
  ```
- [ ] **4.19** Add `src/fenrir_c.cpp` implementing the above. Add unit tests.

---

## Milestone 5 — WebAssembly

Depends on: Milestone 4 (C header) complete.

- [ ] **5.1** Install `emscripten` toolchain. Add `.devcontainer` package or document in `README.md`.
- [ ] **5.2** Add `wasm` target in `CMakeLists.txt`: compiles with `emcc`, exports `fenrir_c.h` functions, outputs `bin/wasm/fenrir.wasm` + `fenrir.js`.
- [ ] **5.3** Add `bindings/js/fenrir.js` — ES module wrapper around the WASM module. Exposes `async createEngine(fen)` and all `Engine` methods as `async` JS functions.
- [ ] **5.4** Write jest tests `bindings/js/tests/fenrir.test.js`.

---

## Cross-Cutting (do alongside milestones as they become relevant)

- [x] **X.0** Migrate build system from GNU Make to CMake, preserving all test coverage reporting and exact tables.
- [ ] **X.1** CI/CD: add `.github/workflows/ci.yml` — on every push: run tests, run coverage target (must be 100%), build release version, upload `libfenrir.so` as a build artifact.
- [ ] **X.2** CI: add a perft regression job — runs bench target and fails if performance drops > 10% from baseline.
- [ ] **X.3** Add `CONTRIBUTING.md` — build instructions, test requirements, coverage rule, coding conventions.
- [ ] **X.4** Replace `vector<Move>` return values with output-parameter versions throughout `Moves` (already done in places) — eliminates one heap allocation per `generateAllMoves()` call. Audit the entire API for unnecessary heap allocations.
- [ ] **X.5** Add Zobrist hashing in `Board`: compute a `uint64_t hash` incrementally in `applyMove`/`undoMove`. Used by repetition detection (Milestone 2) and transposition tables (future).
- [ ] **X.6** Transposition table (future v0.4+): `std::unordered_map<uint64_t, TTEntry>` keyed on Zobrist hash. Stores depth/score/best-move per position. Cuts search tree significantly.

---

## Version Plan Summary

| Version | Milestone | Key deliverable |
|---------|-----------|-----------------|
| **0.2.0** | — | Current: pseudo-legal moves, 300 tests ✅ |
| **0.3.0** | 0 + 1 + 2 | Versioned `.so`, bitboards, 100% legal moves |
| **0.4.0** | 3 | C++ engine binary + UCI (plays chess via GUI) |
| **0.5.0** | 4 | C header + Python + Java + Rust + Go bindings |
| **0.6.0** | 5 | WebAssembly / browser support |
