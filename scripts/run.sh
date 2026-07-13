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

set -e

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PROJECT_ROOT=$(cd "$SCRIPT_DIR/.." && pwd)

build_project() {
    echo "Cleaning and building the project..."
    (cd "$PROJECT_ROOT" && rm -rf build && cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build)
}

run_engine() {
    echo "Running the Fenrir engine..."
    (cd "$PROJECT_ROOT" && ./bin/fenrir)
}

build_project
run_engine
