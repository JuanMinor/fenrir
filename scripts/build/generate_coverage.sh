#!/bin/bash

# -----------------------------------------------------------------------------
# Fenrir Chess Engine Coverage Report Generator
# -----------------------------------------------------------------------------
# This script generates a comprehensive test coverage report with detailed
# per-file analysis and enforces 100% coverage policy.
# It performs the following operations:
#   1. Runs lcov to capture coverage data
#   2. Filters to include only project source files
#   3. Cleans unreachable code markers using AWK script
#   4. Generates HTML report
#   5. Creates detailed coverage table
#   6. Enforces 100% coverage requirement
# -----------------------------------------------------------------------------

set -e

# Configuration
COVERAGE_DIR=".coverage"
COVERAGE_INFO="$COVERAGE_DIR/coverage.info"
COVERAGE_REPORT="$COVERAGE_DIR/report"
DEBUG_DIR=".debug"

# Create directories
mkdir -p "$COVERAGE_DIR" "$DEBUG_DIR"

echo "📊 Generating coverage report..."

# Run tests first
mkdir -p onnx
cp tests/dummy.onnx onnx/fenrir.onnx

echo "🧪 Running tests..."
bint/unit/tests
# Run perft with split FEN to cover FEN reconstruction logic and depth > 1 to cover loop body
bin/perft 2 rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
# Run perft without arguments to cover usage logic
bin/perft || true

# Generate coverage data
# Configuration is loaded from .lcovrc (disables checksums for gcov 15.x compatibility)
# --ignore-errors: gcov 15.x compatibility issues with lcov 2.x:
#   - inconsistent: Format/version mismatches between gcov and lcov
#   - mismatch: Line number discrepancies in intermediate format
#   - negative: Invalid hit counts in system headers
#   - count: Hit count calculation issues in C++15.x STL
# This approach combines:
#   1. Config file (.lcovrc) for reusable settings (checksum=0)
#   2. Targeted error suppression for gcov 15.x specific format issues
# These errors occur in system headers, not project code, so suppression is safe.
lcov --config-file .lcovrc --capture --directory . --output-file "$COVERAGE_INFO" --ignore-errors inconsistent,mismatch,negative,count 2>/dev/null

# Filter to only include project source files
lcov --extract "$COVERAGE_INFO" '*/src/*' --output-file "$COVERAGE_INFO.filtered" 2>/dev/null
mv "$COVERAGE_INFO.filtered" "$COVERAGE_INFO"

# Generate gcov files for unreachable line analysis
echo "🔧 Generating gcov files for unreachable code analysis..."
for src_file in src/*/*.cpp; do
    if [ -f "$src_file" ]; then
        gcno_file=$(find . -name "$(basename "$src_file").gcno" 2>/dev/null | head -n 1)
        if [ -n "$gcno_file" ]; then
            gcov -o "$gcno_file" "$src_file" >/dev/null 2>&1 || true
        else
            legacy_dir="bin/build/$(dirname "$src_file" | sed 's|src/||')"
            if [ -d "$legacy_dir" ]; then
                gcov -o "$legacy_dir" "$src_file" >/dev/null 2>&1 || true
            fi
        fi
    fi
done

# Move only our source file gcov files to debug directory for AWK script
for gcov_file in *.gcov; do
    if [ -f "$gcov_file" ]; then
        # Check if this is one of our project source files (ends with .cpp.gcov)
        if [[ "$gcov_file" == *.cpp.gcov ]]; then
            # This is one of our source files, move it to debug directory
            mv "$gcov_file" "$DEBUG_DIR/"
        else
            # Remove STL/system header gcov files
            rm -f "$gcov_file"
        fi
    fi
done 2>/dev/null || true

# Remove truly unreachable line entries (===== in gcov) and adjust totals dynamically
echo "🔧 Cleaning truly unreachable code markers from coverage data..."
awk -f scripts/build/clean_coverage.awk "$COVERAGE_INFO" > "$COVERAGE_INFO.tmp" && mv "$COVERAGE_INFO.tmp" "$COVERAGE_INFO"

# Generate HTML report
genhtml "$COVERAGE_INFO" --output-directory "$COVERAGE_REPORT" 2>/dev/null

echo ""
echo "📈 Coverage report generated in $COVERAGE_REPORT"
echo ""

# Generate per-file coverage table
scripts/build/generate_coverage_table.sh "$COVERAGE_INFO" "$DEBUG_DIR"

# Check for 100% coverage
total_cov=$(lcov --summary "$COVERAGE_INFO" 2>/dev/null | grep 'lines.*:' | awk '{print $2}' | tr -d '%')
if [ "$total_cov" != "100.0" ]; then
    echo ""
    echo "❌ ERROR: Coverage is not 100% ($total_cov%)"
    echo "   Code changes cannot be committed without 100% test coverage!"
    echo ""
    echo "📈 Uncovered lines analysis:"
    echo "Use: 'lcov --list $COVERAGE_INFO' to see detailed per-file coverage"
    echo "Use: HTML report at $COVERAGE_REPORT/index.html for detailed analysis"
    echo ""
    echo "🔧 Next steps:"
    echo "   1. Check the table above for exact uncovered lines"
    echo "   2. Write tests for the missing lines"
    echo "   3. Run 'make coverage' again until 100% is achieved"
    echo ""
    echo "⚠️  This is enforced to prevent quality issues as the project grows."
    echo "📁 .gcov files available in $DEBUG_DIR/ for line-by-line debugging"
    exit 1
else
    echo ""
    echo "🎉 Perfect! 100% test coverage achieved!"
    echo "✅ Code is ready for commit."
fi
