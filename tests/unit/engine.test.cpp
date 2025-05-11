/*
 *   Copyright (c) 2025 Juan Minor

 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <regex>
#include "include/engine/engine.h"

class EngineTest : public ::testing::Test
{
protected:
    loki::Engine engine;
};

TEST_F(EngineTest, DefaultBoardSetup)
{
    testing::internal::CaptureStdout();
    engine.print_board();
    std::string output = testing::internal::GetCapturedStdout();

    /* Remove ANSI escape sequences using regex */
    std::regex ansi_escape_regex("\033\\[[0-9;]*m");
    std::string stripped_output = std::regex_replace(output, ansi_escape_regex, "");

    EXPECT_NE(stripped_output.find("r n b q k b n r"), std::string::npos);
    EXPECT_NE(stripped_output.find("R N B Q K B N R"), std::string::npos);
}

TEST_F(EngineTest, MakeMove)
{
    engine.make_move(1, 1, 3, 1); // Move white pawn from b2 to b4
    testing::internal::CaptureStdout();
    engine.print_board();
    std::string output = testing::internal::GetCapturedStdout();

    /* Remove ANSI escape sequences using regex */
    std::regex ansi_escape_regex("\033\\[[0-9;]*m");
    std::string stripped_output = std::regex_replace(output, ansi_escape_regex, "");

    EXPECT_NE(stripped_output.find("4 - . P . . . . . . - 4"), std::string::npos);
    EXPECT_NE(stripped_output.find("2 - P . P P P P P P - 2"), std::string::npos);
}

TEST_F(EngineTest, ResetBoard)
{
    engine.make_move(1, 1, 3, 1);
    engine.reset();

    testing::internal::CaptureStdout();
    engine.print_board();
    std::string output = testing::internal::GetCapturedStdout();

    /* Remove ANSI escape sequences using regex */
    std::regex ansi_escape_regex("\033\\[[0-9;]*m");
    std::string stripped_output = std::regex_replace(output, ansi_escape_regex, "");

    ASSERT_NE(stripped_output.find("r n b q k b n r"), std::string::npos);
    ASSERT_NE(stripped_output.find("P P P P P P P P"), std::string::npos);
}

TEST_F(EngineTest, StressTestManyMovesAndResets)
{
    if (!test::CI || std::string(test::CI) != "true")
    {
        GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
    }

    const int num_iterations = 10000;

    for (int i = 0; i < num_iterations; ++i)
    {
        engine.make_move(1, 1, 3, 1);
        engine.make_move(3, 1, 1, 1);

        engine.reset();

        testing::internal::CaptureStdout();
        engine.print_board();
        std::string output = testing::internal::GetCapturedStdout();

        /* Remove ANSI escape sequences using regex */
        std::regex ansi_escape_regex("\033\\[[0-9;]*m");
        std::string stripped_output = std::regex_replace(output, ansi_escape_regex, "");

        ASSERT_NE(stripped_output.find("r n b q k b n r"), std::string::npos);
        ASSERT_NE(stripped_output.find("p p p p p p p p"), std::string::npos);
        ASSERT_NE(stripped_output.find("P P P P P P P P"), std::string::npos);
        ASSERT_NE(stripped_output.find("R N B Q K B N R"), std::string::npos);
    }
}