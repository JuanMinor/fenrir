# Refactoring Plan: Fenrir Codebase

This plan outlines the steps to refactor the Fenrir codebase, conforming to the three requirements:
- **R1. Alphabetization & Cleanup**: Alphabetize functions and data placeholders in all `.h`/`.cpp` files (constructors/destructors at top, inline in headers). Remove old internal comments.
- **R2. Javadoc Documentation**: Add block comments with `@param` and `@returns` tags to all functions.
- **R3. Function Renaming & Clarity**: Rename functions to `snake_case` for clarity, updating signatures and call sites.

## Plan Milestones

1. **Milestone 1: Utilities & Hardware Refactoring**
   - Files: `core/core.h`, `chrono/*`, `logger/*`, `modifier/*`, `utils/*`, `pgn/*`, `hardware/*`, `tuner/tuning_parameters.*`
   - Goal: Apply R1, R2, R3 to base utilities and hardware profiling.
   - Status: DONE

2. **Milestone 2: Rules & Board Representation Refactoring**
   - Files: `abstract/board.h`, `chess/attacks.*`, `chess/move.*`, `chess/fen.*`, `chess/board.*`, `chess/moves.*`
   - Goal: Apply R1, R2, R3 to board models, attack tables, FEN parser, move generator, and core representation.
   - Status: PLANNED

3. **Milestone 3: Engine & Tuning Refactoring**
   - Files: `engine/*`, `abi/*`, `tuner/auto_tuner.*`
   - Goal: Apply R1, R2, R3 to engine controller, C API wrapper, and performance auto-tuner.
   - Status: PLANNED

4. **Milestone 4: NN Search & UCI Loop Refactoring**
   - Files: `eval/*`, `search/*`, `uci/*`, `selfplay/*`
   - Goal: Apply R1, R2, R3 to ONNX neural network evaluator, MCTS search, selfplay generator, and UCI CLI loop.
   - Status: PLANNED

5. **Milestone 5: Verification, Hardening & Acceptance**
   - Goal: Compile cleanly, pass all tests, run white-box adversarial verification (if applicable) and integrity audits.
   - Status: PLANNED
