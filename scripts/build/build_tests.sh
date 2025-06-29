#!/bin/bash

# -----------------------------------------------------------------------------
# Fenrir Chess Engine Test Builder
# -----------------------------------------------------------------------------
# This script builds the test executable with coverage instrumentation.
# It performs the following operations:
#   1. Compiles all source files with coverage flags
#   2. Links the test executable with test frameworks
#   3. Ensures all dependencies are properly built
# -----------------------------------------------------------------------------

set -e

# Configuration
SRC_FILES=(
    "src/chess/board.cpp"
    "src/chrono/chrono.cpp"
    "src/engine/engine.cpp"
    "src/chess/fen.cpp"
    "src/logger/logger.cpp"
    "src/modifier/modifier.cpp"
    "src/chess/moves.cpp"
    "src/pgn/pgn.cpp"
    "src/chess/piece.cpp"
    "src/utils/utils.cpp"
)

TEST_SRC_FILES=(
    "tests/unit/fen.test.cpp"
    "tests/unit/board.test.cpp"
    "tests/unit/piece.test.cpp"
    "tests/unit/engine.test.cpp"
    "tests/unit/utils.test.cpp"
    "tests/unit/moves.test.cpp"
    "tests/unit/logger.test.cpp"
    "tests/unit/chrono.test.cpp"
    "tests/unit/modifier.test.cpp"
    "tests/unit/pgn.test.cpp"
)

BUILD_DIR="bin/build"
TEST_BUILD_DIR="bint/unit"
TEST_EXECUTABLE="$TEST_BUILD_DIR/tests"

# Compiler settings
CC="g++"
PROJECT_ROOT="/workspaces/fenrir"
COMMON_FLAGS="-I $PROJECT_ROOT -fPIC -DFENRIR_BUILD_DLL"
CXXFLAGS="$COMMON_FLAGS -g"
COVERAGE_FLAGS="-fprofile-arcs -ftest-coverage"
CXXFLAGS_TEST="$CXXFLAGS $COVERAGE_FLAGS"
TEST_LIBS="-lgtest -lgtest_main -lpthread"

echo "🔨 Building test executable with coverage..."
mkdir -p "$TEST_BUILD_DIR"

# Rebuild object files with coverage flags for testing
echo "🔄 Rebuilding source files with coverage instrumentation..."
for src_file in "${SRC_FILES[@]}"; do
    obj_file=$(echo "$src_file" | sed "s|src/|$BUILD_DIR/|" | sed 's|\.cpp|\.o|')
    echo "🔨 Compiling with coverage: $src_file -> $obj_file"
    mkdir -p "$(dirname "$obj_file")"
    $CC -c $CXXFLAGS_TEST -o "$obj_file" "$src_file"
done

# Collect object files
OBJECT_FILES=()
for src_file in "${SRC_FILES[@]}"; do
    obj_file=$(echo "$src_file" | sed "s|src/|$BUILD_DIR/|" | sed 's|\.cpp|\.o|')
    OBJECT_FILES+=("$obj_file")
done

# Link test executable
echo "🔗 Linking test executable..."
$CC $CXXFLAGS_TEST -o "$TEST_EXECUTABLE" "${TEST_SRC_FILES[@]}" "${OBJECT_FILES[@]}" $TEST_LIBS

echo "✅ Test executable built successfully: $TEST_EXECUTABLE"
