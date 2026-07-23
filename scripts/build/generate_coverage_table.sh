#!/bin/bash

#   Copyright (c) 2026 Juan Minor
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.

# -----------------------------------------------------------------------------
# Fenrir Chess Engine Coverage Table Generator
# -----------------------------------------------------------------------------
# This script generates a detailed per-file coverage table showing covered lines,
# uncovered lines, and coverage percentages for each source file.
# It performs the following operations:
#   1. Parses lcov output to extract per-file coverage data
#   2. Generates gcov files for detailed line-by-line analysis
#   3. Identifies specific uncovered line numbers
#   4. Formats results in a clean ASCII table
# -----------------------------------------------------------------------------

set -e

COVERAGE_INFO="$1"
DEBUG_DIR="$2"

if [ -z "$COVERAGE_INFO" ] || [ -z "$DEBUG_DIR" ]; then
    echo "Usage: $0 <coverage_info_file> <debug_dir>"
    exit 1
fi

echo "📊 Per-File Coverage Table:"
echo "┌─────────────────────────────────────────────────┬─────────┬─────────────────────────────────────┬─────────┐"
echo "│ File                                            │ Covered │ Uncovered Lines                     │ Percent │"
echo "├─────────────────────────────────────────────────┼─────────┼─────────────────────────────────────┼─────────┤"

lcov --list "$COVERAGE_INFO" 2>/dev/null | grep -E "\.cpp.*\|.*%" | grep -v "test\.cpp" | while read line; do
    file=$(echo "$line" | awk '{print $1}')
    lines_data=$(echo "$line" | awk -F'|' '{print $2}' | awk '{print $1, $2}')
    percent=$(echo "$lines_data" | awk '{print $1}')
    total=$(echo "$lines_data" | awk '{print $2}')
    
    if [ -n "$percent" ] && [ -n "$total" ] && [ "$total" != "0" ]; then
        percent_num=$(echo "$percent" | sed 's/%//')
        covered=$(echo "$percent_num $total" | awk '{printf "%.0f", $2 * $1 / 100}')
        
        if [ "$percent" = "100%" ]; then
            uncovered_display="None (100% covered!)"
        else
            # Use existing gcov file from debug directory (don't regenerate)
            gcov_file="$DEBUG_DIR/$(basename "$file").gcov"
            
            if [ -f "$gcov_file" ]; then
                uncovered_lines=$(grep -E "(####:)" "$gcov_file" | cut -d: -f2 | tr '\n' ',' | sed 's/,$//')
                if [ -n "$uncovered_lines" ]; then
                    uncovered_display="$uncovered_lines"
                else
                    uncovered_display="Unable to detect lines"
                fi
            else
                uncovered_display="Gcov data unavailable"
            fi
        fi
    else
        covered="-"
        uncovered_display="-"
    fi
    
    printf "│ %-47s │ %7s │ %-35s │ %7s │\n" "$file" "$covered" "$uncovered_display" "$percent"
done || echo "│ No source file coverage data found             │    -    │                 -                   │    -    │"

echo "└─────────────────────────────────────────────────┴─────────┴─────────────────────────────────────┴─────────┘"

# Clean up any remaining gcov files
find . -maxdepth 1 -name "*.gcov" -type f -delete 2>/dev/null || true
