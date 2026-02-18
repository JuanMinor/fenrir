# Legal Move Generation Plan

**Goal**: Upgrade `Engine::generateMoves()` from pseudo-legal to fully legal move generation, and implement make/unmake so that agents doing search can call Fenrir efficiently at scale.
**Target version**: v0.3.0
**Total estimated effort**: 48-64 hours (includes bitboard foundation rewrite)

## Context

Fenrir is the rules layer of a three-tier ecosystem. Agents in Python, JS, or any language consume Fenrir via FFI and implement their own search (minimax, alpha-beta, reinforcement learning). A depth-5 minimax search makes roughly 24 million `generateMoves()` calls. Every design decision in this plan must account for that workload. Correctness and performance are both non-negotiable.

## Architectural Decision: Bitboards

**Decision**: Replace the mailbox board representation (`vector<vector<unique_ptr<Piece>>>`) with a bitboard representation before implementing legal moves.

**Why now, not later:**
Building legal moves on the mailbox representation and then rewriting for bitboards would mean implementing Phases 1-7 twice. Bitboards change the meaning of `Board`, `Piece`, `Moves`, and make/unmake fundamentally — there is no incremental migration path that saves work. The rewrite must come first.

**What bitboards are:**
12 `uint64_t` values, one per piece-type/color combination (white pawns, white knights, ..., black king). Each bit represents a square (bit 0 = a1, bit 63 = h8). A `uint64_t` holding all occupied squares is `occupancy = white_pieces | black_pieces`. Attack generation, move filtering, and make/unmake all become bitwise operations: nanoseconds, not pointer traversals.

**Impact on existing types:**

| Type | Current (mailbox) | After bitboard rewrite |
|------|-------------------|----------------------|
| `Board` | `vector<vector<unique_ptr<Piece>>>` | 12 `uint64_t` bitboards + side-state |
| `Piece` | Heap-allocated object with vtable | Mostly unnecessary — identity encoded in which bitboard |
| `Moves` | Virtual dispatch, pointer walks | Bit manipulation, precomputed attack tables |
| Make/unmake | Save/restore `unique_ptr` ownership | Save 12 integers + ~10 bytes of side-state |
| Check detection | Iterate squares, generate opponent moves | `attackers & king_bb` — a few bitwise ops |

**What is preserved:**
- `AbstractBoard` interface — but `getPiece(rank, file)` returns a reconstructed value, not a pointer to a heap object.
- `Engine` public API — `generateMoves()`, `makeMove()`, `getFen()`, `reset()` signatures unchanged from the consumer's perspective.
- `Move` struct — from/to squares, `MoveType`, `promotionPiece`.
- FEN parsing/generation — same contract, different internal representation.
- All 300 existing tests must still pass after Phase 0 completes.

**Precomputed tables required:**
- Knight attack table: `knight_attacks[64]` — one `uint64_t` per square.
- King attack table: `king_attacks[64]`.
- Pawn push/attack tables: `pawn_pushes[2][64]`, `pawn_attacks[2][64]` (indexed by color).
- Ray attack tables for sliding pieces (rooks, bishops, queens): `ray_attacks[8][64]` — one per direction per square.
- These are computed once at startup (static initialization), not per move.

**Slider move generation:**
Sliding pieces (rook, bishop, queen) need occupancy-aware attack generation. The standard approach for a platform-class engine is **magic bitboards** — a hash lookup that maps (square, occupancy mask) to attack bitboard in one array access. Alternative (simpler to implement first): **classical ray attacks** with an "o^(o-2r) trick" or simple loop along rays. Recommended order: implement classical rays first to get correctness, then optionally replace with magic bitboards in v0.4.0 if profiling shows it as a bottleneck.

**`Piece` class fate:**
`Piece` as a heap object is eliminated from the hot path. Options:
1. Keep `Piece` as a lightweight value type (no heap) for the `AbstractBoard::getPiece()` return — board reconstructs it on demand from the bitboards.
2. Remove `Piece` entirely and have consumers work with square indices + piece-type enums.
Option 1 preserves API compatibility. Recommended for v0.3.0.

**`UndoState` with bitboards:**
```cpp
struct UndoState {
    uint64_t bitboards[12];    // full board snapshot — 96 bytes
    uint64_t enPassantBB;      // en passant target square
    uint8_t  castlingRights;   // bitmask: KQkq = bits 3210
    uint8_t  halfMoveClock;
    uint8_t  colorToMove;
};
```
Or, more efficiently, store only the delta (captured piece type + square, changed castling rights, etc.) — this is smaller and faster for search. Full snapshot is simpler to implement and verify first.

---

---

## What "Legal" Means in This Context

A legal move is one where:
1. The piece moves according to its movement rules (already done - pseudo-legal).
2. The resulting board position does NOT leave the moving side's king in check.
3. Special moves (castling, promotion) follow their additional preconditions.
4. Checkmate and stalemate can be detected as a consequence.

---

## Phase 0 - Bitboard Foundation Rewrite

**Estimated time: 16-20 hours**
**Blocking**: All other phases depend on this. Do not start Phase 1 until Phase 0 is complete and all 300 existing tests pass.

### Step 0.1 - Define bitboard types and square indexing (2-3 hours)

Add to `include/core/core.h`:
```cpp
// Square index: 0=a1, 7=h1, 8=a2, ..., 63=h8
// Bit n of a uint64_t corresponds to square n
using Bitboard = uint64_t;

// Piece indices for the bitboard array (Board::bb[12])
enum BBIndex : uint8_t {
    WP=0, WN=1, WB=2, WR=3, WQ=4, WK=5,
    BP=6, BN=7, BB=8, BR=9, BQ=10, BK=11
};

// Castling rights bitmask
constexpr uint8_t CASTLE_WK = 0b0001;
constexpr uint8_t CASTLE_WQ = 0b0010;
constexpr uint8_t CASTLE_BK = 0b0100;
constexpr uint8_t CASTLE_BQ = 0b1000;
```

Helper macros/functions:
```cpp
constexpr Bitboard squareBB(uint8_t sq)    { return 1ULL << sq; }
constexpr uint8_t  sqOf(uint8_t rank, uint8_t file) { return rank * 8 + file; }
constexpr uint8_t  rankOf(uint8_t sq) { return sq >> 3; }
constexpr uint8_t  fileOf(uint8_t sq) { return sq & 7; }
```

### Step 0.2 - Precomputed attack tables (4-5 hours)

New file: `src/chess/attacks.cpp` / `include/chess/attacks.h`

```cpp
namespace fenrir::attacks {
    extern Bitboard knight[64];
    extern Bitboard king[64];
    extern Bitboard pawn_push[2][64];   // [color][sq]
    extern Bitboard pawn_attacks[2][64];
    // Ray attacks: N, NE, E, SE, S, SW, W, NW
    extern Bitboard ray[8][64];

    void init();   // called once at program start
    Bitboard rookAttacks(uint8_t sq, Bitboard occupancy);
    Bitboard bishopAttacks(uint8_t sq, Bitboard occupancy);
    Bitboard queenAttacks(uint8_t sq, Bitboard occupancy);
}
```

Slider attacks use classical ray blocker algorithm (no magic bitboards yet — correctness first):
```cpp
// Example: east ray
Bitboard east = ray[EAST][sq];
Bitboard blockers = east & occupancy;
if (blockers) east ^= ray[EAST][lsb(blockers)];
```

### Step 0.3 - Rewrite `Board` internal representation (5-6 hours)

Replace `vector<vector<unique_ptr<Piece>>>` with:
```cpp
// src/chess/board.cpp (private section)
Bitboard bb[12];          // indexed by BBIndex
Bitboard occupancy[3];    // [WHITE, BLACK, BOTH] — derived, kept in sync
uint64_t enPassantBB;     // single bit or 0
uint8_t  castlingRights;  // bitmask
uint8_t  colorToMove;
uint8_t  halfMoveClock;
uint8_t  fullMoves;
```

`Board::buildBoard()` (called from FEN parsing) populates `bb[]` by iterating the FEN placement string. The 12 bitboards replace the old heap-allocated grid entirely.

`Board::getPiece(rank, file)` reconstructs a lightweight value on demand:
- Compute `sq = rank*8 + file`
- Iterate `bb[0..11]`, find which has bit `sq` set
- Return the corresponding piece type/color
- Returns `nullptr`-equivalent if square is empty

`Board::move()` is rewritten as a sequence of bitboard updates:
- `bb[movingPiece] ^= (fromBB | toBB)` — moves the piece
- `bb[capturedPiece] &= ~toBB` — removes captured piece (if any)
- Update `occupancy[]`, `enPassantBB`, `castlingRights` accordingly

**`Piece` class**: Kept as a lightweight value type (no heap allocation, no `unique_ptr`). The `vector<vector<unique_ptr<Piece>>>` is removed entirely. All Piece objects previously returned by `getPiece()` are reconstructed from bitboard queries. Tests that hold `Piece*` pointers must be updated — they now hold reconstructed values.

### Step 0.4 - Rewrite `Moves` to use bitboards (5-6 hours)

Each `generateXxxMoves()` method becomes a bitboard operation:
```cpp
// Example: knight moves
Bitboard knights = board.getBB(WN);  // or BN
while (knights) {
    uint8_t sq = popLsb(knights);
    Bitboard targets = attacks::knight[sq] & ~board.getOccupancy(WHITE);
    // for each target: emit Move(sq, target, NORMAL or CAPTURE)
}
```

Pawn moves use the precomputed push/attack tables with occupancy masking.
Sliding pieces call `attacks::rookAttacks(sq, occupancy)` etc.

This entirely replaces the current pointer-walk logic in `src/chess/moves.cpp`.

### Step 0.5 - Pass all 300 existing tests (checkpoint)

Before proceeding to Phase 1: `make test` must report 300/300 passing. Coverage must remain at 100%. This is the hard gate between Phase 0 and everything else.

---

## AbstractBoard Analysis - Changes Required (Updated for Bitboards)

**File**: `include/abstract/board.h`

After Phase 0, the interface needs these additions (the old simulation/copy concerns no longer apply — make/unmake with bitboards is 12 integer saves, not heap allocation):

**1. `getCastlingRights()` → replaced by `getCastlingRightsBB()`**
- After Phase 0, castling rights are stored as a `uint8_t` bitmask (`CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ`), not a string.
- `AbstractBoard` should expose: `virtual uint8_t getCastlingRights() const = 0`
- `Board` implements this from its internal `castlingRights` field.

**2. `getColor()`**
- Legal move filtering must know whose turn it is.
- `AbstractBoard` needs `virtual uint8_t getColor() const = 0`
- Needed by `Engine::isCheckmate()` / `Engine::isStalemate()`.

**3. Bitboard accessors needed by `Moves` and check detection**
- `Moves` now works directly on bitboards. `AbstractBoard` should expose:
  ```cpp
  virtual Bitboard getBB(uint8_t bbIndex) const = 0;       // one of the 12 piece boards
  virtual Bitboard getOccupancy(uint8_t color) const = 0;  // WHITE, BLACK, or BOTH
  virtual Bitboard getEnPassantBB() const = 0;
  ```
- `getPiece(rank, file)` is kept for consumers (e.g., `Engine::getBoardView()`, tests) that need human-readable piece info.

**4. No simulation/copy mechanism needed**
- With bitboards, make/unmake saves 12 `uint64_t` values + a few bytes. Simulation is `applyMove()` + check query + `undoMove()` with zero heap allocation. The old Options A/B/C are obsolete.

### Summary of additions needed:

| Method | Reason |
|--------|--------|
| `virtual uint8_t getCastlingRights() const = 0` | Castling generation (bitmask, not string) |
| `virtual uint8_t getColor() const = 0` | Legal filtering, checkmate/stalemate |
| `virtual Bitboard getBB(uint8_t) const = 0` | Bitboard-based move generation in `Moves` |
| `virtual Bitboard getOccupancy(uint8_t) const = 0` | Slider attack masking |
| `virtual Bitboard getEnPassantBB() const = 0` | En passant move generation |

---

## Phase 1 - AbstractBoard + Board Extensions

**Estimated time: 2-3 hours** (can overlap with Phase 0 Step 0.3)

- Add all new virtual methods to `AbstractBoard` (see table above).
- Implement all in `Board` from the new bitboard fields.
- Update tests: `board.test.cpp` and `engine.test.cpp` need coverage for new accessors.
- Run `make test` and `make coverage` — must remain at 100%.

---

## Phase 2 - Check Detection

**Estimated time: 2-3 hours** (faster than mailbox version due to attack tables)

**What**: Determine if a given side's king is currently attacked.

**Where**: `Board::isInCheck(uint8_t color) const`

**How** (with bitboards):
```cpp
bool Board::isInCheck(uint8_t color) const {
    uint8_t kingSq = lsb(bb[color == WHITE ? WK : BK]);
    uint8_t opp    = color ^ 1;
    // Is king square attacked by any opponent piece?
    return (attacks::knight[kingSq]         & bb[opp==WHITE ? WN : BN]) ||
           (attacks::pawn_attacks[color][kingSq] & bb[opp==WHITE ? WP : BP]) ||
           (attacks::king[kingSq]           & bb[opp==WHITE ? WK : BK]) ||
           (attacks::rookAttacks(kingSq, occupancy[BOTH]) & (bb[opp==WHITE ? WR : BR] | bb[opp==WHITE ? WQ : BQ])) ||
           (attacks::bishopAttacks(kingSq, occupancy[BOTH]) & (bb[opp==WHITE ? WB : BB] | bb[opp==WHITE ? WQ : BQ]));
}
```
This is O(1) — a handful of bitwise operations and table lookups. Not O(pieces * move generation) like the old approach.

**Tests needed**:
- King is not in check on starting position.
- King in check by single attacker (each piece type).
- King in check by multiple attackers (double check).
- King not in check after move resolves it.

---

## Phase 3 - Make/Unmake + Legal Move Filtering

**Estimated time: 5-6 hours** (make/unmake moved here — it enables legal filtering without copies)

**Note**: With bitboards, make/unmake is cheap enough to implement *before* legal filtering. There is no longer a reason to defer it to Phase 7. This is the correct order.

**UndoState** (bitboard version):
```cpp
struct UndoState {
    Bitboard bb[12];         // full bitboard snapshot — 96 bytes total
    Bitboard enPassantBB;
    uint8_t  castlingRights;
    uint8_t  halfMoveClock;
    // colorToMove is implicit (flip after apply, flip back on undo)
};
```
Or delta-only (smaller, faster):
```cpp
struct UndoState {
    uint8_t  capturedPieceType;  // BBIndex of captured piece, or 255 if none
    uint8_t  capturedSq;
    Bitboard enPassantBB;        // before the move
    uint8_t  castlingRights;     // before the move
    uint8_t  halfMoveClock;      // before the move
    MoveType moveType;
};
```
Delta-only is preferred — save only what changes, not the whole board. Implement delta-only first.

**`Board::applyMove(const Move&) -> UndoState`**:
- Save current en passant, castling rights, half-move clock, captured piece info.
- Update bitboards: move the piece, remove captured piece, handle special cases.
- Return the undo state.

**`Board::undoMove(const Move&, const UndoState&)`**:
- Reverse the bitboard updates.
- Restore en passant, castling rights, half-move clock.
- Flip color.

**Legal filtering** (replaces old copy-based approach):
```cpp
for (auto &move : pseudoLegalMoves) {
    UndoState state = board.applyMove(move);
    bool legal = !board.isInCheck(movingSide);
    board.undoMove(move, state);
    if (legal) legalMoves.push_back(move);
}
```
Zero allocation per candidate move.

**`Engine::undoMove()`**: Pops from an internal `std::vector<UndoState>` history stack and calls `Board::undoMove()`.

**`Engine::makeMove()` upgrade**: The v0.2.0 implementation validates against pseudo-legal moves only. Once Phase 3 is complete, `makeMove()` must validate against the **full legal move list** (post-filtering). The validation loop already exists in `engine.cpp` — the only change is replacing the pseudo-legal `Moves::getInstance().generateMoves()` call with the legal `Engine::generateMoves()` call. Any move not in the legal list is rejected: the board state is unchanged and an error is logged. This is a hard contract — agents cannot construct a `Move` object directly to bypass Fenrir's rules.

**Tests needed**:
- Apply + undo a normal move: board state identical to before.
- Apply + undo a capture: captured piece bitboard restored.
- Apply + undo en passant: captured pawn bit restored.
- Apply + undo castling: both king and rook bits restored, castling rights restored.
- Apply + undo promotion: pawn bit restored, promoted piece gone.
- `Engine::undoMove()` after `makeMove()` restores full position.
- Multiple `undoMove()` calls walk back multiple moves correctly.
- Pinned piece cannot move (legal filtering test).
- En passant discovery check correctly filtered.
- `makeMove()` with a move not in the legal list: board unchanged, error logged.
- `makeMove()` with a syntactically valid but physically impossible move (e.g., knight to a non-knight square): rejected.

---

## Phase 4 - Castling Generation

**Estimated time: 3-4 hours**

**What**: Generate `CASTLE_KINGSIDE` and `CASTLE_QUEENSIDE` moves.

**Where**: `Moves::generateKingMoves()`

**How** (with bitboards):
```cpp
// Kingside white: king e1->g1, rook h1->f1
if ((castlingRights & CASTLE_WK) &&
    !(occupancy[BOTH] & 0x60ULL) &&     // f1, g1 empty
    !board.isInCheck(WHITE) &&
    !isAttacked(f1, BLACK) &&
    !isAttacked(g1, BLACK)) {
    emitMove(e1, g1, CASTLE_KINGSIDE);
}
```
`Board::move()` for castling: update both king and rook bitboards, clear the used castling right bits.

**Tests needed**: same as before (see original plan).

---

## Phase 5 - Pawn Promotion

**Estimated time: 2-3 hours**

No structural change from original plan — promotion logic is in `generatePawnMoves()`, checking if destination rank is 0 or 7. With bitboards, the rank check is `rankOf(toSq) == 7` or `== 0`. Emit four `Move` objects with `MoveType::PROMOTION` and different `promotionPiece` values. `Board::applyMove()` handles the promotion by clearing the pawn bit and setting the promoted piece bit.

**Tests needed**: same as before (see original plan).

---

## Phase 6 - Checkmate and Stalemate Detection

**Estimated time: 1-2 hours** (simpler with legal move generation already working)

No structural change from original plan. `isCheckmate(color)` and `isStalemate(color)` enumerate all legal moves for `color`; if the list is empty, apply the check/no-check distinction.

**Tests needed**: same as before.

---

## Execution Order and Dependencies

```
Phase 0: Bitboard foundation rewrite         (no dependencies — must complete before anything else)
  0.1 Types + square helpers in core.h
  0.2 Precomputed attack tables (attacks.h/cpp)
  0.3 Rewrite Board internals (bb[12], occupancy, castlingRights bitmask)
  0.4 Rewrite Moves to use bitboards
  0.5 Gate: make test 300/300, coverage 100%
    |
Phase 1: AbstractBoard extensions            (needs Phase 0.3 for new Board fields)
    |
Phase 2: Check detection                     (needs Phase 0.2 attack tables + Phase 1 getColor)
    |
Phase 3: Make/unmake + Legal filtering       (needs Phase 2 — make/unmake before filtering)
    |              \
Phase 4: Castling   Phase 5: Promotion       (Phase 4 needs Phase 2; Phase 5 needs Phase 3)
    |              /
Phase 6: Checkmate / Stalemate               (needs Phases 3, 4, 5)
```

---

## Coverage Requirement

Every phase must end with `make test` and `make coverage` passing at 100%. Do not proceed to the next phase with failing tests or a coverage gap. New code that cannot be covered (e.g., defensive null checks) should be avoided or justified.

---

## Files That Will Change

| File | Changes |
|------|---------|
| `include/core/core.h` | Add `Bitboard` typedef, `BBIndex` enum, `CASTLE_*` bitmask constants, square helpers |
| `include/chess/attacks.h` | **New file** — attack table declarations, `rookAttacks()`, `bishopAttacks()`, `queenAttacks()` |
| `src/chess/attacks.cpp` | **New file** — precomputed knight/king/pawn tables, ray-based slider attack generation, `init()` |
| `include/abstract/board.h` | Add `getCastlingRights()`, `getColor()`, `getBB()`, `getOccupancy()`, `getEnPassantBB()` |
| `include/chess/board.h` | Replace `vector<vector<unique_ptr<Piece>>>` with `bb[12]` + occupancy fields; add `isInCheck()`, `applyMove()`, `undoMove()` |
| `src/chess/board.cpp` | Full rewrite of internal representation; `buildBoard()` populates bitboards; `getPiece()` reconstructs from bitboards; `move()` → `applyMove()`/`undoMove()` |
| `include/chess/piece.h` | Kept as value type (no heap); `unique_ptr` usage removed |
| `src/chess/piece.cpp` | Remove heap management; keep piece identity helpers |
| `include/chess/moves.h` | Interface unchanged |
| `src/chess/moves.cpp` | Full rewrite to bitboard iteration; all `generateXxxMoves()` become bitboard operations; add castling (Phase 4) and promotion (Phase 5) |
| `include/engine/engine.h` | Add `isCheckmate()`, `isStalemate()`, `undoMove()` |
| `src/engine/engine.cpp` | Legal filtering in `generateMoves()`; checkmate/stalemate; `undoMove()` with `vector<UndoState>` history |
| `tests/unit/board.test.cpp` | Update for bitboard-based accessors; new tests for check detection, make/unmake, castling rights |
| `tests/unit/moves.test.cpp` | Update for bitboard move generation; new tests for castling, promotion |
| `tests/unit/engine.test.cpp` | New tests for legal filtering, checkmate, stalemate, `undoMove()` |
| `tests/unit/attacks.test.cpp` | **New file** — attack table correctness, slider generation for all occupancy patterns |
| `AI_CONTEXT.md` | Update architecture section (bitboard representation), completed/pending tables |
| `README.md` | Update feature table and roadmap |
| `RELEASE_NOTES.md` | Add v0.3.0 entry |
