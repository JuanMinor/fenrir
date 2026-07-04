#!/bin/bash

# -----------------------------------------------------------------------------
# Fenrir Chess Engine Test Runner Script
# -----------------------------------------------------------------------------
# This script automates the process of running the test suite for the Fenrir chess engine.
# It performs the following steps:
#   1. Navigates to the project root directory.
#   2. Executes the `make test` command to run the unit tests.
# -----------------------------------------------------------------------------

# Exit immediately if a command exits with a non-zero status
set -e

# Get the directory of this script
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PROJECT_ROOT=$(cd "$SCRIPT_DIR/.." && pwd)

# Function to run tests
run_tests() {
	(cd "$PROJECT_ROOT" && \
	 rm -rf build && \
	 cmake -B build -DCMAKE_BUILD_TYPE=Debug && \
	 cmake --build build && \
	 cmake --build build --target coverage)
}

# Main execution
run_tests
