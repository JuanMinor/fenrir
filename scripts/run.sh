#!/bin/bash

# -----------------------------------------------------------------------------
# Fenrir Chess Engine Executor Script
# -----------------------------------------------------------------------------
# This script automates the process of building and running the Fenrir chess engine.
# It performs the following steps:
#   1. Cleans and builds the project using the Makefile.
#   2. Runs the Fenrir chess engine with the default FEN string.
# -----------------------------------------------------------------------------

# Exit immediately if a command exits with a non-zero status
set -e

# Get the directory of this script
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PROJECT_ROOT=$(cd "$SCRIPT_DIR/.." && pwd)

# Function to clean and build the project
build_project() {
	echo "Cleaning and building the project..."
	(cd "$PROJECT_ROOT" && make clean release)
}

# Function to run the Fenrir engine with the default FEN string
run_engine() {
	echo "Running the Fenrir engine..."
	(cd "$PROJECT_ROOT" && g++ -O2 -DNDEBUG -I . -fPIC -DFENRIR_BUILD_DLL -o bin/mainh main.cpp -Lbin/lib -lfenrir)
	(cd "$PROJECT_ROOT" && LD_LIBRARY_PATH=bin/lib ./bin/mainh)
}

# Main execution
build_project
run_engine
