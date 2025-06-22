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
 *   along with this program.  If not, see <https:
 */

#include <gtest/gtest.h>
#include "include/engine/engine.h"

class EngineTest : public ::testing::Test
{
protected:
    fenrir::Engine engine;

    char get_piece(const std::string &square)
    {
#ifndef NDEBUG
        return engine.get_piece(square);
#else

        return '.';
#endif
    }
};

TEST_F(EngineTest, DefaultBoardSetup)
{
    EXPECT_EQ(get_piece("a1"), 'R');
    EXPECT_EQ(get_piece("e1"), 'K');
    EXPECT_EQ(get_piece("a8"), 'r');
    EXPECT_EQ(get_piece("e8"), 'k');

    EXPECT_EQ(get_piece("a2"), 'P');
    EXPECT_EQ(get_piece("h7"), 'p');

    EXPECT_EQ(get_piece("e4"), '.');
    EXPECT_EQ(get_piece("d5"), '.');
}

TEST_F(EngineTest, MakeMove)
{
    engine.make_move("b2", "b4");

    EXPECT_EQ(get_piece("b2"), '.');
    EXPECT_EQ(get_piece("b4"), 'P');

    EXPECT_EQ(get_piece("a1"), 'R');
    EXPECT_EQ(get_piece("c2"), 'P');
}

TEST_F(EngineTest, ResetBoard)
{
    engine.make_move("b2", "b4");
    EXPECT_EQ(get_piece("b4"), 'P');

    engine.reset();

    EXPECT_EQ(get_piece("b2"), 'P');
    EXPECT_EQ(get_piece("b4"), '.');
    EXPECT_EQ(get_piece("e1"), 'K');
    EXPECT_EQ(get_piece("e8"), 'k');
}

TEST_F(EngineTest, GenerateMovesValidPiece)
{

    auto moves = engine.generate_moves("b2");
    EXPECT_FALSE(moves.empty());

    bool found_b3 = false;
    bool found_b4 = false;
    for (const auto &move : moves)
    {
        if (move.second == "b3")
            found_b3 = true;
        if (move.second == "b4")
            found_b4 = true;
    }
    EXPECT_TRUE(found_b3 || found_b4);
}

TEST_F(EngineTest, GenerateMovesInvalidAddress)
{

    EXPECT_THROW(engine.generate_moves("z9"), std::runtime_error);
}

TEST_F(EngineTest, GenerateMovesEmptySquare)
{

    auto moves = engine.generate_moves("e4");
    EXPECT_TRUE(moves.empty());
}

TEST_F(EngineTest, MakeMoveFromEmptySquare)
{

    testing::internal::CaptureStderr();
    engine.make_move("e4", "e5");
    std::string error_output = testing::internal::GetCapturedStderr();

    EXPECT_EQ(get_piece("e4"), '.');
    EXPECT_EQ(get_piece("e5"), '.');
}

TEST_F(EngineTest, PrintBoard)
{

    testing::internal::CaptureStdout();
    engine.print_board();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_FALSE(output.empty());
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
        engine.make_move("b2", "b4");
        engine.make_move("b4", "b2");

        engine.reset();

        ASSERT_EQ(get_piece("b2"), 'P');
        ASSERT_EQ(get_piece("e1"), 'K');
        ASSERT_EQ(get_piece("e8"), 'k');
        ASSERT_EQ(get_piece("e4"), '.');
    }
}