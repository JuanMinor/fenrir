# Scope: Milestone 1 - Utilities & Hardware Refactoring

## Files in Scope
- `include/core/core.h`
- `include/chrono/chrono.h`, `src/chrono/chrono.cpp`
- `include/logger/logger.h`, `src/logger/logger.cpp`
- `include/modifier/modifier.h`, `src/modifier/modifier.cpp`
- `include/utils/utils.h`, `src/utils/utils.cpp`
- `include/pgn/pgn.h`, `src/pgn/pgn.cpp`
- `include/hardware/hardware.h`, `src/hardware/hardware.cpp`
- `include/tuner/tuning_parameters.h`, `src/tuner/tuning_parameters.cpp`

## Requirements
1. **R1. Alphabetization & Cleanup**:
   - Alphabetize all functions and data placeholders in all headers and source files in scope.
   - Ensure constructors/destructors remain at the top.
   - Inline functions remain in headers.
   - Remove all old internal comments.
2. **R2. Javadoc Documentation**:
   - Add comprehensive Javadoc-style block comments to every function, utilizing `@param` and `@returns` tags.
3. **R3. Function Renaming & Clarity**:
   - Identify camelCase or unclear functions and rename them to clear, descriptive `snake_case` names.
   - E.g., `setCpuType` -> `set_cpu_type`, `setGpuType` -> `set_gpu_type`.
   - Update all function signatures across header/source pairs and all call sites in tests, main.cpp, and other codebase files.

## Completion Criteria
- Files are completely refactored and comply with R1, R2, and R3.
- Project compiles without errors (cmake, make).
- All unit tests pass.
- No integrity violations.
