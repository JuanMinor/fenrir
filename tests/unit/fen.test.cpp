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
#include <iostream>
#include "include/core/core.h"
#include "include/chess/fen.h"
#include "include/modifier/modifier.h"

class FenTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        test_fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }

    static void TearDownTestSuite() {}

    static std::string test_fen_string;
};

std::string FenTest::test_fen_string = "";

/* Parsing tests */
TEST_F(FenTest, ParsePlacement)
{
    fenrir::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_placement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
}

TEST_F(FenTest, ParseColor)
{
    fenrir::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_color(), fenrir::WHITE);
}

TEST_F(FenTest, ParseCastling)
{
    fenrir::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_castling(), "KQkq");
}

TEST_F(FenTest, ParseEnPassant)
{
    fenrir::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_en_passant(), "-");
}

TEST_F(FenTest, ParseHalfmoveClock)
{
    fenrir::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_halfmove_clock(), 0);
}

TEST_F(FenTest, ParseFullmoves)
{
    fenrir::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_fullmoves(), 1);
}

/* Invalid arguments */
TEST_F(FenTest, InvalidFenStringThrows)
{
    EXPECT_THROW(fenrir::Fen("invalid_fen_string"), std::runtime_error);
    EXPECT_THROW(fenrir::Fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq"), std::runtime_error);
    EXPECT_THROW(fenrir::Fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w X - 0 1"), std::runtime_error);
    EXPECT_THROW(fenrir::Fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN w KQkq - 0 1"), std::runtime_error);
    EXPECT_THROW(fenrir::Fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1x"), std::runtime_error);
    EXPECT_THROW(fenrir::Fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq z3 0 1"), std::runtime_error);
}

/* Edge cases */
TEST_F(FenTest, NoCastlingRights)
{
    fenrir::Fen fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    EXPECT_EQ(fen.get_castling(), "-");
}

TEST_F(FenTest, EnPassantSquareSet)
{
    fenrir::Fen fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq e3 0 1");
    EXPECT_EQ(fen.get_en_passant(), "e3");
}

/* Boundary values */
TEST_F(FenTest, BoundaryValues)
{
    fenrir::Fen fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 255 255");
    EXPECT_EQ(fen.get_halfmove_clock(), 255);
    EXPECT_EQ(fen.get_fullmoves(), 255);
}

/* Memory management */
TEST_F(FenTest, DestructorCleansUpMemory)
{
    fenrir::Fen *fen = new fenrir::Fen(test_fen_string);
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
        fenrir::Fen fen(test_fen_string);
        EXPECT_EQ(fen.get_placement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
        EXPECT_EQ(fen.get_color(), fenrir::WHITE);
        EXPECT_EQ(fen.get_castling(), "KQkq");
        EXPECT_EQ(fen.get_en_passant(), "-");
        EXPECT_EQ(fen.get_halfmove_clock(), 0);
        EXPECT_EQ(fen.get_fullmoves(), 1);
    }
}

TEST_F(FenTest, SetPlacementValid)
{
    fenrir::Fen fen(test_fen_string);
    fen.set_placement("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R");
    EXPECT_EQ(fen.get_placement(), "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R");
}

TEST_F(FenTest, SetPlacementInvalid)
{
    fenrir::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_placement("invalid_placement"), std::runtime_error);
    EXPECT_THROW(fen.set_placement("rnbqkbnr/pppppppp"), std::runtime_error);
    EXPECT_THROW(fen.set_placement("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/extra"), std::runtime_error);
}

TEST_F(FenTest, SetCastlingValid)
{
    fenrir::Fen fen(test_fen_string);
    fen.set_castling("KQ");
    EXPECT_EQ(fen.get_castling(), "KQ");

    fen.set_castling("kq");
    EXPECT_EQ(fen.get_castling(), "kq");

    fen.set_castling("K");
    EXPECT_EQ(fen.get_castling(), "K");

    fen.set_castling("-");
    EXPECT_EQ(fen.get_castling(), "-");
}

TEST_F(FenTest, SetCastlingInvalid)
{
    fenrir::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_castling("X"), std::runtime_error);
    EXPECT_THROW(fen.set_castling("KQX"), std::runtime_error);
    EXPECT_THROW(fen.set_castling("123"), std::runtime_error);
}

TEST_F(FenTest, SetEnPassantValid)
{
    fenrir::Fen fen(test_fen_string);
    fen.set_en_passant("e3");
    EXPECT_EQ(fen.get_en_passant(), "e3");

    fen.set_en_passant("a6");
    EXPECT_EQ(fen.get_en_passant(), "a6");

    fen.set_en_passant("h3");
    EXPECT_EQ(fen.get_en_passant(), "h3");

    fen.set_en_passant("-");
    EXPECT_EQ(fen.get_en_passant(), "-");
}

TEST_F(FenTest, SetEnPassantInvalid)
{
    fenrir::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_en_passant("e4"), std::runtime_error);
    EXPECT_THROW(fen.set_en_passant("i3"), std::runtime_error);
    EXPECT_THROW(fen.set_en_passant("e1"), std::runtime_error);
    EXPECT_THROW(fen.set_en_passant("ee"), std::runtime_error);
}

TEST_F(FenTest, SetColorValid)
{
    fenrir::Fen fen(test_fen_string);
    fen.set_color(fenrir::BLACK);
    EXPECT_EQ(fen.get_color(), fenrir::BLACK);

    fen.set_color(fenrir::WHITE);
    EXPECT_EQ(fen.get_color(), fenrir::WHITE);
}

TEST_F(FenTest, SetColorInvalid)
{
    fenrir::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_color(2), std::runtime_error);
    EXPECT_THROW(fen.set_color(255), std::runtime_error);
}

TEST_F(FenTest, SetHalfmoveClock)
{
    fenrir::Fen fen(test_fen_string);
    fen.set_halfmove_clock(50);
    EXPECT_EQ(fen.get_halfmove_clock(), 50);

    fen.set_halfmove_clock(0);
    EXPECT_EQ(fen.get_halfmove_clock(), 0);

    fen.set_halfmove_clock(100);
    EXPECT_EQ(fen.get_halfmove_clock(), 100);
}

TEST_F(FenTest, SetFullmovesValid)
{
    fenrir::Fen fen(test_fen_string);
    fen.set_fullmoves(1);
    EXPECT_EQ(fen.get_fullmoves(), 1);

    fen.set_fullmoves(10);
    EXPECT_EQ(fen.get_fullmoves(), 10);

    fen.set_fullmoves(1000);
    EXPECT_EQ(fen.get_fullmoves(), 1000);
}

TEST_F(FenTest, SetFullmovesInvalid)
{
    fenrir::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_fullmoves(0), std::runtime_error);
}

TEST_F(FenTest, EmptyFenString)
{
    EXPECT_THROW(fenrir::Fen(""), std::runtime_error);
}

TEST_F(FenTest, GenerateFEN)
{
    fenrir::Fen fen(test_fen_string);
    std::string generated_fen = fen.generate_fen();
    EXPECT_EQ(generated_fen, test_fen_string);
}
TEST_F(FenTest, GenerateFENAfterModifications)
{
    fenrir::Fen fen(test_fen_string);
    fen.set_placement("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R");
    fen.set_castling("KQ");
    fen.set_en_passant("e3");
    fen.set_color(fenrir::BLACK);
    fen.set_halfmove_clock(10);
    fen.set_fullmoves(2);

    std::string generated_fen = fen.generate_fen();
    EXPECT_EQ(generated_fen, "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQ e3 10 2");
}