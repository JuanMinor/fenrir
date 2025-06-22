# ==============================================================================
# Fenrir Chess Engine - Makefile
# ==============================================================================
# This Makefile builds the Fenrir chess engine shared library and runs tests.
# 
# Main targets:
#   make          - Build debug version of shared library
#   make release  - Build optimized release version  
#   make test     - Run unit tests (debug mode only)
#   make coverage - Generate test coverage report
#   make clean    - Clean all build artifacts
#   make help     - Show detailed help
# ==============================================================================

# ------------------------------------------------------------------------------
# Build Configuration
# ------------------------------------------------------------------------------

# Compiler and basic flags
CC = g++
PROJECT_ROOT = /workspaces/fenrir

# Common compiler flags for all builds
COMMON_FLAGS = -I $(PROJECT_ROOT) -fPIC -DFENRIR_BUILD_DLL

# Build mode selection (default: debug)
BUILD_MODE ?= debug

# Mode-specific flags
ifeq ($(BUILD_MODE),release)
    # Release: optimized, no debug symbols, assertions disabled
    CXXFLAGS = $(COMMON_FLAGS) -O2 -DNDEBUG
else
    # Debug: debug symbols, no optimization, assertions enabled
    CXXFLAGS = $(COMMON_FLAGS) -g
endif

# Test-specific flags (adds coverage instrumentation)
COVERAGE_FLAGS = -fprofile-arcs -ftest-coverage
CXXFLAGS_TEST = $(CXXFLAGS) $(COVERAGE_FLAGS)

# ------------------------------------------------------------------------------
# Directory Structure
# ------------------------------------------------------------------------------

# Build directories
BUILD_DIR = bin/build
LIB_DIR = bin/lib
TEST_BUILD_DIR = bint/unit

# Coverage analysis directories  
COVERAGE_DIR = .coverage
COVERAGE_INFO = $(COVERAGE_DIR)/coverage.info
COVERAGE_REPORT = $(COVERAGE_DIR)/report

# Runtime files that get generated
LOG_FILES = logs/fenrir.log
PGN_FILES = pgn/fenrir.pgn pgn/fenrir.store.txt

# ------------------------------------------------------------------------------
# Source Files and Targets
# ------------------------------------------------------------------------------

# All source files for the main library
SRC_FILES = src/chess/board.cpp \
           src/chrono/chrono.cpp \
           src/engine/engine.cpp \
           src/chess/fen.cpp \
           src/logger/logger.cpp \
           src/modifier/modifier.cpp \
           src/chess/moves.cpp \
           src/pgn/pgn.cpp \
           src/chess/piece.cpp \
           src/utils/utils.cpp

# Convert source files to object file paths
OBJECT_FILES = $(SRC_FILES:src/%.cpp=$(BUILD_DIR)/%.o)

# Main shared library target
SHARED_LIB = $(LIB_DIR)/libfenrir.so

# Test source files
TEST_SRC_FILES = tests/unit/fen.test.cpp \
                tests/unit/board.test.cpp \
                tests/unit/piece.test.cpp \
                tests/unit/engine.test.cpp \
                tests/unit/utils.test.cpp \
                tests/unit/moves.test.cpp \
                tests/unit/logger.test.cpp \
                tests/unit/chrono.test.cpp \
                tests/unit/modifier.test.cpp \
                tests/unit/pgn.test.cpp

# Test executable
TEST_EXECUTABLE = $(TEST_BUILD_DIR)/tests

# Test libraries
TEST_LIBS = -lgtest -lgtest_main -lpthread

# ==============================================================================
# Build Targets
# ==============================================================================

# ------------------------------------------------------------------------------
# Default and Main Targets
# ------------------------------------------------------------------------------

# Default target: build debug version of shared library
all: $(SHARED_LIB)

# Explicit debug build
debug:
	@echo "🔧 Building Fenrir in DEBUG mode..."
	@current_mode="$$(cat .build_mode 2>/dev/null || echo '')"; \
	if [ "$$current_mode" != "debug" ]; then \
		if [ -n "$$current_mode" ]; then \
			echo "🔄 Build mode changed from $$current_mode to debug - cleaning object files..."; \
			rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/*/*.o 2>/dev/null || true; \
		fi; \
		echo "debug" > .build_mode; \
	fi
	@$(MAKE) --no-print-directory BUILD_MODE=debug all

# Explicit release build  
release:
	@echo "🚀 Building Fenrir in RELEASE mode..."
	@current_mode="$$(cat .build_mode 2>/dev/null || echo '')"; \
	if [ "$$current_mode" != "release" ]; then \
		if [ -n "$$current_mode" ]; then \
			echo "🔄 Build mode changed from $$current_mode to release - cleaning object files..."; \
			rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/*/*.o 2>/dev/null || true; \
		fi; \
		echo "release" > .build_mode; \
	fi
	@$(MAKE) --no-print-directory BUILD_MODE=release all

# ------------------------------------------------------------------------------
# Library Build Rules
# ------------------------------------------------------------------------------

# Build the shared library from object files
$(SHARED_LIB): $(OBJECT_FILES)
	@echo "🔗 Linking shared library: $@"
	@mkdir -p $(LIB_DIR)
	$(CC) -shared -o $@ $^
	@echo "✅ Shared library built successfully"

# Pattern rule: build object files from source files
$(BUILD_DIR)/%.o: src/%.cpp
	@echo "🔨 Compiling: $< -> $@"
	@mkdir -p $(dir $@)
	$(CC) -c $(CXXFLAGS) -o $@ $<

# ------------------------------------------------------------------------------
# Testing Targets
# ------------------------------------------------------------------------------

# Run unit tests (debug mode only for reliability)
test: $(TEST_EXECUTABLE)
	@echo "🧪 Running unit tests..."
	$(TEST_EXECUTABLE)

# Build test executable
$(TEST_EXECUTABLE): $(TEST_SRC_FILES) | check-debug-mode
	@echo "🔨 Building test executable with coverage..."
	@mkdir -p $(TEST_BUILD_DIR)
	
	# Rebuild object files with coverage flags for testing
	@echo "🔄 Rebuilding source files with coverage instrumentation..."
	@for src_file in $(SRC_FILES); do \
		obj_file=$$(echo $$src_file | sed 's|src/|$(BUILD_DIR)/|' | sed 's|\.cpp|\.o|'); \
		echo "🔨 Compiling with coverage: $$src_file -> $$obj_file"; \
		mkdir -p $$(dirname $$obj_file); \
		$(CC) -c $(CXXFLAGS_TEST) -o $$obj_file $$src_file; \
	done
	
	# Link test executable
	$(CC) $(CXXFLAGS_TEST) -o $@ $(TEST_SRC_FILES) $(OBJECT_FILES) $(TEST_LIBS)

# Generate test coverage report
coverage: $(TEST_EXECUTABLE) | check-debug-mode
	@echo "📊 Generating coverage report..."
	@mkdir -p $(COVERAGE_DIR)
	
	# Run tests first
	$(TEST_EXECUTABLE)
	
	# Generate coverage data
	lcov --capture --directory . --output-file $(COVERAGE_INFO)
	
	# Filter to only include project source files
	lcov --extract $(COVERAGE_INFO) '*/src/*' --output-file $(COVERAGE_INFO).filtered
	mv $(COVERAGE_INFO).filtered $(COVERAGE_INFO)
	
	genhtml $(COVERAGE_INFO) --output-directory $(COVERAGE_REPORT)
	
	@echo ""
	@echo "📈 Coverage report generated in $(COVERAGE_REPORT)"
	@echo ""
	@echo "📊 Per-File Coverage Table:"
	@echo "┌─────────────────────────────────────────────────┬─────────┬─────────────────────────────────────┬─────────┐"
	@echo "│ File                                            │ Covered │ Uncovered Lines                     │ Percent │"
	@echo "├─────────────────────────────────────────────────┼─────────┼─────────────────────────────────────┼─────────┤"
	@lcov --list $(COVERAGE_INFO) 2>/dev/null | grep -E "\.cpp.*\|.*%" | grep -v "test\.cpp" | while read line; do \
		file=$$(echo "$$line" | awk '{print $$1}'); \
		lines_data=$$(echo "$$line" | awk -F'|' '{print $$2}' | awk '{print $$1, $$2}'); \
		percent=$$(echo "$$lines_data" | awk '{print $$1}'); \
		total=$$(echo "$$lines_data" | awk '{print $$2}'); \
		if [ -n "$$percent" ] && [ -n "$$total" ] && [ "$$total" != "0" ]; then \
			percent_num=$$(echo "$$percent" | sed 's/%//'); \
			covered=$$(echo "$$percent_num $$total" | awk '{printf "%.0f", $$2 * $$1 / 100}'); \
			if [ "$$percent" = "100%" ]; then \
				uncovered_display="None (100% covered!)"; \
			else \
				src_file="src/$$(echo $$file | sed 's/\.cpp//')/$$(echo $$file)"; \
				if [ ! -f "$$src_file" ]; then src_file="src/chess/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/chrono/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/engine/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/logger/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/modifier/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/pgn/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/utils/$$file"; fi; \
				if [ -f "$$src_file" ]; then \
					obj_dir="bin/build/$$(dirname $$src_file | sed 's|src/||')"; \
					gcov -o "$$obj_dir" "$$src_file" >/dev/null 2>&1; \
					gcov_file="$$(basename $$file).gcov"; \
					if [ -f "$$gcov_file" ]; then \
						uncovered_lines=$$(grep "####:" "$$gcov_file" | cut -d: -f2 | tr '\n' ',' | sed 's/,$$//'); \
						if [ -n "$$uncovered_lines" ]; then \
							uncovered_display="$$uncovered_lines"; \
						else \
							uncovered_display="Unable to detect lines"; \
						fi; \
						rm -f "$$gcov_file"; \
					else \
						uncovered_display="Gcov data unavailable"; \
					fi; \
				else \
					uncovered_display="Source file not found"; \
				fi; \
			fi; \
		else \
			covered="-"; uncovered_display="-"; \
		fi; \
		printf "│ %-47s │ %7s │ %-35s │ %7s │\n" "$$file" "$$covered" "$$uncovered_display" "$$percent"; \
	done || echo "│ No source file coverage data found             │    -    │                 -                   │    -    │"
	@echo "└─────────────────────────────────────────────────┴─────────┴─────────────────────────────────────┴─────────┘"
	
	@echo ""
	@echo "❌ Uncovered lines analysis:"
	@echo "Use: 'lcov --list $(COVERAGE_INFO)' to see detailed per-file coverage"
	@echo "Use: 'make coverage-details' for detailed line-by-line analysis"
	
	# Check for 100% coverage
	@total_cov=$$(lcov --summary $(COVERAGE_INFO) | grep 'lines\.*:' | awk '{print $$2}' | tr -d '%'); \
	if [ "$$total_cov" != "100.0" ]; then \
		echo ""; \
		echo "❌ ERROR: Coverage is not 100% ($$total_cov%)"; \
		echo "   Code changes cannot be committed without 100% test coverage!"; \
		echo ""; \
		echo "🔧 Next steps:"; \
		echo "   1. Run 'make coverage-details' to see exact uncovered lines"; \
		echo "   2. Write tests for the missing lines"; \
		echo "   3. Run 'make coverage' again until 100% is achieved"; \
		echo ""; \
		echo "⚠️  This is enforced to prevent quality issues as the project grows."; \
		find . -name "*.gcov" -type f -delete 2>/dev/null || true; \
		echo "🧹 Cleaned up .gcov files"; \
		exit 1; \
	else \
		echo ""; \
		echo "🎉 Perfect! 100% test coverage achieved!"; \
		echo "✅ Code is ready for commit."; \
		find . -name "*.gcov" -type f -delete 2>/dev/null || true; \
		echo "🧹 Cleaned up .gcov files"; \
	fi

# Detailed coverage analysis - shows exactly which lines are uncovered
coverage-details: $(COVERAGE_INFO)
	@echo "🔍 Detailed Coverage Analysis"
	@echo "=============================="
	@echo ""
	@echo "📊 Per-File Coverage Table with Uncovered Lines:"
	@echo "┌─────────────────────────────────────────────────┬─────────┬─────────────────────────────────────┬─────────┐"
	@echo "│ File                                            │ Covered │ Uncovered Lines                     │ Percent │"
	@echo "├─────────────────────────────────────────────────┼─────────┼─────────────────────────────────────┼─────────┤"
	@lcov --list $(COVERAGE_INFO) 2>/dev/null | grep -E "\.cpp.*\|.*%" | grep -v "test\.cpp" | while read line; do \
		file=$$(echo "$$line" | awk '{print $$1}'); \
		lines_data=$$(echo "$$line" | awk -F'|' '{print $$2}' | awk '{print $$1, $$2}'); \
		percent=$$(echo "$$lines_data" | awk '{print $$1}'); \
		total=$$(echo "$$lines_data" | awk '{print $$2}'); \
		if [ -n "$$percent" ] && [ -n "$$total" ] && [ "$$total" != "0" ]; then \
			percent_num=$$(echo "$$percent" | sed 's/%//'); \
			covered=$$(echo "$$percent_num $$total" | awk '{printf "%.0f", $$2 * $$1 / 100}'); \
			if [ "$$percent" = "100%" ]; then \
				uncovered_display="None (100% covered!)"; \
			else \
				src_file="src/$$(echo $$file | sed 's/\.cpp//')/$$(echo $$file)"; \
				if [ ! -f "$$src_file" ]; then src_file="src/chess/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/chrono/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/engine/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/logger/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/modifier/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/pgn/$$file"; fi; \
				if [ ! -f "$$src_file" ]; then src_file="src/utils/$$file"; fi; \
				if [ -f "$$src_file" ]; then \
					obj_dir="bin/build/$$(dirname $$src_file | sed 's|src/||')"; \
					gcov -o "$$obj_dir" "$$src_file" >/dev/null 2>&1; \
					gcov_file="$$(basename $$file).gcov"; \
					if [ -f "$$gcov_file" ]; then \
						uncovered_lines=$$(grep "####:" "$$gcov_file" | cut -d: -f2 | tr '\n' ',' | sed 's/,$$//'); \
						if [ -n "$$uncovered_lines" ]; then \
							uncovered_display="$$uncovered_lines"; \
						else \
							uncovered_display="Unable to detect lines"; \
						fi; \
						rm -f "$$gcov_file"; \
					else \
						uncovered_display="Gcov data unavailable"; \
					fi; \
				else \
					uncovered_display="Source file not found"; \
				fi; \
			fi; \
		else \
			covered="-"; uncovered_display="-"; \
		fi; \
		printf "│ %-47s │ %7s │ %-35s │ %7s │\n" "$$file" "$$covered" "$$uncovered_display" "$$percent"; \
	done || echo "│ No source file coverage data found             │    -    │                 -                   │    -    │"
	@echo "└─────────────────────────────────────────────────┴─────────┴─────────────────────────────────────┴─────────┘"
	@echo ""
	@echo "🎯 Uncovered Lines Details:"
	@echo "=========================="
	@lcov --list $(COVERAGE_INFO) 2>/dev/null | grep -E "\.cpp.*\|.*%" | grep -v "test\.cpp" | while read line; do \
		file=$$(echo "$$line" | awk '{print $$1}'); \
		lines_data=$$(echo "$$line" | awk -F'|' '{print $$2}' | awk '{print $$1, $$2}'); \
		percent=$$(echo "$$lines_data" | awk '{print $$1}'); \
		if [ "$$percent" != "100%" ]; then \
			echo ""; \
			echo "🔸 $$file ($$percent coverage):"; \
			src_file="src/$$(echo $$file | sed 's/\.cpp//')/$$(echo $$file)"; \
			if [ ! -f "$$src_file" ]; then \
				src_file="src/chess/$$file"; \
			fi; \
			if [ ! -f "$$src_file" ]; then \
				src_file="src/chrono/$$file"; \
			fi; \
			if [ ! -f "$$src_file" ]; then \
				src_file="src/engine/$$file"; \
			fi; \
			if [ ! -f "$$src_file" ]; then \
				src_file="src/logger/$$file"; \
			fi; \
			if [ ! -f "$$src_file" ]; then \
				src_file="src/modifier/$$file"; \
			fi; \
			if [ ! -f "$$src_file" ]; then \
				src_file="src/pgn/$$file"; \
			fi; \
			if [ ! -f "$$src_file" ]; then \
				src_file="src/utils/$$file"; \
			fi; \
			if [ -f "$$src_file" ]; then \
				obj_dir="bin/build/$$(dirname $$src_file | sed 's|src/||')"; \
				gcov -o "$$obj_dir" "$$src_file" >/dev/null 2>&1; \
				gcov_file="$$(basename $$file).gcov"; \
				if [ -f "$$gcov_file" ]; then \
					uncovered_lines=$$(grep "####:" "$$gcov_file" | cut -d: -f2 | tr '\n' ',' | sed 's/,$$//'); \
					if [ -n "$$uncovered_lines" ]; then \
						echo "   ❌ Lines missing coverage: $$uncovered_lines"; \
						echo "   📝 File: $$src_file"; \
						echo "   🔧 Add tests to execute these lines"; \
					else \
						echo "   ⚠️  Coverage incomplete but no specific lines found"; \
					fi; \
					rm -f "$$gcov_file"; \
				else \
					echo "   ⚠️  Could not generate gcov data for $$src_file"; \
				fi; \
			else \
				echo "   ⚠️  Source file not found: $$file"; \
			fi; \
		fi; \
	done
	@echo ""
	@echo "📈 Overall Summary:"
	@lcov --summary $(COVERAGE_INFO) 2>/dev/null | grep -E "(lines|functions)" | while read line; do \
		echo "   $$line"; \
	done || echo "   No summary available"
	@echo ""
	@echo "🎯 HTML Report: .coverage/report/index.html"
	@echo ""
	@echo "💡 To achieve 100% coverage:"
	@echo "   1. Navigate to each file and the specific line numbers listed above"
	@echo "   2. Write tests that execute those lines (focus on error paths, edge cases)"
	@echo "   3. Run 'make coverage-details' again to verify improvements"
	@echo "   4. Repeat until all files show 100% coverage"
	@find . -name "*.gcov" -type f -delete 2>/dev/null || true
	@echo "🧹 Cleaned up .gcov files"

# ------------------------------------------------------------------------------
# Utility Targets  
# ------------------------------------------------------------------------------

# Clean all build artifacts and generated files
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) $(LIB_DIR) bin
	rm -rf $(TEST_BUILD_DIR) $(COVERAGE_DIR) bint  
	rm -rf $(LOG_FILES) $(PGN_FILES)
	rm -f .build_mode
	find . -name "*.gcov" -type f -delete 2>/dev/null || true
	@echo "✅ Clean complete"

# Display detailed help information
help:
	@echo ""
	@echo "🐺 Fenrir Chess Engine - Build System"
	@echo "======================================"
	@echo ""
	@echo "📋 Available targets:"
	@echo "  all              - Build shared library (default: debug mode)"
	@echo "  debug            - Build in debug mode (symbols + assertions)"
	@echo "  release          - Build in release mode (optimized + stripped)"
	@echo "  test             - Run unit tests (debug mode only)"
	@echo "  coverage         - Generate test coverage report (debug mode only)"
	@echo "  coverage-details - Show exact uncovered line numbers for each file"
	@echo "  clean            - Remove all build artifacts"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "🔧 Build modes:"
	@echo "  Debug   : -g (debug symbols), assertions enabled"
	@echo "  Release : -O2 (optimized), -DNDEBUG (assertions disabled)"
	@echo ""
	@echo "📁 Output locations:"
	@echo "  Library     : $(SHARED_LIB)"
	@echo "  Tests       : $(TEST_EXECUTABLE)" 
	@echo "  Coverage    : $(COVERAGE_REPORT)/index.html"
	@echo ""
	@echo "⚠️  Coverage Policy: 100% test coverage is REQUIRED!"
	@echo "     - 'make coverage' will fail if coverage < 100%"
	@echo "     - Use 'make coverage-details' to see uncovered lines"
	@echo "     - This enforces code quality as the project grows"
	@echo ""

# ------------------------------------------------------------------------------
# Internal Helper Targets
# ------------------------------------------------------------------------------

# Ensure we're in debug mode for testing/coverage
check-debug-mode:
ifeq ($(BUILD_MODE),release)
	@echo ""
	@echo "❌ ERROR: Testing not allowed in release mode!"
	@echo ""
	@echo "   Release builds strip debug information and disable assertions,"
	@echo "   which can lead to unreliable test results and poor debugging."
	@echo ""
	@echo "💡 Solution: Use debug mode for testing:"
	@echo "   make test              (uses debug mode by default)"
	@echo "   make coverage          (uses debug mode by default)"  
	@echo "   make BUILD_MODE=debug test"
	@echo ""
	@exit 1
endif

# ------------------------------------------------------------------------------
# Make Configuration
# ------------------------------------------------------------------------------

# Declare phony targets (targets that don't create files)
.PHONY: all debug release test coverage coverage-details clean help check-debug-mode

# Don't delete intermediate files
.PRECIOUS: $(OBJECT_FILES)
