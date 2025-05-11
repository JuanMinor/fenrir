# Compiler settings
CC = g++
COVERAGE_FLAGS = -fprofile-arcs -ftest-coverage
COMMON_FLAGS = -g -I /workspaces/loki -fPIC -DLOKI_BUILD_DLL
CXXFLAGS = $(COMMON_FLAGS)
CXXFLAGST = $(COMMON_FLAGS) $(COVERAGE_FLAGS)

# Paths
BIN = bin
BIN_TEST = bint
BUILD = $(BIN)/build
LIB = $(BIN)/lib
LOGS = logs/loki.log
PGN_FILES = pgn/loki.pgn pgn/loki.store.txt

# Coverage settings
COVERAGE_DIR = .coverage
COVERAGE_INFO = $(COVERAGE_DIR)/coverage.info
COVERAGE_REPORT = $(COVERAGE_DIR)/report

# Source and object files
SRC_FILES = src/chess/board.cpp \
			src/chrono/chrono.cpp \
			src/engine/engine.cpp \
			src/chess/fen.cpp \
			src/logger/logger.cpp \
			src/modifier/modifier.cpp \
			src/pgn/pgn.cpp \
			src/chess/piece.cpp
OBJECT_FILES = $(SRC_FILES:src/%.cpp=$(BUILD)/%.o)

# Shared library
SHARED_LIB = $(LIB)/libloki.so

# Test settings
UNIT_TEST_BIN_DIR = $(BIN_TEST)/unit
UNIT_TEST_SRC = tests/unit/fen.test.cpp \
				tests/unit/board.test.cpp \
				tests/unit/piece.test.cpp \
				tests/unit/engine.test.cpp
TEST_LIBS = -lgtest -lgtest_main -lpthread

# Default target
all: $(SHARED_LIB)

# Build shared library
$(SHARED_LIB): $(OBJECT_FILES)
	@mkdir -p $(LIB)
	$(CC) -shared -o $@ $^

# Pattern rule for object files
$(BUILD)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) -c $(CXXFLAGS) -o $@ $<

# Test target
test: $(UNIT_TEST_SRC) $(OBJECT_FILES)
	@mkdir -p $(UNIT_TEST_BIN_DIR)
	$(CC) $(CXXFLAGST) -o $(UNIT_TEST_BIN_DIR)/tests $(UNIT_TEST_SRC) $(OBJECT_FILES) $(TEST_LIBS)
	$(UNIT_TEST_BIN_DIR)/tests

# Coverage target
coverage: test
	@mkdir -p $(COVERAGE_DIR)
	lcov --capture --directory . --output-file $(COVERAGE_INFO)
	genhtml $(COVERAGE_INFO) --output-directory $(COVERAGE_REPORT)
	@echo "Coverage report generated in $(COVERAGE_REPORT)"
	@echo "Per-file coverage breakdown:"
	@lcov --list $(COVERAGE_INFO) | awk '/^\/workspaces\/loki/ {printf "%-60s %s\n", $$1, $$2}'
	@echo ""
	@echo "Uncovered lines per file:"
	@lcov --list $(COVERAGE_INFO) | awk 'BEGIN{file=""} \
		/^\/workspaces\/loki/ {file=$$1} \
		/^[ ]*[0-9]+/ && $$2=="#####:" && file!="" {printf "%s: line %s is not covered\n", file, $$1}'
	@total_cov=$$(lcov --summary $(COVERAGE_INFO) | grep 'lines\.*:' | awk '{print $$2}' | tr -d '%'); \
	if [ "$$total_cov" != "100.0" ]; then \
		echo "ERROR: Coverage is not 100% ($$total_cov%)"; \
		exit 1; \
	fi

# Clean target
clean:
	rm -rf $(BUILD) $(BIN) $(LOGS) $(PGN_FILES)
	rm -rf $(UNIT_TEST_BIN_DIR) $(OBJECT_FILES) $(COVERAGE_DIR) $(BIN_TEST)

# Phony targets
.PHONY: clean test coverage
