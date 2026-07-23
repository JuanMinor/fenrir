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
#include "include/selfplay/selfplay.h"
#include <filesystem>

using namespace chess;

TEST(SelfPlayTest, RunGameDefault) {
    SelfPlay sp(0, 1, 1);
    sp.run();
    EXPECT_TRUE(true);
}

TEST(SelfPlayTest, RunGameWhiteMated) {
    SelfPlay sp(0, 1, 1, "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
    sp.run();
    EXPECT_TRUE(true);
}

TEST(SelfPlayTest, RunGameBlackMated) {
    SelfPlay sp(0, 1, 1, "rnbqkbnr/ppppp2p/5p2/6pQ/4P3/8/PPPP1PPP/RNB1KBNR b KQkq - 1 3");
    sp.run();
    EXPECT_TRUE(true);
}
