# 🏗️ Architecture Refactoring Roadmap

> **Purpose**: Track critical architectural improvements needed before implementing game rules.
> **Start Date**: 2025-11-09
> **Target**: Complete foundation before v0.3.0 (game rules phase)
> **Status**: 🚧 In Progress

---

## 🎯 Guiding Principles

1. **Foundation before features** - Get architecture right first
2. **Maintain 100% test coverage** - Every change must be tested
3. **One change at a time** - Small, focused commits
4. **Tests pass continuously** - Never break the build
5. **Update documentation** - Keep docs in sync with changes

---

## 📋 Phase 1: Critical Foundation Fixes

### ✅ Task 1.1: Fix C++ Naming Conventions (CRITICAL)
**Status**: ⬜ Not Started
**Priority**: 🔴 CRITICAL (Undefined Behavior)
**Estimated Time**: 2-3 hours
**Blocker**: No

**Problem**: Using Python-style `__private__()` naming is reserved for implementation in C++ and technically undefined behavior.

**Decision**: Use simple `camelCase` for private methods and parameters (standard C++ practice).

**Changes Required**:

#### Files to Update:
- [ ] `include/chess/board.h` + `src/chess/board.cpp`
  - [ ] `__build_board__` → `buildBoard`
  - [ ] `__generate_placement_from_board__` → `generatePlacementFromBoard`
  - [ ] `__log_piece_action__` → `logPieceAction`
  - [ ] All `__parameter` → `parameter` or use clear names

- [ ] `include/chess/piece.h` + `src/chess/piece.cpp`
  - [ ] `__has_piece_moved__` → `hasPieceMoved`
  - [ ] All `__parameter` → `parameter`

- [ ] `include/chess/moves.h` + `src/chess/moves.cpp`
  - [ ] `__bishop__` → `generateBishopMoves`
  - [ ] `__capture__` → `addCaptureMove`
  - [ ] `__king__` → `generateKingMoves`
  - [ ] `__knight__` → `generateKnightMoves`
  - [ ] `__log_generated_moves__` → `logGeneratedMoves`
  - [ ] `__pawn__` → `generatePawnMoves`
  - [ ] `__queen__` → `generateQueenMoves`
  - [ ] `__rook__` → `generateRookMoves`
  - [ ] `__slide__` → `slideInDirections`
  - [ ] All `__parameter` → `parameter`

- [ ] `include/chess/fen.h` + `src/chess/fen.cpp`
  - [ ] `__split__` → `splitString`
  - [ ] `__validate_chess_rules__` → `validateChessRules`
  - [ ] `__validate_placement__` → `validatePlacement`
  - [ ] `__validate_pawn_placement__` → `validatePawnPlacement`
  - [ ] `__validate_king_safety__` → `validateKingSafety`
  - [ ] All `__parameter` → `parameter`

- [ ] `include/pgn/pgn.h` + `src/pgn/pgn.cpp`
  - [ ] `__clear_stream_flags__` → `clearStreamFlags`
  - [ ] `__set_metadata__` → `setMetadata`
  - [ ] All `__parameter` → `parameter`

- [ ] `include/chrono/chrono.h` + `src/chrono/chrono.cpp`
  - [ ] `__get_local_time__` → `getLocalTime`
  - [ ] `__get_raw_time__` → `getRawTime`
  - [ ] `__get_time_with_format__` → `getTimeWithFormat`
  - [ ] All `__parameter` → `parameter`

- [ ] All test files in `tests/unit/`
  - [ ] Update any tests that reference private methods (if any)
  - [ ] Verify all 262 tests still pass

**Completion Criteria**:
- [ ] All `__method__` patterns removed
- [ ] All `__parameter` patterns removed
- [ ] All tests pass (`make test`)
- [ ] 100% coverage maintained (`make coverage`)
- [ ] Code compiles without warnings
- [ ] Commit with message: "refactor: migrate from Python-style to C++ naming conventions"

---

### ✅ Task 1.2: Create Proper Move Class
**Status**: ⬜ Not Started
**Priority**: 🟡 HIGH
**Estimated Time**: 4-6 hours
**Blocker**: No (but recommended after 1.1)

**Problem**: Using `pair<const string, const string>` is not semantic, not type-safe, and can't store metadata.

**Solution**: Create a proper `Move` class.

**Implementation**:

#### Step 1: Create Move Class
- [ ] Create `include/chess/move.h`
- [ ] Create `src/chess/move.cpp`
- [ ] Implement basic `Move` class:

```cpp
// include/chess/move.h
#pragma once
#include <string>
#include <stdint.h>

namespace fenrir {

enum class MoveType {
    NORMAL,
    CAPTURE,
    EN_PASSANT,
    CASTLE_KINGSIDE,
    CASTLE_QUEENSIDE,
    PROMOTION
};

class Move {
private:
    std::string from;
    std::string to;
    MoveType type;
    char promotionPiece;  // 'q', 'r', 'b', 'n' or '\0'

public:
    Move(const std::string& from, const std::string& to,
         MoveType type = MoveType::NORMAL, char promotionPiece = '\0');
    ~Move();

    // Getters
    const std::string& getFrom() const;
    const std::string& getTo() const;
    MoveType getType() const;
    char getPromotionPiece() const;

    // Utility methods
    bool isCapture() const;
    bool isPromotion() const;
    bool isCastling() const;

    // String representations
    std::string toAlgebraic() const;  // "e2e4"
    std::string toUCI() const;        // "e2e4" or "e7e8q"

    // For logging
    std::string toString() const;
};

}  // namespace fenrir
```

#### Step 2: Update API
- [ ] Change `Engine::generate_moves()` return type from `vector<pair<...>>` to `vector<Move>`
- [ ] Update `Moves::generate_moves()` signature to use `vector<Move>&`
- [ ] Update all internal move generation methods
- [ ] Update `Board` to work with `Move` objects if needed

#### Step 3: Update Tests
- [ ] Create `tests/unit/move.test.cpp` with comprehensive tests
- [ ] Update all existing tests that use `pair<...>` to use `Move`
- [ ] Test all move types (normal, capture, en passant, etc.)
- [ ] Verify all 262+ tests pass

#### Step 4: Update Documentation
- [ ] Update `README.md` API examples
- [ ] Update `AI_CONTEXT.md` architecture section
- [ ] Add Move class to documentation

**Completion Criteria**:
- [ ] `Move` class fully implemented and tested
- [ ] All `pair<const string, const string>` removed from codebase
- [ ] All tests pass (`make test`)
- [ ] 100% coverage maintained (`make coverage`)
- [ ] Documentation updated
- [ ] Commit with message: "feat: introduce Move class to replace pair<string, string>"

---

### ✅ Task 1.3: Create Board Interface (IBoardView)
**Status**: ⬜ Not Started
**Priority**: 🟡 HIGH
**Estimated Time**: 3-4 hours
**Blocker**: Recommended after 1.1 and 1.2

**Problem**: `Moves` class takes `const Board*` everywhere, creating tight coupling and making testing difficult.

**Solution**: Create an interface that abstracts board queries.

**Implementation**:

#### Step 1: Create IBoardView Interface
- [ ] Create `include/chess/iboard_view.h`

```cpp
// include/chess/iboard_view.h
#pragma once
#include <string>
#include <stdint.h>

namespace fenrir {

class Piece;  // Forward declaration

class IBoardView {
public:
    virtual ~IBoardView() = default;

    // Query methods that Moves needs
    virtual const Piece* getPiece(uint8_t rank, uint8_t file) const = 0;
    virtual const std::string& getEnPassant() const = 0;

    // For future use (check detection, etc.)
    virtual uint8_t getCurrentColor() const = 0;
};

}  // namespace fenrir
```

#### Step 2: Make Board Implement Interface
- [ ] Update `include/chess/board.h` to inherit from `IBoardView`
- [ ] Implement interface methods (may just delegate to existing methods)
- [ ] Ensure no breaking changes to existing API

#### Step 3: Update Moves Class
- [ ] Change all `const Board*` parameters to `const IBoardView&`
- [ ] Update method signatures in `include/chess/moves.h`
- [ ] Update implementation in `src/chess/moves.cpp`
- [ ] Verify functionality unchanged

#### Step 4: Create Mock for Testing
- [ ] Create `tests/mocks/mock_board_view.h`
- [ ] Implement mock that allows setting up specific board states
- [ ] Use mock in existing tests where appropriate

#### Step 5: Update Tests
- [ ] Add tests for IBoardView interface
- [ ] Update existing tests to use interface where beneficial
- [ ] Verify all tests pass

**Completion Criteria**:
- [ ] `IBoardView` interface created and documented
- [ ] `Board` implements interface
- [ ] `Moves` uses interface instead of concrete `Board*`
- [ ] Mock implementation available for testing
- [ ] All tests pass (`make test`)
- [ ] 100% coverage maintained (`make coverage`)
- [ ] Commit with message: "refactor: introduce IBoardView interface to reduce coupling"

---

### ✅ Task 1.4: Add Const Correctness
**Status**: ⬜ Not Started
**Priority**: 🟢 MEDIUM
**Estimated Time**: 2-3 hours
**Blocker**: Recommended after 1.1

**Problem**: Missing `const` on methods that don't modify state affects API contract and optimization.

**Solution**: Audit all methods and add `const` where appropriate.

**Implementation**:

#### Step 1: Audit All Classes
- [ ] Review `Board` class methods
- [ ] Review `Piece` class methods
- [ ] Review `Moves` class methods
- [ ] Review `Engine` class methods
- [ ] Review `Fen` class methods
- [ ] Review all other classes

#### Step 2: Add Const to Methods
- [ ] Mark all non-mutating methods as `const`
- [ ] Mark all parameters that aren't modified as `const` or `const&`
- [ ] Use `const` references for parameters where appropriate
- [ ] Update headers and implementations

#### Step 3: Fix Compilation Errors
- [ ] Resolve any const-correctness errors that arise
- [ ] May need to mark some members as `mutable` (e.g., for logging mutexes)
- [ ] Verify behavior unchanged

#### Step 4: Update Tests
- [ ] Ensure tests work with new const signatures
- [ ] Add tests that verify const correctness (e.g., can call methods on const objects)
- [ ] Verify all tests pass

**Completion Criteria**:
- [ ] All appropriate methods marked `const`
- [ ] All appropriate parameters marked `const` or `const&`
- [ ] Code compiles without warnings
- [ ] All tests pass (`make test`)
- [ ] 100% coverage maintained (`make coverage`)
- [ ] Commit with message: "refactor: add const correctness throughout codebase"

---

### ✅ Task 1.5: Review Memory Management
**Status**: ⬜ Not Started
**Priority**: 🟢 MEDIUM
**Estimated Time**: 2-3 hours
**Blocker**: No

**Problem**: `Board` has `vector<vector<Piece*>>` with raw pointers. Need to verify ownership and ensure no leaks.

**Implementation**:

#### Step 1: Audit Current Implementation
- [ ] Review `Board` constructor and destructor
- [ ] Review `Board::__build_board__` (or new name after 1.1)
- [ ] Identify who owns the `Piece*` objects
- [ ] Check for any memory leaks

#### Step 2: Document Ownership
- [ ] Add comments clarifying ownership
- [ ] Ensure destructor properly deletes all pieces
- [ ] Verify copy constructor and assignment operator (if needed)

#### Step 3: Consider Smart Pointers (Optional)
- [ ] Evaluate if `unique_ptr<Piece>` would be better
- [ ] If yes, refactor to use `unique_ptr`
- [ ] If no, document why raw pointers are acceptable

#### Step 4: Add Tests
- [ ] Add tests for proper cleanup (valgrind or ASAN)
- [ ] Verify no memory leaks
- [ ] Test edge cases (empty board, reset, etc.)

**Completion Criteria**:
- [ ] Memory ownership clearly documented
- [ ] No memory leaks verified (valgrind or ASAN)
- [ ] All tests pass (`make test`)
- [ ] Decision documented (smart pointers or raw pointers)
- [ ] Commit with message: "refactor: clarify memory ownership in Board class"

---

## 📋 Phase 2: Architecture Enhancements (Optional)

### ✅ Task 2.1: Consider Strategy Pattern for Piece Moves
**Status**: ⬜ Not Started
**Priority**: 🔵 LOW
**Estimated Time**: 8-12 hours
**Blocker**: Complete Phase 1 first

**Problem**: Switch statement in `Moves::generate_moves()` is not extensible.

**Decision Point**: Is this worth the complexity for chess (only 6 piece types)?

**Options**:
1. **Keep switch statement** - Simple, works fine for chess
2. **Strategy pattern** - More extensible, better OOP
3. **Polymorphic pieces** - Most flexible, most complex

**Implementation** (if decided to proceed):
- [ ] Design chosen pattern
- [ ] Implement for one piece type as proof of concept
- [ ] Evaluate benefits vs. complexity
- [ ] Decide: proceed or abandon
- [ ] If proceed: migrate all piece types
- [ ] Update tests

**Note**: This may be overkill for chess. Revisit after Phase 1 complete.

---

### ✅ Task 2.2: Add IPiece Interface
**Status**: ⬜ Not Started
**Priority**: 🔵 LOW
**Estimated Time**: 2-3 hours
**Blocker**: Complete Phase 1 first

**Problem**: `Piece` class not mockable for testing.

**Solution**: Create `IPiece` interface.

**Implementation**:
- [ ] Create `include/chess/ipiece.h`
- [ ] Define interface with all public methods
- [ ] Make `Piece` implement interface
- [ ] Update code to use interface where beneficial
- [ ] Create mock implementation for tests

**Note**: May not be necessary if testing strategy changes. Evaluate after Phase 1.

---

## 📊 Progress Tracking

### Overall Progress: 0% Complete

| Phase | Tasks | Completed | Status |
|-------|-------|-----------|--------|
| Phase 1: Critical Fixes | 5 | 0/5 | ⬜ Not Started |
| Phase 2: Enhancements | 2 | 0/2 | ⬜ Not Started |

### Task Status Legend
- ⬜ Not Started
- 🔄 In Progress
- ✅ Complete
- ❌ Blocked

---

## 📝 Completion Checklist (Before Moving to Game Rules)

Before starting game rules implementation (check detection, move validation, etc.), ensure:

- [ ] All Phase 1 tasks complete
- [ ] All 262+ tests passing
- [ ] 100% test coverage maintained
- [ ] No compiler warnings
- [ ] Documentation updated (README.md, AI_CONTEXT.md)
- [ ] Code review completed (self or peer)
- [ ] Commit history clean and descriptive
- [ ] Branch merged to main/develop

---

## 🎯 Success Metrics

**Code Quality**:
- Zero uses of `__private__()` naming
- 100% test coverage maintained
- Clean separation of concerns
- No memory leaks

**Testability**:
- Can mock Board for unit tests
- Can create Move objects easily
- Interface-based design

**Maintainability**:
- Clear naming conventions
- Proper const correctness
- Well-documented ownership

**Performance**:
- No performance regressions
- Efficient move generation maintained

---

## 📅 Timeline

**Start Date**: 2025-11-09
**Target Completion**: 2025-11-23 (2 weeks)
**Actual Completion**: _TBD_

### Week 1: Critical Fixes
- Days 1-2: Task 1.1 (Naming conventions)
- Days 3-4: Task 1.2 (Move class)
- Days 5-7: Task 1.3 (IBoardView interface)

### Week 2: Polish & Review
- Days 1-2: Task 1.4 (Const correctness)
- Days 3-4: Task 1.5 (Memory management)
- Days 5-7: Buffer for testing, documentation, review

---

## 🚀 After Completion

Once this refactoring is complete, we'll be ready to implement game rules with a solid foundation:

1. Check detection (using IBoardView interface)
2. Move validation (using Move class with metadata)
3. Castling (using Move class with CASTLE type)
4. Pawn promotion (using Move class with promotion field)
5. Checkmate/Stalemate detection

The architecture will support these features cleanly without needing major refactoring.

---

**Last Updated**: 2025-11-09
**Document Owner**: Architecture Team
**Next Review**: After Phase 1 completion
