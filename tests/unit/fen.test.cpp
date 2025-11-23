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
	EXPECT_EQ(fen.getPlacement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
}

TEST_F(FenTest, ParseColor)
{
	fenrir::Fen fen(test_fen_string);
	EXPECT_EQ(fen.getColor(), fenrir::WHITE);
}

TEST_F(FenTest, ParseCastling)
{
	fenrir::Fen fen(test_fen_string);
	EXPECT_EQ(fen.getCastling(), "KQkq");
}

TEST_F(FenTest, ParseEnPassant)
{
	fenrir::Fen fen(test_fen_string);
	EXPECT_EQ(fen.getEnPassant(), "-");
}

TEST_F(FenTest, ParseHalfmoveClock)
{
	fenrir::Fen fen(test_fen_string);
	EXPECT_EQ(fen.getHalfMoveClock(), 0);
}

TEST_F(FenTest, ParseFullmoves)
{
	fenrir::Fen fen(test_fen_string);
	EXPECT_EQ(fen.getFullMoves(), 1);
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
	EXPECT_EQ(fen.getCastling(), "-");
}

TEST_F(FenTest, EnPassantSquareSet)
{
	fenrir::Fen fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq e3 0 1");
	EXPECT_EQ(fen.getEnPassant(), "e3");
}

/* Boundary values */
TEST_F(FenTest, BoundaryValues)
{
	fenrir::Fen fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 255 255");
	EXPECT_EQ(fen.getHalfMoveClock(), 255);
	EXPECT_EQ(fen.getFullMoves(), 255);
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
		EXPECT_EQ(fen.getPlacement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
		EXPECT_EQ(fen.getColor(), fenrir::WHITE);
		EXPECT_EQ(fen.getCastling(), "KQkq");
		EXPECT_EQ(fen.getEnPassant(), "-");
		EXPECT_EQ(fen.getHalfMoveClock(), 0);
		EXPECT_EQ(fen.getFullMoves(), 1);
	}
}

TEST_F(FenTest, SetPlacementValid)
{
	fenrir::Fen fen(test_fen_string);
	fen.setPlacement("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R");
	EXPECT_EQ(fen.getPlacement(), "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R");
}

TEST_F(FenTest, SetPlacementInvalid)
{
	fenrir::Fen fen(test_fen_string);
	EXPECT_THROW(fen.setPlacement("invalid_placement"), std::runtime_error);
	EXPECT_THROW(fen.setPlacement("rnbqkbnr/pppppppp"), std::runtime_error);
	EXPECT_THROW(fen.setPlacement("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/extra"), std::runtime_error);
}

TEST_F(FenTest, SetCastlingValid)
{
	fenrir::Fen fen(test_fen_string);
	fen.setCastling("KQ");
	EXPECT_EQ(fen.getCastling(), "KQ");

	fen.setCastling("kq");
	EXPECT_EQ(fen.getCastling(), "kq");

	fen.setCastling("K");
	EXPECT_EQ(fen.getCastling(), "K");

	fen.setCastling("-");
	EXPECT_EQ(fen.getCastling(), "-");
}

TEST_F(FenTest, SetCastlingInvalid)
{
	fenrir::Fen fen(test_fen_string);
	EXPECT_THROW(fen.setCastling("X"), std::runtime_error);
	EXPECT_THROW(fen.setCastling("KQX"), std::runtime_error);
	EXPECT_THROW(fen.setCastling("123"), std::runtime_error);
}

TEST_F(FenTest, SetEnPassantValid)
{
	fenrir::Fen fen(test_fen_string);
	fen.setEnPassant("e3");
	EXPECT_EQ(fen.getEnPassant(), "e3");

	fen.setEnPassant("a6");
	EXPECT_EQ(fen.getEnPassant(), "a6");

	fen.setEnPassant("h3");
	EXPECT_EQ(fen.getEnPassant(), "h3");

	fen.setEnPassant("-");
	EXPECT_EQ(fen.getEnPassant(), "-");
}

TEST_F(FenTest, SetEnPassantInvalid)
{
	fenrir::Fen fen(test_fen_string);
	EXPECT_THROW(fen.setEnPassant("e4"), std::runtime_error);
	EXPECT_THROW(fen.setEnPassant("i3"), std::runtime_error);
	EXPECT_THROW(fen.setEnPassant("e1"), std::runtime_error);
	EXPECT_THROW(fen.setEnPassant("ee"), std::runtime_error);
}

TEST_F(FenTest, SetColorValid)
{
	fenrir::Fen fen(test_fen_string);
	fen.setColor(fenrir::BLACK);
	EXPECT_EQ(fen.getColor(), fenrir::BLACK);

	fen.setColor(fenrir::WHITE);
	EXPECT_EQ(fen.getColor(), fenrir::WHITE);
}

TEST_F(FenTest, SetColorInvalid)
{
	fenrir::Fen fen(test_fen_string);
	EXPECT_THROW(fen.setColor(2), std::runtime_error);
	EXPECT_THROW(fen.setColor(255), std::runtime_error);
}

TEST_F(FenTest, SetHalfmoveClock)
{
	fenrir::Fen fen(test_fen_string);
	fen.setHalfMoveClock(50);
	EXPECT_EQ(fen.getHalfMoveClock(), 50);

	fen.setHalfMoveClock(0);
	EXPECT_EQ(fen.getHalfMoveClock(), 0);

	fen.setHalfMoveClock(100);
	EXPECT_EQ(fen.getHalfMoveClock(), 100);
}

TEST_F(FenTest, SetFullmovesValid)
{
	fenrir::Fen fen(test_fen_string);
	fen.setFullMoves(1);
	EXPECT_EQ(fen.getFullMoves(), 1);

	fen.setFullMoves(10);
	EXPECT_EQ(fen.getFullMoves(), 10);

	fen.setFullMoves(1000);
	EXPECT_EQ(fen.getFullMoves(), 1000);
}

TEST_F(FenTest, SetFullmovesInvalid)
{
	fenrir::Fen fen(test_fen_string);
	EXPECT_THROW(fen.setFullMoves(0), std::runtime_error);
}

TEST_F(FenTest, EmptyFenString)
{
	EXPECT_THROW(fenrir::Fen(""), std::runtime_error);
}

TEST_F(FenTest, GenerateFEN)
{
	fenrir::Fen fen(test_fen_string);
	std::string generated_fen = fen.generateFen();
	EXPECT_EQ(generated_fen, test_fen_string);
}
TEST_F(FenTest, GenerateFENAfterModifications)
{
	fenrir::Fen fen(test_fen_string);
	fen.setPlacement("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R");
	fen.setCastling("KQ");
	fen.setEnPassant("e3");
	fen.setColor(fenrir::BLACK);
	fen.setHalfMoveClock(10);
	fen.setFullMoves(2);

	std::string generated_fen = fen.generateFen();
	EXPECT_EQ(generated_fen, "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQ e3 10 2");
}

TEST_F(FenTest, PermissiveModeDefaultConstructor)
{
	fenrir::Fen fen(test_fen_string);
	EXPECT_EQ(fen.getPlacement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
	EXPECT_EQ(fen.getColor(), fenrir::WHITE);
	EXPECT_EQ(fen.getCastling(), "KQkq");
	EXPECT_EQ(fen.getEnPassant(), "-");
	EXPECT_EQ(fen.getHalfMoveClock(), 0);
	EXPECT_EQ(fen.getFullMoves(), 1);
}

TEST_F(FenTest, PermissiveModeExplicitConstructor)
{
	fenrir::Fen fen(test_fen_string, fenrir::GameMode::PERMISSIVE);
	EXPECT_EQ(fen.getPlacement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
	EXPECT_EQ(fen.getColor(), fenrir::WHITE);
	EXPECT_EQ(fen.getCastling(), "KQkq");
	EXPECT_EQ(fen.getEnPassant(), "-");
	EXPECT_EQ(fen.getHalfMoveClock(), 0);
	EXPECT_EQ(fen.getFullMoves(), 1);
}

TEST_F(FenTest, TournamentModeValidFen)
{
	fenrir::Fen fen(test_fen_string, fenrir::GameMode::TOURNAMENT);
	EXPECT_EQ(fen.getPlacement(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
	EXPECT_EQ(fen.getColor(), fenrir::WHITE);
	EXPECT_EQ(fen.getCastling(), "KQkq");
	EXPECT_EQ(fen.getEnPassant(), "-");
	EXPECT_EQ(fen.getHalfMoveClock(), 0);
	EXPECT_EQ(fen.getFullMoves(), 1);
}

TEST_F(FenTest, PermissiveModeAllowsHalfmoveClockOver100)
{
	std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 150 1";
	fenrir::Fen fen(fen_string, fenrir::GameMode::PERMISSIVE);
	EXPECT_EQ(fen.getHalfMoveClock(), 150);
}

TEST_F(FenTest, TournamentModeRejectsHalfmoveClockOver100)
{
	std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 101 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeAllowsHalfmoveClock100)
{
	std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 100 1";
	fenrir::Fen fen(fen_string, fenrir::GameMode::TOURNAMENT);
	EXPECT_EQ(fen.getHalfMoveClock(), 100);
}

TEST_F(FenTest, TournamentModeRejectsFullmovesZero)
{
	std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, PermissiveModeAllowsInvalidPieceCountsWithPawnsOnPromotionRanks)
{
	std::string fen_string = "rnbqkbPr/pppppppp/8/8/8/8/pPPPPPPP/RNBQKBNR w KQkq - 0 1";
	fenrir::Fen fen(fen_string, fenrir::GameMode::PERMISSIVE);
	EXPECT_EQ(fen.getPlacement(), "rnbqkbPr/pppppppp/8/8/8/8/pPPPPPPP/RNBQKBNR");
}

TEST_F(FenTest, TournamentModeRejectsWhitePawnsOnEighthRank)
{
	std::string fen_string = "rnbqkbPr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsBlackPawnsOnFirstRank)
{
	std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBpR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, PermissiveModeAllowsAdjacentKings)
{
	std::string fen_string = "8/8/8/3kK3/8/8/8/8 w - - 0 1";
	fenrir::Fen fen(fen_string, fenrir::GameMode::PERMISSIVE);
	EXPECT_EQ(fen.getPlacement(), "8/8/8/3kK3/8/8/8/8");
}

TEST_F(FenTest, TournamentModeRejectsAdjacentKings)
{
	std::string fen_string = "8/8/8/3kK3/8/8/8/8 w - - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, PermissiveModeAllowsInvalidPieceCounts)
{
	std::string fen_string = "QQQQQQQQ/QQQQQQQQ/QQQQQQQQ/QQQQQQQQ/qqqqqqqq/qqqqqqqq/qqqqqqqq/qqqqqqqq w - - 0 1";
	fenrir::Fen fen(fen_string, fenrir::GameMode::PERMISSIVE);
	EXPECT_EQ(fen.getPlacement(), "QQQQQQQQ/QQQQQQQQ/QQQQQQQQ/QQQQQQQQ/qqqqqqqq/qqqqqqqq/qqqqqqqq/qqqqqqqq");
}

TEST_F(FenTest, TournamentModeRejectsInvalidPieceCounts)
{
	std::string fen_string = "QQQQQQQQ/QQQQQQQQ/QQQQQQQQ/QQQQQQQQ/qqqqqqqq/qqqqqqqq/qqqqqqqq/qqqqqqqq w - - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, PermissiveModeSetterAllowsHalfmoveClockOver100)
{
	fenrir::Fen fen(test_fen_string, fenrir::GameMode::PERMISSIVE);
	fen.setHalfMoveClock(200);
	EXPECT_EQ(fen.getHalfMoveClock(), 200);
}

TEST_F(FenTest, TournamentModeSetterRejectsHalfmoveClockOver100)
{
	fenrir::Fen fen(test_fen_string, fenrir::GameMode::TOURNAMENT);
	EXPECT_THROW(fen.setHalfMoveClock(101), std::runtime_error);
}

TEST_F(FenTest, TournamentModeSetterAllowsHalfmoveClock100)
{
	fenrir::Fen fen(test_fen_string, fenrir::GameMode::TOURNAMENT);
	fen.setHalfMoveClock(100);
	EXPECT_EQ(fen.getHalfMoveClock(), 100);
}

TEST_F(FenTest, BothModesSetterRejectsFullmovesZero)
{
	fenrir::Fen fen_permissive(test_fen_string, fenrir::GameMode::PERMISSIVE);
	fenrir::Fen fen_tournament(test_fen_string, fenrir::GameMode::TOURNAMENT);

	EXPECT_THROW(fen_permissive.setFullMoves(0), std::runtime_error);
	EXPECT_THROW(fen_tournament.setFullMoves(0), std::runtime_error);
}

TEST_F(FenTest, PermissiveModeSetterAllowsInvalidPlacements)
{
	fenrir::Fen fen(test_fen_string, fenrir::GameMode::PERMISSIVE);
	std::string invalid_placement = "rnbqkbPr/pppppppp/8/8/8/8/pPPPPPPP/RNBQKBNR";
	fen.setPlacement(invalid_placement);
	EXPECT_EQ(fen.getPlacement(), invalid_placement);
}

TEST_F(FenTest, TournamentModeSetterRejectsInvalidPlacements)
{
	fenrir::Fen fen(test_fen_string, fenrir::GameMode::TOURNAMENT);
	std::string invalid_placement = "rnbqkbPr/pppppppp/8/8/8/8/pPPPPPPP/RNBQKBNR";
	EXPECT_THROW(fen.setPlacement(invalid_placement), std::runtime_error);
}

TEST_F(FenTest, TournamentModeValidComplexPosition)
{
	std::string complex_fen = "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/3P1N2/PPP2PPP/RNBQKB1R w KQkq - 2 4";
	fenrir::Fen fen(complex_fen, fenrir::GameMode::TOURNAMENT);
	EXPECT_EQ(fen.getPlacement(), "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/3P1N2/PPP2PPP/RNBQKB1R");
	EXPECT_EQ(fen.getColor(), fenrir::WHITE);
	EXPECT_EQ(fen.getCastling(), "KQkq");
	EXPECT_EQ(fen.getEnPassant(), "-");
	EXPECT_EQ(fen.getHalfMoveClock(), 2);
	EXPECT_EQ(fen.getFullMoves(), 4);
}

TEST_F(FenTest, PermissiveModeValidComplexPosition)
{
	std::string complex_fen = "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/3P1N2/PPP2PPP/RNBQKB1R w KQkq - 2 4";
	fenrir::Fen fen(complex_fen, fenrir::GameMode::PERMISSIVE);
	EXPECT_EQ(fen.getPlacement(), "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/3P1N2/PPP2PPP/RNBQKB1R");
	EXPECT_EQ(fen.getColor(), fenrir::WHITE);
	EXPECT_EQ(fen.getCastling(), "KQkq");
	EXPECT_EQ(fen.getEnPassant(), "-");
	EXPECT_EQ(fen.getHalfMoveClock(), 2);
	EXPECT_EQ(fen.getFullMoves(), 4);
}

TEST_F(FenTest, TournamentModeRejectsInvalidPieceCharacter)
{
	std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBXR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsMultipleKings)
{
	std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKKNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsTooManyQueens)
{
	std::string fen_string = "rnbqqqqq/qqqqqqqq/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsTooManyRooks)
{
	std::string fen_string = "rrrrrrrrr/rrrrrrrrr/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsTooManyBishops)
{
	std::string fen_string = "bbbbbbbbb/bbbbbbbbb/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsTooManyKnights)
{
	std::string fen_string = "nnnnnnnnn/nnnnnnnnn/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsWhitePromotionExceedsLostPawns)
{
	std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/1PPPPPPP/QQQQQQQQ w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsBlackPromotionExceedsLostPawns)
{
	std::string fen_string = "qqqqqqqq/1ppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsInvalidPieceCharacterInPlacement)
{
	std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBXR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsWhitePawnsOn8thRankInPlacement)
{
	std::string fen_string = "rnbqkbnP/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsBlackPawnsOn1stRankInPlacement)
{
	std::string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNp w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsWhitePawnOnEighthRankDirectly)
{
	std::string fen_string = "rnbqkbnP/pppppppp/8/8/8/8/PPPPPPP1/RNBQKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsBlackPawnOnFirstRankDirectly)
{
	std::string fen_string = "rnbqkbnr/ppppppp1/8/8/8/8/PPPPPPPP/RNBQKBNp w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsTooManyQueensInPlacement)
{
	std::string fen_string = "QQQQQQQQQQ/8/8/8/8/8/8/8 w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsTooManyRooksInPlacement)
{
	std::string fen_string = "RRRRRRRRRRR/8/8/8/8/8/8/8 w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsTooManyBishopsInPlacement)
{
	std::string fen_string = "BBBBBBBBBBB/8/8/8/8/8/8/8 w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}

TEST_F(FenTest, TournamentModeRejectsTooManyKnightsInPlacement)
{
	std::string fen_string = "NNNNNNNNNNN/8/8/8/8/8/8/8 w KQkq - 0 1";
	EXPECT_THROW(fenrir::Fen(fen_string, fenrir::GameMode::TOURNAMENT), std::runtime_error);
}
