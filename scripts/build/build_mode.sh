#!/bin/bash

# -----------------------------------------------------------------------------
# Fenrir Chess Engine Build Mode Manager
# -----------------------------------------------------------------------------
# This script manages build mode switching between debug and release modes.
# It performs the following operations:
#   1. Checks current build mode from .build_mode file
#   2. Cleans object files if mode changed
#   3. Updates build mode file
#   4. Calls appropriate build target
# -----------------------------------------------------------------------------

set -e

BUILD_MODE="$1"
BUILD_DIR="bin/build"

if [ -z "$BUILD_MODE" ]; then
    echo "Usage: $0 <debug|release>"
    exit 1
fi

case "$BUILD_MODE" in
    debug)
        echo "🔧 Building Fenrir in DEBUG mode..."
        ;;
    release)
        echo "🚀 Building Fenrir in RELEASE mode..."
        ;;
    *)
        echo "❌ ERROR: Invalid build mode '$BUILD_MODE'. Use 'debug' or 'release'."
        exit 1
        ;;
esac

# Check if build mode changed
current_mode=""
if [ -f ".build_mode" ]; then
    current_mode=$(cat .build_mode)
fi

if [ "$current_mode" != "$BUILD_MODE" ]; then
    if [ -n "$current_mode" ]; then
        echo "🔄 Build mode changed from $current_mode to $BUILD_MODE - cleaning object files..."
        rm -rf "$BUILD_DIR"/*.o "$BUILD_DIR"/*/*.o 2>/dev/null || true
    fi
    echo "$BUILD_MODE" > .build_mode
fi

# Build with the specified mode
make --no-print-directory "BUILD_MODE=$BUILD_MODE" all
