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
#include <iostream>
#include "include/core/core.h"
#include "include/chess/fen.h"
#include "include/modifier/modifier.h"

class FenTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::cout << color::Modifier(color::Color::FG_CYAN) << std::endl
                  << "****************************************************" << std::endl
                  << "Forsyth-Edwards Notation (FEN) tests are running..." << std::endl
                  << "****************************************************" << std::endl
                  << color::Modifier(color::Color::RESET) << std::endl;
        test_fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }

    static void TearDownTestSuite() {}

    static const char *test_fen_string;
};

const char *FenTest::test_fen_string = nullptr;

/* Parsing tests */
TEST_F(FenTest, ParsePlacement)
{
    loki::Fen fen(test_fen_string);
    EXPECT_STREQ(fen.get_placement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
}

TEST_F(FenTest, ParseColor)
{
    loki::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_color(), loki::WHITE);
}

TEST_F(FenTest, ParseCastling)
{
    loki::Fen fen(test_fen_string);
    EXPECT_STREQ(fen.get_castling(), "KQkq");
}

TEST_F(FenTest, ParseEnPassant)
{
    loki::Fen fen(test_fen_string);
    EXPECT_STREQ(fen.get_en_passant(), "-");
}

TEST_F(FenTest, ParseHalfmoveClock)
{
    loki::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_halfmove_clock(), 0);
}

TEST_F(FenTest, ParseFullmoves)
{
    loki::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_fullmoves(), 1);
}

/* Invalid arguments */
TEST_F(FenTest, InvalidFenStringThrows)
{
    EXPECT_THROW(loki::Fen("invalid_fen_string"), std::invalid_argument);
    EXPECT_THROW(loki::Fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq"), std::invalid_argument);
    EXPECT_THROW(loki::Fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w X - 0 1"), std::invalid_argument);
    EXPECT_THROW(loki::Fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN w KQkq - 0 1"), std::invalid_argument);
    EXPECT_THROW(loki::Fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1x"), std::invalid_argument);
    EXPECT_THROW(loki::Fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq z3 0 1"), std::invalid_argument);
}

/* Edge cases */
TEST_F(FenTest, NoCastlingRights)
{
    loki::Fen fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    EXPECT_STREQ(fen.get_castling(), "-");
}

TEST_F(FenTest, EnPassantSquareSet)
{
    loki::Fen fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq e3 0 1");
    EXPECT_STREQ(fen.get_en_passant(), "e3");
}

/* Boundary values */
TEST_F(FenTest, BoundaryValues)
{
    loki::Fen fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 255 255");
    EXPECT_EQ(fen.get_halfmove_clock(), 255);
    EXPECT_EQ(fen.get_fullmoves(), 255);
}

/* Memory management */
TEST_F(FenTest, DestructorCleansUpMemory)
{
    loki::Fen *fen = new loki::Fen(test_fen_string);
    delete fen;
}

/* Stress test */
TEST_F(FenTest, StressTest)
{
    if (!test::CI || std::string(test::CI) != "true")
    {
        GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
    }

    for (int i = 0; i < 100000; ++i)
    {
        loki::Fen fen(test_fen_string);
        EXPECT_STREQ(fen.get_placement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
        EXPECT_EQ(fen.get_color(), loki::WHITE);
        EXPECT_STREQ(fen.get_castling(), "KQkq");
        EXPECT_STREQ(fen.get_en_passant(), "-");
        EXPECT_EQ(fen.get_halfmove_clock(), 0);
        EXPECT_EQ(fen.get_fullmoves(), 1);
    }
}
