/*
 *   Copyright (c) 2025 Juan Minor
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
    chess::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_placement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
}

TEST_F(FenTest, ParseColor)
{
    chess::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_color(), chess::WHITE);
}

TEST_F(FenTest, ParseCastling)
{
    chess::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_castling(), "KQkq");
}

TEST_F(FenTest, ParseEnPassant)
{
    chess::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_en_passant(), "-");
}

TEST_F(FenTest, ParseHalfmoveClock)
{
    chess::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_half_move_clock(), 0);
}

TEST_F(FenTest, ParseFullmoves)
{
    chess::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_full_moves(), 1);
}

/* Invalid arguments */
TEST_F(FenTest, InvalidFenStringThrows)
{
    EXPECT_THROW(chess::Fen{"invalid_fen_string"}, std::runtime_error);
    EXPECT_THROW(chess::Fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq"}, std::runtime_error);
    EXPECT_THROW(chess::Fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w X - 0 1"}, std::runtime_error);
    EXPECT_THROW(chess::Fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN w KQkq - 0 1"}, std::runtime_error);
    EXPECT_THROW(chess::Fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1x"}, std::runtime_error);
    EXPECT_THROW(chess::Fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq z3 0 1"}, std::runtime_error);
}

/* Edge cases */
TEST_F(FenTest, NoCastlingRights)
{
    chess::Fen fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    EXPECT_EQ(fen.get_castling(), "-");
}

/* EnPassantSquareSet */
TEST_F(FenTest, EnPassantSquareSet)
{
    chess::Fen fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq e3 0 1");
    EXPECT_EQ(fen.get_en_passant(), "e3");
}

/* Boundary values */
TEST_F(FenTest, BoundaryValues)
{
    chess::Fen fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 100 255");
    EXPECT_EQ(fen.get_half_move_clock(), 100);
    EXPECT_EQ(fen.get_full_moves(), 255);
}

/* Memory management */
TEST_F(FenTest, DestructorCleansUpMemory)
{
    chess::Fen *fen = new chess::Fen(test_fen_string);
    delete fen;
}

/* Stress test */
TEST_F(FenTest, StressTest)
{
    if (!test::get_ci() || std::string(test::get_ci()) != "true")
    {
        GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
    }

    for (int i = 0; i < 100000; ++i)
    {
        chess::Fen fen(test_fen_string);
        EXPECT_EQ(fen.get_placement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
        EXPECT_EQ(fen.get_color(), chess::WHITE);
        EXPECT_EQ(fen.get_castling(), "KQkq");
        EXPECT_EQ(fen.get_en_passant(), "-");
        EXPECT_EQ(fen.get_half_move_clock(), 0);
        EXPECT_EQ(fen.get_full_moves(), 1);
    }
}

TEST_F(FenTest, SetPlacementValid)
{
    chess::Fen fen(test_fen_string);
    fen.set_placement("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R");
    EXPECT_EQ(fen.get_placement(), "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R");
}

TEST_F(FenTest, SetPlacementInvalid)
{
    chess::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_placement("invalid_placement"), std::runtime_error);
    EXPECT_THROW(fen.set_placement("rnbqkbnr/pppppppp"), std::runtime_error);
    EXPECT_THROW(fen.set_placement("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/extra"), std::runtime_error);
}

TEST_F(FenTest, SetCastlingValid)
{
    chess::Fen fen(test_fen_string);
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
    chess::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_castling("X"), std::runtime_error);
    EXPECT_THROW(fen.set_castling("KQX"), std::runtime_error);
    EXPECT_THROW(fen.set_castling("123"), std::runtime_error);
}

TEST_F(FenTest, SetEnPassantValid)
{
    chess::Fen fen(test_fen_string);
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
    chess::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_en_passant("e4"), std::runtime_error);
    EXPECT_THROW(fen.set_en_passant("i3"), std::runtime_error);
    EXPECT_THROW(fen.set_en_passant("e1"), std::runtime_error);
    EXPECT_THROW(fen.set_en_passant("ee"), std::runtime_error);
}

TEST_F(FenTest, SetColorValid)
{
    chess::Fen fen(test_fen_string);
    fen.set_color(chess::BLACK);
    EXPECT_EQ(fen.get_color(), chess::BLACK);

    fen.set_color(chess::WHITE);
    EXPECT_EQ(fen.get_color(), chess::WHITE);
}

TEST_F(FenTest, SetColorInvalid)
{
    chess::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_color(2), std::runtime_error);
    EXPECT_THROW(fen.set_color(255), std::runtime_error);
}

TEST_F(FenTest, SetHalfmoveClock)
{
    chess::Fen fen(test_fen_string);
    fen.set_half_move_clock(50);
    EXPECT_EQ(fen.get_half_move_clock(), 50);

    fen.set_half_move_clock(0);
    EXPECT_EQ(fen.get_half_move_clock(), 0);

    fen.set_half_move_clock(100);
    EXPECT_EQ(fen.get_half_move_clock(), 100);
}

TEST_F(FenTest, SetFullmovesValid)
{
    chess::Fen fen(test_fen_string);
    fen.set_full_moves(1);
    EXPECT_EQ(fen.get_full_moves(), 1);

    fen.set_full_moves(10);
    EXPECT_EQ(fen.get_full_moves(), 10);

    fen.set_full_moves(1000);
    EXPECT_EQ(fen.get_full_moves(), 1000);
}

TEST_F(FenTest, SetFullmovesInvalid)
{
    chess::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_full_moves(0), std::runtime_error);
}

TEST_F(FenTest, EmptyFenString)
{
    EXPECT_THROW(chess::Fen{""}, std::runtime_error);
}

TEST_F(FenTest, GenerateFEN)
{
    chess::Fen fen(test_fen_string);
    std::string generated_fen = fen.generate_fen();
    EXPECT_EQ(generated_fen, test_fen_string);
}

TEST_F(FenTest, GenerateFENAfterModifications)
{
    chess::Fen fen(test_fen_string);
    fen.set_placement("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R");
    fen.set_castling("KQ");
    fen.set_en_passant("e3");
    fen.set_color(chess::BLACK);
    fen.set_half_move_clock(10);
    fen.set_full_moves(2);

    std::string generated_fen = fen.generate_fen();
    EXPECT_EQ(generated_fen, "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQ e3 10 2");
}

TEST_F(FenTest, DefaultConstructor)
{
    chess::Fen fen(test_fen_string);
    EXPECT_EQ(fen.get_placement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    EXPECT_EQ(fen.get_color(), chess::WHITE);
    EXPECT_EQ(fen.get_castling(), "KQkq");
    EXPECT_EQ(fen.get_en_passant(), "-");
    EXPECT_EQ(fen.get_half_move_clock(), 0);
    EXPECT_EQ(fen.get_full_moves(), 1);
}

TEST_F(FenTest, RejectsHalfmoveClockOver100)
{
    std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 101 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, AllowsHalfmoveClock100)
{
    std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 100 1";
    chess::Fen fen(fen_string);
    EXPECT_EQ(fen.get_half_move_clock(), 100);
}

TEST_F(FenTest, RejectsFullmovesZero)
{
    std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsWhitePawnsOnEighthRank)
{
    std::string fen_string = "rnbqkbPr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsBlackPawnsOnFirstRank)
{
    std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBpR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsAdjacentKings)
{
    std::string fen_string = "8/8/8/3kK3/8/8/8/8 w - - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsInvalidPieceCounts)
{
    std::string fen_string = "QQQQQQQQ/QQQQQQQQ/QQQQQQQQ/QQQQQQQQ/qqqqqqqq/qqqqqqqq/qqqqqqqq/qqqqqqqq w - - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, SetterRejectsHalfmoveClockOver100)
{
    chess::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_half_move_clock(101), std::runtime_error);
}

TEST_F(FenTest, SetterAllowsHalfmoveClock100)
{
    chess::Fen fen(test_fen_string);
    fen.set_half_move_clock(100);
    EXPECT_EQ(fen.get_half_move_clock(), 100);
}

TEST_F(FenTest, SetterRejectsFullmovesZero)
{
    chess::Fen fen(test_fen_string);
    EXPECT_THROW(fen.set_full_moves(0), std::runtime_error);
}

TEST_F(FenTest, SetterRejectsInvalidPlacements)
{
    chess::Fen fen(test_fen_string);
    std::string invalid_placement = "rnbqkbPr/pppppppp/8/8/8/8/pPPPPPPP/RNBQKBNR";
    EXPECT_THROW(fen.set_placement(invalid_placement), std::runtime_error);
}

TEST_F(FenTest, ValidComplexPosition)
{
    std::string complex_fen = "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/3P1N2/PPP2PPP/RNBQKB1R w KQkq - 2 4";
    chess::Fen fen(complex_fen);
    EXPECT_EQ(fen.get_placement(), "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/3P1N2/PPP2PPP/RNBQKB1R");
    EXPECT_EQ(fen.get_color(), chess::WHITE);
    EXPECT_EQ(fen.get_castling(), "KQkq");
    EXPECT_EQ(fen.get_en_passant(), "-");
    EXPECT_EQ(fen.get_half_move_clock(), 2);
    EXPECT_EQ(fen.get_full_moves(), 4);
}

TEST_F(FenTest, RejectsInvalidPieceCharacter)
{
    std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBXR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsMultipleKings)
{
    std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKKNR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsTooManyQueens)
{
    std::string fen_string = "rnbqqqqq/qqqqqqqq/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsTooManyRooks)
{
    std::string fen_string = "rrrrrrrrr/rrrrrrrrr/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsTooManyBishops)
{
    std::string fen_string = "bbbbbbbbb/bbbbbbbbb/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsTooManyKnights)
{
    std::string fen_string = "nnnnnnnnn/nnnnnnnnn/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsWhitePromotionExceedsLostPawns)
{
    std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/1PPPPPPP/QQQQQQQQ w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsBlackPromotionExceedsLostPawns)
{
    std::string fen_string = "qqqqqqqq/1ppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsInvalidPieceCharacterInPlacement)
{
    std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBXR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsWhitePawnsOn8thRankInPlacement)
{
    std::string fen_string = "rnbqkbnP/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsBlackPawnsOn1stRankInPlacement)
{
    std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNp w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsWhitePawnOnEighthRankDirectly)
{
    std::string fen_string = "rnbqkbnP/pppppppp/8/8/8/8/PPPPPPP1/RNBQKBNR w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsBlackPawnOnFirstRankDirectly)
{
    std::string fen_string = "rnbqkbnr/ppppppp1/8/8/8/8/PPPPPPPP/RNBQKBNp w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsTooManyQueensInPlacement)
{
    std::string fen_string = "QQQQQQQQQQ/8/8/8/8/8/8/8 w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsTooManyRooksInPlacement)
{
    std::string fen_string = "RRRRRRRRRRR/8/8/8/8/8/8/8 w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsTooManyBishopsInPlacement)
{
    std::string fen_string = "BBBBBBBBBBB/8/8/8/8/8/8/8 w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}

TEST_F(FenTest, RejectsTooManyKnightsInPlacement)
{
    std::string fen_string = "NNNNNNNNNNN/8/8/8/8/8/8/8 w KQkq - 0 1";
    EXPECT_THROW(chess::Fen{fen_string}, std::runtime_error);
}
