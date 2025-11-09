# ==============================================================================
# Fenrir Chess Engine - Makefile
# ==============================================================================
# This Makefile builds the Fenrir chess engine shared library and runs tests.
#
# Main targets:
#   make		  - Build debug version of shared library
#   make release  - Build optimized release version
#   make test	 - Run unit tests (debug mode only)
#   make coverage - Generate test coverage report
#   make clean	- Clean all build artifacts
#   make help	 - Show detailed help
# ==============================================================================

# ------------------------------------------------------------------------------
# Build Configuration
# ------------------------------------------------------------------------------

# Compiler and basic flags
CC = g++
CC_VERSION = c++20
PROJECT_ROOT = /workspaces/fenrir

# Common compiler flags for all builds
COMMON_FLAGS = -std=$(CC_VERSION) -I $(PROJECT_ROOT) -fPIC -DFENRIR_BUILD_DLL

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
DEBUG_DIR = .debug

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
	@scripts/build/build_mode.sh debug

# Explicit release build
release:
	@scripts/build/build_mode.sh release

# ------------------------------------------------------------------------------
# Library Build Rules
# ------------------------------------------------------------------------------

# Build the shared library from object files
$(SHARED_LIB): $(OBJECT_FILES)
	@echo "🔗 Linking shared library: $@"
	@mkdir -p $(LIB_DIR)
	$(CC) -shared -o $@ $(OBJECT_FILES)
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
	@scripts/build/build_tests.sh

# Generate test coverage report
coverage: $(TEST_EXECUTABLE) | check-debug-mode
	@scripts/build/generate_coverage.sh



# ------------------------------------------------------------------------------
# Utility Targets
# ------------------------------------------------------------------------------

# Clean all build artifacts and generated files
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) $(LIB_DIR) bin
	rm -rf $(TEST_BUILD_DIR) $(COVERAGE_DIR) $(DEBUG_DIR) bint
	rm -rf $(LOG_FILES) $(PGN_FILES)
	rm -f .build_mode
	find . -name "*.gcov" -type f -delete 2>/dev/null || true
	find . -name "*.gcda" -type f -delete 2>/dev/null || true
	find . -name "*.gcno" -type f -delete 2>/dev/null || true
	@echo "✅ Clean complete"

# Display detailed help information
help:
	@echo ""
	@echo "🐺 Fenrir Chess Engine - Build System"
	@echo "======================================"
	@echo ""
	@echo "📋 Available targets:"
	@echo "  all			  - Build shared library (default: debug mode)"
	@echo "  debug			- Build in debug mode (symbols + assertions)"
	@echo "  release		  - Build in release mode (optimized + stripped)"
	@echo "  test			 - Run unit tests (debug mode only)"
	@echo "  coverage		 - Generate test coverage report (debug mode only)"
	@echo "  clean			- Remove all build artifacts"
	@echo "  help			 - Show this help message"
	@echo ""
	@echo "🔧 Build modes:"
	@echo "  Debug   : -g (debug symbols), assertions enabled"
	@echo "  Release : -O2 (optimized), -DNDEBUG (assertions disabled)"
	@echo ""
	@echo "📁 Output locations:"
	@echo "  Library	 : $(SHARED_LIB)"
	@echo "  Tests	   : $(TEST_EXECUTABLE)"
	@echo "  Debug		 : $(DEBUG_DIR)"
	@echo "  Coverage	: $(COVERAGE_REPORT)/index.html"
	@echo ""
	@echo "⚠️  Coverage Policy: 100% test coverage is REQUIRED!"
	@echo "	 - 'make coverage' will fail if coverage < 100%"
	@echo "	 - Coverage table shows exact uncovered line numbers"
	@echo "	 - This enforces code quality as the project grows"
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
	@echo "   make test			  (uses debug mode by default)"
	@echo "   make coverage		  (uses debug mode by default)"
	@echo "   make BUILD_MODE=debug test"
	@echo ""
	@exit 1
endif

# ------------------------------------------------------------------------------
# Make Configuration
# ------------------------------------------------------------------------------

# Declare phony targets (targets that don't create files)
.PHONY: all debug release test coverage clean help check-debug-mode

# Don't delete intermediate files
.PRECIOUS: $(OBJECT_FILES)
