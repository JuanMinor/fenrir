/*
 *   Copyright (c) 2026 Juan Minor
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include "include/mcts/mcts.h"
#include "include/engine/engine.h"

using namespace chess;

/**
 * An absurdly high thread count exceeds OS thread limits, forcing the
 * synchronous fallback path.
 */
TEST(MCTSTestFallback, FallbackToSynchronous)
{
    Engine engine;
    mcts::Tree search(nullptr, 100000, 8);
    search.find_best_move_with_policy(engine, 10, false);
}
