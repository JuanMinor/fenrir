# 🚀 Fenrir Chess Engine - TODO & Improvement Roadmap

> **Purpose**: Track improvements, refactoring tasks, and feature development for Fenrir.
> **Last Updated**: 2025-11-08
> **Current Version**: 0.2.0-dev

---

## 🎯 Priority Legend

- 🔴 **Critical**: Blocks other work or causes issues
- 🟡 **High**: Important for code quality/maintainability
- 🟢 **Medium**: Nice to have, improves architecture
- 🔵 **Low**: Polish, minor improvements

---

## 📋 Code Quality & Refactoring

### 🟡 HIGH: Fix C++ Naming Conventions

**Issue**: Currently using Python-style `__private__()` naming, which is non-standard in C++ and technically reserved for implementation.

**Current:**
```cpp
void __slide__(...);
void __bishop__(...);
void __log_generated_moves__(...);
```

**Should be:**
```cpp
void slide(...);           // Option 1: Simple private methods
void bishop(...);
void logGeneratedMoves();

// OR

void slideImpl(...);       // Option 2: Explicit "implementation" suffix
void bishopImpl(...);
void logGeneratedMovesImpl();
```

**Tasks:**
- [ ] Choose naming convention (recommend: simple lowercase for private methods)
- [ ] Update all `__method__()` to new convention
- [ ] Update all `__parameter` to `parameter` or `param`
- [ ] Update tests if they reference private methods
- [ ] Ensure 100% coverage maintained

**Files to update:**
- `include/chess/moves.h`
- `src/chess/moves.cpp`
- `include/chess/board.h`
- `src/chess/board.cpp`
- `include/chess/piece.h`
- `src/chess/piece.cpp`
- All test files

**Estimated effort**: 2-3 hours

---

### 🟢 MEDIUM: Reduce Coupling Between Moves and Board

**Issue**: `Moves` class takes `const Board*` everywhere, creating tight coupling.

**Current:**
```cpp
void generate_moves(const Piece *piece, const Board *board, vector<...> &moves);
```

**Better approach:**
```cpp
// Option 1: Pass only what's needed
void generate_moves(const Piece *piece, 
                   function<const Piece*(uint8_t, uint8_t)> getPiece,
                   const string &enPassant,
                   vector<...> &moves);

// Option 2: Create a BoardView interface
class IBoardView {
    virtual const Piece* getPiece(uint8_t rank, uint8_t file) const = 0;
    virtual const string& getEnPassant() const = 0;
};

void generate_moves(const Piece *piece, const IBoardView &board, vector<...> &moves);
```

**Benefits:**
- Easier to test (can mock board state)
- Reduces dependencies
- More flexible for future changes

**Tasks:**
- [ ] Design interface or data structure for board queries
- [ ] Update `Moves` class to use new interface
- [ ] Update `Board` to implement interface (if using that approach)
- [ ] Update all call sites
- [ ] Add tests for new abstraction

**Estimated effort**: 4-6 hours

---

### 🟢 MEDIUM: Replace Switch Statement with Polymorphism

**Issue**: Move generation uses switch statement on piece type. Not extensible.

**Current:**
```cpp
switch (tolower(piece->get_alias())) {
    case 'b': this->__bishop__(...); break;
    case 'k': this->__king__(...); break;
    // etc.
}
```

**Better approach:**
```cpp
// Option 1: Strategy pattern
class MoveGenerator {
public:
    virtual void generate(const Piece*, const Board*, vector<...>&) = 0;
};

class PawnMoveGenerator : public MoveGenerator { ... };
class RookMoveGenerator : public MoveGenerator { ... };
// etc.

// Option 2: Piece-based polymorphism
class Piece {
public:
    virtual void generateMoves(const Board*, vector<...>&) const = 0;
};

class Pawn : public Piece { ... };
class Rook : public Piece { ... };
```

**Benefits:**
- Open/Closed Principle (open for extension, closed for modification)
- Easier to add new piece types
- Each piece type is self-contained

**Drawbacks:**
- More complex architecture
- More files to manage
- May be overkill for chess (only 6 piece types)

**Tasks:**
- [ ] Decide on approach (Strategy vs. Piece polymorphism)
- [ ] Create base classes/interfaces
- [ ] Implement concrete classes for each piece type
- [ ] Update `Moves` or `Piece` to use polymorphism
- [ ] Migrate tests
- [ ] Ensure 100% coverage

**Estimated effort**: 8-12 hours (significant refactor)

**Recommendation**: Consider this for v0.3.0+, not urgent for current state.

---

### 🟢 MEDIUM: Create Move Class Instead of pair<string, string>

**Issue**: Using `pair<const string, const string>` for moves is not semantic.

**Current:**
```cpp
vector<pair<const string, const string>> moves;
moves.emplace_back("e2", "e4");
```

**Better:**
```cpp
class Move {
private:
    string from;
    string to;
    MoveType type;  // NORMAL, CAPTURE, EN_PASSANT, CASTLE, PROMOTION
    char promotionPiece;  // For pawn promotion
    
public:
    Move(const string &from, const string &to, MoveType type = NORMAL);
    
    const string& getFrom() const { return from; }
    const string& getTo() const { return to; }
    MoveType getType() const { return type; }
    bool isCapture() const { return type == CAPTURE; }
    bool isPromotion() const { return type == PROMOTION; }
    
    string toAlgebraic() const;  // "e2e4" or "e7e8q"
    string toUCI() const;        // For UCI protocol
};

vector<Move> moves;
moves.emplace_back(Move("e2", "e4"));
```

**Benefits:**
- Type safety
- Can add metadata (capture, promotion, castling)
- Easier to extend for UCI protocol
- Self-documenting code

**Tasks:**
- [ ] Create `include/chess/move.h` and `src/chess/move.cpp`
- [ ] Define `Move` class with constructors
- [ ] Add `MoveType` enum
- [ ] Update all `vector<pair<...>>` to `vector<Move>`
- [ ] Update `Engine::generate_moves()` return type
- [ ] Update all tests
- [ ] Ensure 100% coverage

**Estimated effort**: 4-6 hours

---

### 🟡 HIGH: Add const Correctness Throughout

**Issue**: Many methods that don't modify state aren't marked `const`.

**Examples to fix:**
```cpp
// In Piece class
uint8_t get_rank(void) const;  // ✅ Already const
void set_rank(const uint8_t &__rank);  // ✅ Correctly not const

// In Board class - check if these modify state
Piece *get_piece(const uint8_t &__rank, const uint8_t &__file) const;  // ✅ Good

// In Moves class - these should probably be const
void __log_generated_moves__(...) const;  // ✅ Already const
```

**Tasks:**
- [ ] Audit all methods in all classes
- [ ] Mark methods that don't modify state as `const`
- [ ] Mark parameters that aren't modified as `const`
- [ ] Use `const` references where appropriate
- [ ] Fix any compilation errors
- [ ] Run tests to ensure behavior unchanged

**Estimated effort**: 2-3 hours

---

## 🎮 Chess Features (Game Rules)

### 🔴 CRITICAL: Implement Check Detection

**Priority**: Must be done before move validation.

**Requirements:**
- Detect if a king is in check
- Method: `bool Board::isInCheck(uint8_t color) const`
- Algorithm:
  1. Find king of given color
  2. Generate all opponent moves
  3. Check if any opponent move targets king's square

**Tasks:**
- [ ] Add `Board::findKing(uint8_t color)` method
- [ ] Add `Board::isInCheck(uint8_t color)` method
- [ ] Write comprehensive tests (king in check from all piece types)
- [ ] Test edge cases (multiple attackers, discovered check)
- [ ] Ensure 100% coverage

**Files to create/modify:**
- `include/chess/board.h` (add method declarations)
- `src/chess/board.cpp` (implement methods)
- `tests/unit/board.test.cpp` (add tests)

**Estimated effort**: 4-6 hours

---

### 🔴 CRITICAL: Implement Move Validation

**Priority**: Required for legal chess games.

**Requirements:**
- Validate that a move doesn't leave own king in check
- Filter out illegal moves from `generate_moves()`

**Approach:**
```cpp
bool Board::isMoveLegal(const Piece *piece, uint8_t toRank, uint8_t toFile) const {
    // 1. Make a copy of the board
    // 2. Apply the move on the copy
    // 3. Check if moving side's king is in check
    // 4. Return true if NOT in check
}

// Update generate_moves to filter:
void Moves::generate_moves(...) {
    vector<Move> pseudoLegalMoves;
    // ... generate all pseudo-legal moves ...
    
    // Filter out illegal moves
    for (const auto &move : pseudoLegalMoves) {
        if (board->isMoveLegal(piece, move.toRank, move.toFile)) {
            legalMoves.push_back(move);
        }
    }
}
```

**Tasks:**
- [ ] Implement `Board::copy()` or copy constructor
- [ ] Implement `Board::isMoveLegal()`
- [ ] Update `Moves::generate_moves()` to filter illegal moves
- [ ] Update `Engine::make_move()` to reject illegal moves
- [ ] Write tests for pinned pieces, discovered checks
- [ ] Ensure 100% coverage

**Dependencies**: Requires check detection first.

**Estimated effort**: 6-8 hours

---

### 🟡 HIGH: Implement Castling

**Requirements:**
- Kingside and queenside castling for both colors
- Validate castling conditions:
  - King and rook haven't moved
  - No pieces between king and rook
  - King not in check
  - King doesn't move through check
  - King doesn't end in check

**Tasks:**
- [ ] Add `Board::canCastle(uint8_t color, bool kingside)` method
- [ ] Update `Moves::__king__()` to generate castling moves
- [ ] Update `Board::move()` to handle castling (move both king and rook)
- [ ] Track king and rook movement in FEN castling rights
- [ ] Write comprehensive tests
- [ ] Ensure 100% coverage

**Files to modify:**
- `include/chess/board.h`
- `src/chess/board.cpp`
- `include/chess/moves.h`
- `src/chess/moves.cpp`
- `tests/unit/moves.test.cpp`

**Dependencies**: Requires check detection and move validation.

**Estimated effort**: 6-8 hours

---

### 🟡 HIGH: Implement Pawn Promotion

**Requirements:**
- When pawn reaches rank 0 or 7, promote to Q/R/B/N
- Default to queen if not specified
- Update FEN generation to handle promoted pieces

**Tasks:**
- [ ] Add promotion parameter to `Move` class (if created)
- [ ] Update `Board::move()` to detect pawn promotion
- [ ] Add `Engine::make_move()` overload with promotion piece
- [ ] Update piece creation to handle promoted pieces
- [ ] Write tests for all promotion types
- [ ] Ensure 100% coverage

**Files to modify:**
- `include/chess/move.h` (if Move class exists)
- `include/engine/engine.h`
- `src/engine/engine.cpp`
- `src/chess/board.cpp`
- `tests/unit/engine.test.cpp`

**Estimated effort**: 4-6 hours

---

### 🟢 MEDIUM: Implement Checkmate Detection

**Requirements:**
- Detect when a king is in checkmate
- Method: `bool Board::isCheckmate(uint8_t color) const`
- Algorithm:
  1. Check if king is in check
  2. Generate all legal moves for that color
  3. If no legal moves exist, it's checkmate

**Tasks:**
- [ ] Add `Board::isCheckmate(uint8_t color)` method
- [ ] Add `Board::getAllLegalMoves(uint8_t color)` helper
- [ ] Write tests for various checkmate patterns
- [ ] Test edge cases (smothered mate, back rank mate, etc.)
- [ ] Ensure 100% coverage

**Dependencies**: Requires move validation.

**Estimated effort**: 3-4 hours

---

### 🟢 MEDIUM: Implement Stalemate Detection

**Requirements:**
- Detect when a player has no legal moves but is not in check
- Method: `bool Board::isStalemate(uint8_t color) const`

**Tasks:**
- [ ] Add `Board::isStalemate(uint8_t color)` method
- [ ] Write tests for stalemate positions
- [ ] Ensure 100% coverage

**Dependencies**: Requires move validation.

**Estimated effort**: 2-3 hours

---

## 🏗️ Architecture Improvements

### 🟢 MEDIUM: Add Interfaces for Testability

**Issue**: Hard to mock dependencies for unit testing.

**Proposed interfaces:**
```cpp
// include/chess/iboard.h
class IBoardView {
public:
    virtual ~IBoardView() = default;
    virtual const Piece* getPiece(uint8_t rank, uint8_t file) const = 0;
    virtual const string& getEnPassant() const = 0;
    virtual bool isInCheck(uint8_t color) const = 0;
};

// include/chess/ipiece.h
class IPiece {
public:
    virtual ~IPiece() = default;
    virtual char getAlias() const = 0;
    virtual uint8_t getRank() const = 0;
    virtual uint8_t getFile() const = 0;
    virtual uint8_t getColor() const = 0;
    virtual bool hasMoved() const = 0;
};
```

**Benefits:**
- Can create mock implementations for testing
- Reduces coupling
- Follows Dependency Inversion Principle

**Tasks:**
- [ ] Create interface header files
- [ ] Make `Board` implement `IBoardView`
- [ ] Make `Piece` implement `IPiece`
- [ ] Update code to use interfaces where appropriate
- [ ] Create mock implementations for tests
- [ ] Ensure 100% coverage

**Estimated effort**: 6-8 hours

---

### 🔵 LOW: Consider PIMPL Idiom for ABI Stability

**Issue**: Changes to private members require recompilation of dependent code.

**PIMPL (Pointer to Implementation):**
```cpp
// board.h
class Board {
private:
    class Impl;
    unique_ptr<Impl> pImpl;
    
public:
    Board(const string &fen);
    ~Board();
    // ... public interface ...
};

// board.cpp
class Board::Impl {
    vector<vector<Piece*>> board;
    string castling;
    // ... all private members ...
};
```

**Benefits:**
- ABI stability (can change private members without recompiling clients)
- Faster compilation (less in headers)
- Better encapsulation

**Drawbacks:**
- Extra indirection (minor performance cost)
- More boilerplate
- Harder to debug

**Recommendation**: Consider for v1.0.0 when API stabilizes.

**Estimated effort**: 8-12 hours (significant refactor)

---

## 🤖 AI & Advanced Features

### 🔵 LOW: Implement Basic Position Evaluation

**Requirements:**
- Evaluate a position and return a score
- Method: `int Engine::evaluate() const`
- Simple evaluation:
  - Material count (pawn=1, knight=3, bishop=3, rook=5, queen=9)
  - Piece position bonuses (center control, etc.)

**Tasks:**
- [ ] Create `include/engine/evaluator.h`
- [ ] Implement material counting
- [ ] Add piece-square tables
- [ ] Write tests
- [ ] Ensure 100% coverage

**Dependencies**: None (can be done anytime).

**Estimated effort**: 4-6 hours

---

### 🔵 LOW: Implement Minimax Search

**Requirements:**
- Search game tree to find best move
- Method: `Move Engine::findBestMove(int depth)`
- Basic minimax algorithm

**Tasks:**
- [ ] Create `include/engine/search.h`
- [ ] Implement minimax recursion
- [ ] Add alpha-beta pruning
- [ ] Write tests
- [ ] Ensure 100% coverage

**Dependencies**: Requires move validation, checkmate detection, evaluation.

**Estimated effort**: 8-12 hours

---

### 🔵 LOW: Implement UCI Protocol

**Requirements:**
- Universal Chess Interface for GUI integration
- Commands: `uci`, `isready`, `position`, `go`, `stop`, `quit`

**Tasks:**
- [ ] Create `include/uci/uci.h`
- [ ] Implement UCI command parser
- [ ] Implement UCI response formatter
- [ ] Write integration tests
- [ ] Test with actual chess GUI

**Dependencies**: Requires AI search.

**Estimated effort**: 12-16 hours

---

## 📚 Documentation & Tooling

### 🟢 MEDIUM: Add Doxygen Comments

**Issue**: No API documentation generation.

**Tasks:**
- [ ] Add Doxygen comments to all public methods
- [ ] Create `Doxyfile` configuration
- [ ] Add `make docs` target
- [ ] Generate HTML documentation
- [ ] Add to CI/CD pipeline

**Estimated effort**: 4-6 hours

---

### 🟢 MEDIUM: Add CI/CD Pipeline

**Tasks:**
- [ ] Create `.github/workflows/ci.yml`
- [ ] Run tests on every push
- [ ] Check coverage (fail if < 100%)
- [ ] Build release artifacts
- [ ] Run static analysis (cppcheck, clang-tidy)

**Estimated effort**: 3-4 hours

---

### 🔵 LOW: Add Benchmarking Suite

**Tasks:**
- [ ] Create `tests/benchmark/` directory
- [ ] Add perft (performance test) for move generation
- [ ] Benchmark FEN parsing
- [ ] Track performance over time

**Estimated effort**: 4-6 hours

---

## 📅 Suggested Roadmap

### Phase 1: Code Quality (v0.2.1) - **2-3 weeks**
- [ ] Fix naming conventions (`__private__` → `private`)
- [ ] Add const correctness
- [ ] Create `Move` class
- [ ] Add Doxygen comments

### Phase 2: Game Rules (v0.3.0) - **3-4 weeks**
- [ ] Implement check detection
- [ ] Implement move validation
- [ ] Implement castling
- [ ] Implement pawn promotion
- [ ] Implement checkmate/stalemate detection

### Phase 3: Architecture (v0.4.0) - **2-3 weeks**
- [ ] Add interfaces (IBoardView, IPiece)
- [ ] Reduce coupling (Moves ↔ Board)
- [ ] Consider polymorphism for pieces (optional)

### Phase 4: AI (v0.5.0) - **4-6 weeks**
- [ ] Position evaluation
- [ ] Minimax with alpha-beta pruning
- [ ] Opening book (optional)

### Phase 5: Integration (v0.6.0) - **2-3 weeks**
- [ ] UCI protocol
- [ ] GUI integration testing
- [ ] Performance optimization

### Phase 6: Release (v1.0.0) - **1-2 weeks**
- [ ] Final documentation
- [ ] CI/CD pipeline
- [ ] Release artifacts
- [ ] Announcement

---

## 🎯 Quick Wins (Start Here!)

If you want to make immediate improvements, start with these:

1. **Fix naming conventions** (2-3 hours) - Easy, high impact
2. **Add const correctness** (2-3 hours) - Easy, improves code quality
3. **Create Move class** (4-6 hours) - Medium difficulty, sets up for future work
4. **Implement check detection** (4-6 hours) - Critical for game rules

---

## 📝 Notes

- **Always maintain 100% test coverage** - This is your superpower!
- **Write tests first** - Continue the TDD approach
- **Update documentation** - Keep README.md, AI_CONTEXT.md, RELEASE_NOTES.md in sync
- **Commit frequently** - Small, focused commits are easier to review
- **Don't break the build** - `make test` should always pass

---

**Total estimated effort for v1.0.0: ~80-120 hours (10-15 weeks part-time)**

Good luck! 🚀
