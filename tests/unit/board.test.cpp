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
#include <regex>
#include "include/chess/board.h"
#include "include/chess/piece.h"
#include "include/core/core.h"

class BoardTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		valid_fen_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq a3 0 1";
		empty_fen_position = "8/8/8/8/8/8/8/8 w KQkq - 0 1";
	}

	static void TearDownTestSuite() {}

	static std::string valid_fen_position;
	static std::string empty_fen_position;
};

std::string BoardTest::valid_fen_position = "";
std::string BoardTest::empty_fen_position = "";

/* Parsing tests */
TEST_F(BoardTest, ParseValidFENPosition)
{
	fenrir::Board board(valid_fen_position);

	/* Check pieces via getPiece() */
	/* White rook */
	const fenrir::Piece *rook = board.getPiece(0, 0);
	EXPECT_NE(rook, nullptr);
	EXPECT_EQ(rook->getAlias(), 'R');
	EXPECT_EQ(rook->getColor(), fenrir::WHITE);

	/* Black king */
	const fenrir::Piece *king = board.getPiece(7, 4);
	EXPECT_NE(king, nullptr);
	EXPECT_EQ(king->getAlias(), 'k');
	EXPECT_EQ(king->getColor(), fenrir::BLACK);
}

TEST_F(BoardTest, ParseEmptyFENPosition)
{
	fenrir::Board board(empty_fen_position);

	for (uint8_t r = 0; r < fenrir::BOARD_SIZE; ++r)
	{
		for (uint8_t f = 0; f < fenrir::BOARD_SIZE; ++f)
		{
			EXPECT_EQ(board.getPiece(r, f), nullptr);
		}
	}
}

TEST_F(BoardTest, SetEnPassant)
{
	fenrir::Board board(valid_fen_position);

	EXPECT_EQ(board.getEnPassant(), "a3");
}

TEST_F(BoardTest, GetPiece_AtValidPosition)
{
	fenrir::Board board(valid_fen_position);
	fenrir::Piece *piece = board.getPiece(0, 0);

	ASSERT_NE(piece, nullptr);
	EXPECT_EQ(piece->getAlias(), 'R');
	EXPECT_EQ(piece->getColor(), fenrir::WHITE);
}

TEST_F(BoardTest, GetPiece_AtInvalidPosition)
{
	fenrir::Board board(valid_fen_position);
	fenrir::Piece *piece = board.getPiece(8, 0);

	EXPECT_EQ(piece, nullptr);
}

/* Move tests */
TEST_F(BoardTest, MovePiece)
{
	fenrir::Board board(valid_fen_position);

	/* White pawn @ a2 */
	fenrir::Piece *pawn = board.getPiece(1, 0);

	ASSERT_NE(pawn, nullptr);
	EXPECT_EQ(pawn->getAlias(), 'P');
	EXPECT_EQ(pawn->getColor(), fenrir::WHITE);

	/* Move the pawn from a2 to a4 */
	board.move(pawn, 3, 0);

	EXPECT_NE(board.getPiece(3, 0), nullptr);
	EXPECT_EQ(board.getPiece(1, 0), nullptr);
	EXPECT_EQ(board.getPiece(3, 0)->getRank(), 3);
	EXPECT_EQ(board.getPiece(3, 0)->getFile(), 0);
}

TEST_F(BoardTest, MoveToOccupiedSquare)
{
	fenrir::Board board(valid_fen_position);

	/* White pawn @ a2 */
	fenrir::Piece *pawn = board.getPiece(1, 0);
	ASSERT_NE(pawn, nullptr);

	/* Attempt to move the pawn to an occupied square (a1) */
	board.move(pawn, 0, 0);

	/* Ensure the move was not performed */
	EXPECT_NE(board.getPiece(1, 0), nullptr);
	EXPECT_NE(board.getPiece(0, 0), nullptr);
}

TEST_F(BoardTest, MoveOutOfBounds)
{
	fenrir::Board board(valid_fen_position);

	/* Black pawn @ a7 (rank 6) */
	fenrir::Piece *pawn = board.getPiece(6, 0);

	ASSERT_NE(pawn, nullptr);

	/* Attempt to move out of bounds */
	board.move(pawn, 8, 0);

	EXPECT_NE(board.getPiece(6, 0), nullptr);
}

TEST_F(BoardTest, MovePawnAndEnPassantIsSet)
{
	fenrir::Board board(valid_fen_position);

	/* White pawn @ a2 */
	fenrir::Piece *pawn = board.getPiece(1, 0);
	ASSERT_NE(pawn, nullptr);

	/* Move the pawn to a4 */
	board.move(pawn, 3, 0);

	EXPECT_EQ(board.getEnPassant(), "a3");
}

TEST_F(BoardTest, MovePawnAndEnPassantIsCleared)
{
	fenrir::Board board(valid_fen_position);

	/* White pawn @ a2 */
	fenrir::Piece *pawn = board.getPiece(1, 0);
	ASSERT_NE(pawn, nullptr);

	/* Move the pawn to a4 */
	board.move(pawn, 3, 0);

	/* Move the pawn to a5 */
	board.move(pawn, 4, 0);

	EXPECT_EQ(board.getEnPassant(), "");
}

/* Error handling tests for uncovered lines */
TEST_F(BoardTest, InvalidFENIncorrectSquaresInRank)
{

	std::string invalid_fen = "rnbqkbnr/pppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Board board(invalid_fen), std::runtime_error);
}

TEST_F(BoardTest, InvalidFENUnknownPieceCharacter)
{

	std::string invalid_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBXKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Board board(invalid_fen), std::runtime_error);
}

TEST_F(BoardTest, InvalidFENBoardCreationFailure)
{

	std::string invalid_fen = "rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Board board(invalid_fen), std::runtime_error);
}

TEST_F(BoardTest, InvalidFENTooManySquaresInRank)
{

	std::string invalid_fen = "rnbqkbnr/ppppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	EXPECT_THROW(fenrir::Board board(invalid_fen), std::runtime_error);
}

TEST_F(BoardTest, MoveFromInvalidBoardPosition)
{
	fenrir::Board board(valid_fen_position);

	/* A piece not on the board */
	fenrir::Piece detached_piece('P', 4, 4);

	board.move(&detached_piece, 5, 4);

	EXPECT_EQ(board.getPiece(4, 4), nullptr);
	EXPECT_EQ(board.getPiece(5, 4), nullptr);
}

TEST_F(BoardTest, StressTestManyMoves)
{
	if (!test::getCI() || std::string(test::getCI()) != "true")

	{
		GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
	}

	fenrir::Board board(valid_fen_position);

	uint8_t white_pawn_rank = 1, white_pawn_file = 0;
	uint8_t black_pawn_rank = 6, black_pawn_file = 0;
	int white_pawn_direction = 1, black_pawn_direction = -1;

	for (int i = 0; i < 100000; ++i)
	{
		if (i % 2 == 0)
		{
			fenrir::Piece *pawn = board.getPiece(white_pawn_rank, white_pawn_file);
			ASSERT_NE(pawn, nullptr);
			ASSERT_EQ(pawn->getAlias(), 'P');
			ASSERT_EQ(pawn->getColor(), fenrir::WHITE);

			uint8_t target_rank = static_cast<uint8_t>(white_pawn_rank + white_pawn_direction);
			if (!board.getPiece(target_rank, white_pawn_file))
			{
				board.move(pawn, target_rank, white_pawn_file);
				EXPECT_NE(board.getPiece(target_rank, white_pawn_file), nullptr);
				EXPECT_EQ(board.getPiece(white_pawn_rank, white_pawn_file), nullptr);
				white_pawn_rank = target_rank;
			}
			if (white_pawn_rank == 0 || white_pawn_rank == fenrir::BOARD_SIZE - 1)
			{
				white_pawn_direction *= -1;
			}
		}
		else
		{
			fenrir::Piece *pawn = board.getPiece(black_pawn_rank, black_pawn_file);
			ASSERT_NE(pawn, nullptr);
			ASSERT_EQ(pawn->getAlias(), 'p');
			ASSERT_EQ(pawn->getColor(), fenrir::BLACK);

			uint8_t target_rank = static_cast<uint8_t>(black_pawn_rank + black_pawn_direction);
			if (!board.getPiece(target_rank, black_pawn_file))
			{
				board.move(pawn, target_rank, black_pawn_file);
				EXPECT_NE(board.getPiece(target_rank, black_pawn_file), nullptr);
				EXPECT_EQ(board.getPiece(black_pawn_rank, black_pawn_file), nullptr);
				black_pawn_rank = target_rank;
			}
			if (black_pawn_rank == 0 || black_pawn_rank == fenrir::BOARD_SIZE - 1)
			{
				black_pawn_direction *= -1;
			}
		}
	}

	/* Verify all remaining pieces are within bounds */
	for (uint8_t r = 0; r < fenrir::BOARD_SIZE; ++r)
	{
		for (uint8_t f = 0; f < fenrir::BOARD_SIZE; ++f)
		{
			const fenrir::Piece *p = board.getPiece(r, f);
			if (p)
			{
				EXPECT_LT(p->getRank(), fenrir::BOARD_SIZE);
				EXPECT_LT(p->getFile(), fenrir::BOARD_SIZE);
			}
		}
	}
}

/* FEN generation tests */
TEST_F(BoardTest, GetFenAfterCreation)
{
	fenrir::Board board(valid_fen_position);

	EXPECT_EQ(board.getFen(), valid_fen_position);
}

TEST_F(BoardTest, GetFenAfterMove)
{
	fenrir::Board board("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
	fenrir::Piece *black_pawn = board.getPiece(6, 3);
	board.move(black_pawn, 4, 3);
	EXPECT_EQ(board.getFen(), "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR b KQkq d6 0 1");
}

TEST_F(BoardTest, GetFenEmptyBoard)
{
	fenrir::Board board(empty_fen_position);

	EXPECT_EQ(board.getFen(), empty_fen_position);
}

TEST_F(BoardTest, GetFenWithNoCastling)
{
	std::string fen_no_castling = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1";
	fenrir::Board board(fen_no_castling);

	EXPECT_EQ(board.getFen(), fen_no_castling);
}

TEST_F(BoardTest, GetFenWithPartialCastling)
{
	std::string fen_partial_castling = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kq - 0 1";
	fenrir::Board board(fen_partial_castling);

	EXPECT_EQ(board.getFen(), fen_partial_castling);
}

TEST_F(BoardTest, GetFenAfterPawnDoubleMove)
{
	fenrir::Board board(valid_fen_position);
	fenrir::Piece *pawn = board.getPiece(1, 0);
	board.move(pawn, 3, 0);
	EXPECT_EQ(board.getFen(), "rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR w KQkq a3 0 1");
}
