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
#include "include/chess/moves.h"
#include "include/chess/board.h"
#include "include/chess/piece.h"
#include "include/core/core.h"

class MovesTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		standard_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		pawn_position = "8/8/8/8/3p4/4P3/8/8 w KQkq - 0 1";
		en_passant_position = "rnbqkbnr/1ppppppp/8/pP6/8/8/P1PPPPPP/RNBQKBNR w KQkq a6 0 1";
	}

	static void TearDownTestSuite() {}

	static std::string standard_position;
	static std::string pawn_position;
	static std::string en_passant_position;

	bool moveExists(const std::vector<std::pair<const std::string, const std::string>> &moves,
					const std::string &from, const std::string &to) const
	{
		for (const auto &move : moves)
		{
			if (move.first == from && move.second == to)
			{
				return true;
			}
		}
		return false;
	}
};

std::string MovesTest::standard_position = "";
std::string MovesTest::pawn_position = "";
std::string MovesTest::en_passant_position = "";

/* Singleton tests */
TEST_F(MovesTest, GetInstance)
{
	fenrir::Moves &moves1 = fenrir::Moves::getInstance();
	fenrir::Moves &moves2 = fenrir::Moves::getInstance();

	EXPECT_EQ(&moves1, &moves2);
}

/* Pawn movement tests */
TEST_F(MovesTest, WhitePawnSingleMove)
{
	fenrir::Board board(pawn_position);
	std::vector<std::pair<const std::string, const std::string>> moves;
	const fenrir::Piece *pawn = board.getPiece(2, 4);

	ASSERT_NE(pawn, nullptr);
	EXPECT_EQ(pawn->getAlias(), 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, &board, moves);

	EXPECT_FALSE(moves.empty());
	EXPECT_TRUE(moveExists(moves, "e3", "e4"));
}

TEST_F(MovesTest, BlackPawnSingleMove)
{
	fenrir::Board board(pawn_position);
	std::vector<std::pair<const std::string, const std::string>> moves;
	const fenrir::Piece *pawn = board.getPiece(3, 3);

	ASSERT_NE(pawn, nullptr);
	EXPECT_EQ(pawn->getAlias(), 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, &board, moves);

	EXPECT_FALSE(moves.empty());
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
}

TEST_F(MovesTest, WhitePawnDoubleMove)
{
	fenrir::Board board(standard_position);
	std::vector<std::pair<const std::string, const std::string>> moves;
	const fenrir::Piece *pawn = board.getPiece(1, 4);

	ASSERT_NE(pawn, nullptr);
	EXPECT_EQ(pawn->getAlias(), 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, &board, moves);

	EXPECT_EQ(moves.size(), 2);
	EXPECT_TRUE(moveExists(moves, "e2", "e3"));
	EXPECT_TRUE(moveExists(moves, "e2", "e4"));
}

TEST_F(MovesTest, BlackPawnDoubleMove)
{
	fenrir::Board board(standard_position);
	std::vector<std::pair<const std::string, const std::string>> moves;
	const fenrir::Piece *pawn = board.getPiece(6, 4);

	ASSERT_NE(pawn, nullptr);
	EXPECT_EQ(pawn->getAlias(), 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, &board, moves);

	EXPECT_EQ(moves.size(), 2);
	EXPECT_TRUE(moveExists(moves, "e7", "e6"));
	EXPECT_TRUE(moveExists(moves, "e7", "e5"));
}

TEST_F(MovesTest, PawnCapture)
{

	fenrir::Board board("8/8/8/3p4/2P5/8/8/8 w KQkq - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;
	const fenrir::Piece *pawn = board.getPiece(3, 2);

	ASSERT_NE(pawn, nullptr);
	EXPECT_EQ(pawn->getAlias(), 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, &board, moves);

	EXPECT_TRUE(moveExists(moves, "c4", "d5"));
}

TEST_F(MovesTest, EnPassantCapture)
{
	fenrir::Board board(en_passant_position);
	EXPECT_EQ(board.getEnPassant(), "a6");

	std::vector<std::pair<const std::string, const std::string>> moves;
	const fenrir::Piece *pawn = board.getPiece(4, 1);

	ASSERT_NE(pawn, nullptr);
	EXPECT_EQ(pawn->getAlias(), 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, &board, moves);

	EXPECT_TRUE(moveExists(moves, "b5", "a6"));
}

TEST_F(MovesTest, BlockedPawn)
{

	fenrir::Board board("8/8/8/3p4/3P4/8/8/8 w KQkq - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *pawn = board.getPiece(3, 3);

	ASSERT_NE(pawn, nullptr);
	EXPECT_EQ(pawn->getAlias(), 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, &board, moves);

	EXPECT_FALSE(moveExists(moves, "d4", "d5"));
}

TEST_F(MovesTest, PawnAtEdge)
{

	fenrir::Board board("P7/8/8/8/8/8/8/8 w KQkq - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *pawn = board.getPiece(7, 0);

	ASSERT_NE(pawn, nullptr);
	EXPECT_EQ(pawn->getAlias(), 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, &board, moves);

	EXPECT_TRUE(moves.empty());
}

/* Edge cases */
TEST_F(MovesTest, NullPiece)
{
	fenrir::Board board(standard_position);
	std::vector<std::pair<const std::string, const std::string>> moves;

	fenrir::Moves::getInstance().generateMoves(nullptr, &board, moves);

	EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, EmptySquare)
{
	fenrir::Board board(standard_position);
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *empty = board.getPiece(3, 4);

	ASSERT_EQ(empty, nullptr);

	fenrir::Moves::getInstance().generateMoves(empty, &board, moves);

	EXPECT_TRUE(moves.empty());
}

/* Utility test */
TEST_F(MovesTest, LogGeneratedMoves)
{

	fenrir::Board board(standard_position);
	std::vector<std::pair<const std::string, const std::string>> moves;
	const fenrir::Piece *pawn = board.getPiece(1, 4);

	ASSERT_NE(pawn, nullptr);

	fenrir::Moves::getInstance().generateMoves(pawn, &board, moves);

	SUCCEED();
}

/* Test non-pawn piece to cover default case */
TEST_F(MovesTest, NonPawnPiece)
{
	fenrir::Board board(standard_position);
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *knight = board.getPiece(0, 1);

	ASSERT_NE(knight, nullptr);
	ASSERT_EQ(std::tolower(knight->getAlias()), 'n');

	fenrir::Moves::getInstance().generateMoves(knight, &board, moves);

	// Knight on b1 should have 2 moves: a3 and c3
	EXPECT_EQ(moves.size(), 2);
	EXPECT_TRUE(moveExists(moves, "b1", "a3"));
	EXPECT_TRUE(moveExists(moves, "b1", "c3"));
}

/* Test with rook piece to ensure default case coverage */
TEST_F(MovesTest, RookPiece)
{
	fenrir::Board board(standard_position);
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *rook = board.getPiece(0, 0);

	ASSERT_NE(rook, nullptr);
	ASSERT_EQ(std::tolower(rook->getAlias()), 'r');

	fenrir::Moves::getInstance().generateMoves(rook, &board, moves);

	EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, RookVerticalMovement)
{
	fenrir::Board board("8/8/8/8/3R4/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *rook = board.getPiece(3, 3);

	ASSERT_NE(rook, nullptr);
	ASSERT_EQ(std::tolower(rook->getAlias()), 'r');

	fenrir::Moves::getInstance().generateMoves(rook, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "d1"));
	EXPECT_TRUE(moveExists(moves, "d4", "d2"));
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
	EXPECT_TRUE(moveExists(moves, "d4", "d5"));
	EXPECT_TRUE(moveExists(moves, "d4", "d6"));
	EXPECT_TRUE(moveExists(moves, "d4", "d7"));
	EXPECT_TRUE(moveExists(moves, "d4", "d8"));
	EXPECT_EQ(moves.size(), 14);
}

TEST_F(MovesTest, RookHorizontalMovement)
{
	fenrir::Board board("8/8/8/8/3R4/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *rook = board.getPiece(3, 3);

	ASSERT_NE(rook, nullptr);
	ASSERT_EQ(std::tolower(rook->getAlias()), 'r');

	fenrir::Moves::getInstance().generateMoves(rook, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "a4"));
	EXPECT_TRUE(moveExists(moves, "d4", "b4"));
	EXPECT_TRUE(moveExists(moves, "d4", "c4"));
	EXPECT_TRUE(moveExists(moves, "d4", "e4"));
	EXPECT_TRUE(moveExists(moves, "d4", "f4"));
	EXPECT_TRUE(moveExists(moves, "d4", "g4"));
	EXPECT_TRUE(moveExists(moves, "d4", "h4"));
	EXPECT_EQ(moves.size(), 14);
}

TEST_F(MovesTest, RookBlockedByFriendlyPiece)
{
	fenrir::Board board("8/8/8/3P4/3R4/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *rook = board.getPiece(3, 3);

	ASSERT_NE(rook, nullptr);
	ASSERT_EQ(std::tolower(rook->getAlias()), 'r');

	fenrir::Moves::getInstance().generateMoves(rook, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "d1"));
	EXPECT_TRUE(moveExists(moves, "d4", "d2"));
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
	EXPECT_FALSE(moveExists(moves, "d4", "d5"));
	EXPECT_FALSE(moveExists(moves, "d4", "d6"));
	EXPECT_FALSE(moveExists(moves, "d4", "d7"));
	EXPECT_FALSE(moveExists(moves, "d4", "d8"));
	EXPECT_EQ(moves.size(), 10);
}

TEST_F(MovesTest, RookCaptureEnemyPiece)
{
	fenrir::Board board("8/8/8/3p4/3R4/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *rook = board.getPiece(3, 3);

	ASSERT_NE(rook, nullptr);
	ASSERT_EQ(std::tolower(rook->getAlias()), 'r');

	fenrir::Moves::getInstance().generateMoves(rook, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "d1"));
	EXPECT_TRUE(moveExists(moves, "d4", "d2"));
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
	EXPECT_TRUE(moveExists(moves, "d4", "d5"));
	EXPECT_FALSE(moveExists(moves, "d4", "d6"));
	EXPECT_FALSE(moveExists(moves, "d4", "d7"));
	EXPECT_FALSE(moveExists(moves, "d4", "d8"));
	EXPECT_EQ(moves.size(), 11);
}

TEST_F(MovesTest, RookMultipleDirectionBlocking)
{
	fenrir::Board board("8/8/8/2pPp3/3R4/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *rook = board.getPiece(3, 3);

	ASSERT_NE(rook, nullptr);
	ASSERT_EQ(std::tolower(rook->getAlias()), 'r');

	fenrir::Moves::getInstance().generateMoves(rook, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "d1"));
	EXPECT_TRUE(moveExists(moves, "d4", "d2"));
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
	EXPECT_TRUE(moveExists(moves, "d4", "a4"));
	EXPECT_TRUE(moveExists(moves, "d4", "b4"));
	EXPECT_TRUE(moveExists(moves, "d4", "c4"));
	EXPECT_TRUE(moveExists(moves, "d4", "e4"));
	EXPECT_TRUE(moveExists(moves, "d4", "f4"));
	EXPECT_TRUE(moveExists(moves, "d4", "g4"));
	EXPECT_TRUE(moveExists(moves, "d4", "h4"));
	EXPECT_FALSE(moveExists(moves, "d4", "d5"));
	EXPECT_EQ(moves.size(), 10);
}

TEST_F(MovesTest, RookCornerPosition)
{
	fenrir::Board board("R7/8/8/8/8/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *rook = board.getPiece(7, 0);

	ASSERT_NE(rook, nullptr);
	ASSERT_EQ(std::tolower(rook->getAlias()), 'r');

	fenrir::Moves::getInstance().generateMoves(rook, &board, moves);

	EXPECT_TRUE(moveExists(moves, "a8", "a7"));
	EXPECT_TRUE(moveExists(moves, "a8", "a6"));
	EXPECT_TRUE(moveExists(moves, "a8", "a1"));
	EXPECT_TRUE(moveExists(moves, "a8", "b8"));
	EXPECT_TRUE(moveExists(moves, "a8", "c8"));
	EXPECT_TRUE(moveExists(moves, "a8", "h8"));
	EXPECT_EQ(moves.size(), 14);
}

TEST_F(MovesTest, RookEdgePosition)
{
	fenrir::Board board("8/8/8/8/R7/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *rook = board.getPiece(3, 0);

	ASSERT_NE(rook, nullptr);
	ASSERT_EQ(std::tolower(rook->getAlias()), 'r');

	fenrir::Moves::getInstance().generateMoves(rook, &board, moves);

	EXPECT_EQ(moves.size(), 14);
	EXPECT_TRUE(moveExists(moves, "a4", "a8"));
	EXPECT_TRUE(moveExists(moves, "a4", "a1"));
	EXPECT_TRUE(moveExists(moves, "a4", "h4"));
}

TEST_F(MovesTest, BlackRookMovement)
{
	fenrir::Board board("8/8/8/8/3r4/8/8/8 b - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *rook = board.getPiece(3, 3);

	ASSERT_NE(rook, nullptr);
	ASSERT_EQ(std::tolower(rook->getAlias()), 'r');

	fenrir::Moves::getInstance().generateMoves(rook, &board, moves);

	EXPECT_EQ(moves.size(), 14);
	EXPECT_TRUE(moveExists(moves, "d4", "d1"));
	EXPECT_TRUE(moveExists(moves, "d4", "d8"));
	EXPECT_TRUE(moveExists(moves, "d4", "a4"));
	EXPECT_TRUE(moveExists(moves, "d4", "h4"));
}

/* Bishop movement tests */
TEST_F(MovesTest, BishopDiagonalMovement)
{
	fenrir::Board board("8/8/8/8/3B4/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *bishop = board.getPiece(3, 3);

	ASSERT_NE(bishop, nullptr);
	ASSERT_EQ(std::tolower(bishop->getAlias()), 'b');

	fenrir::Moves::getInstance().generateMoves(bishop, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "a1"));
	EXPECT_TRUE(moveExists(moves, "d4", "b2"));
	EXPECT_TRUE(moveExists(moves, "d4", "c3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e5"));
	EXPECT_TRUE(moveExists(moves, "d4", "f6"));
	EXPECT_TRUE(moveExists(moves, "d4", "g7"));
	EXPECT_TRUE(moveExists(moves, "d4", "h8"));
	EXPECT_TRUE(moveExists(moves, "d4", "a7"));
	EXPECT_TRUE(moveExists(moves, "d4", "b6"));
	EXPECT_TRUE(moveExists(moves, "d4", "c5"));
	EXPECT_TRUE(moveExists(moves, "d4", "e3"));
	EXPECT_TRUE(moveExists(moves, "d4", "f2"));
	EXPECT_TRUE(moveExists(moves, "d4", "g1"));
	EXPECT_EQ(moves.size(), 13);
}

TEST_F(MovesTest, BishopBlockedByFriendlyPiece)
{
	fenrir::Board board("8/8/8/2P1P3/3B4/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *bishop = board.getPiece(3, 3);

	ASSERT_NE(bishop, nullptr);
	ASSERT_EQ(std::tolower(bishop->getAlias()), 'b');

	fenrir::Moves::getInstance().generateMoves(bishop, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "a1"));
	EXPECT_TRUE(moveExists(moves, "d4", "b2"));
	EXPECT_TRUE(moveExists(moves, "d4", "c3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e3"));
	EXPECT_TRUE(moveExists(moves, "d4", "f2"));
	EXPECT_TRUE(moveExists(moves, "d4", "g1"));
	EXPECT_FALSE(moveExists(moves, "d4", "c5"));
	EXPECT_FALSE(moveExists(moves, "d4", "e5"));
	EXPECT_EQ(moves.size(), 6);
}

TEST_F(MovesTest, BishopCaptureEnemyPiece)
{
	fenrir::Board board("8/8/8/2p1p3/3B4/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *bishop = board.getPiece(3, 3);

	ASSERT_NE(bishop, nullptr);
	ASSERT_EQ(std::tolower(bishop->getAlias()), 'b');

	fenrir::Moves::getInstance().generateMoves(bishop, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "a1"));
	EXPECT_TRUE(moveExists(moves, "d4", "b2"));
	EXPECT_TRUE(moveExists(moves, "d4", "c3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e3"));
	EXPECT_TRUE(moveExists(moves, "d4", "f2"));
	EXPECT_TRUE(moveExists(moves, "d4", "g1"));
	EXPECT_TRUE(moveExists(moves, "d4", "c5"));
	EXPECT_TRUE(moveExists(moves, "d4", "e5"));
	EXPECT_FALSE(moveExists(moves, "d4", "b6"));
	EXPECT_FALSE(moveExists(moves, "d4", "f6"));
	EXPECT_EQ(moves.size(), 8);
}

TEST_F(MovesTest, BishopCornerPosition)
{
	fenrir::Board board("B7/8/8/8/8/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *bishop = board.getPiece(7, 0);

	ASSERT_NE(bishop, nullptr);
	ASSERT_EQ(std::tolower(bishop->getAlias()), 'b');

	fenrir::Moves::getInstance().generateMoves(bishop, &board, moves);

	EXPECT_TRUE(moveExists(moves, "a8", "b7"));
	EXPECT_TRUE(moveExists(moves, "a8", "c6"));
	EXPECT_TRUE(moveExists(moves, "a8", "d5"));
	EXPECT_TRUE(moveExists(moves, "a8", "e4"));
	EXPECT_TRUE(moveExists(moves, "a8", "f3"));
	EXPECT_TRUE(moveExists(moves, "a8", "g2"));
	EXPECT_TRUE(moveExists(moves, "a8", "h1"));
	EXPECT_EQ(moves.size(), 7);
}

TEST_F(MovesTest, BishopEdgePosition)
{
	fenrir::Board board("8/8/8/8/B7/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *bishop = board.getPiece(3, 0);

	ASSERT_NE(bishop, nullptr);
	ASSERT_EQ(std::tolower(bishop->getAlias()), 'b');

	fenrir::Moves::getInstance().generateMoves(bishop, &board, moves);

	EXPECT_TRUE(moveExists(moves, "a4", "b5"));
	EXPECT_TRUE(moveExists(moves, "a4", "c6"));
	EXPECT_TRUE(moveExists(moves, "a4", "d7"));
	EXPECT_TRUE(moveExists(moves, "a4", "e8"));
	EXPECT_TRUE(moveExists(moves, "a4", "b3"));
	EXPECT_TRUE(moveExists(moves, "a4", "c2"));
	EXPECT_TRUE(moveExists(moves, "a4", "d1"));
	EXPECT_EQ(moves.size(), 7);
}

TEST_F(MovesTest, BlackBishopMovement)
{
	fenrir::Board board("8/8/8/8/3b4/8/8/8 b - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *bishop = board.getPiece(3, 3);

	ASSERT_NE(bishop, nullptr);
	ASSERT_EQ(std::tolower(bishop->getAlias()), 'b');

	fenrir::Moves::getInstance().generateMoves(bishop, &board, moves);

	EXPECT_EQ(moves.size(), 13);
	EXPECT_TRUE(moveExists(moves, "d4", "a1"));
	EXPECT_TRUE(moveExists(moves, "d4", "h8"));
	EXPECT_TRUE(moveExists(moves, "d4", "g1"));
	EXPECT_TRUE(moveExists(moves, "d4", "a7"));
}

/* Queen movement tests */
TEST_F(MovesTest, QueenCombinedMovement)
{
	fenrir::Board board("8/8/8/8/3Q4/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *queen = board.getPiece(3, 3);

	ASSERT_NE(queen, nullptr);
	ASSERT_EQ(std::tolower(queen->getAlias()), 'q');

	fenrir::Moves::getInstance().generateMoves(queen, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "d1"));
	EXPECT_TRUE(moveExists(moves, "d4", "d8"));
	EXPECT_TRUE(moveExists(moves, "d4", "a4"));
	EXPECT_TRUE(moveExists(moves, "d4", "h4"));
	EXPECT_TRUE(moveExists(moves, "d4", "a1"));
	EXPECT_TRUE(moveExists(moves, "d4", "h8"));
	EXPECT_TRUE(moveExists(moves, "d4", "g1"));
	EXPECT_TRUE(moveExists(moves, "d4", "a7"));
	EXPECT_EQ(moves.size(), 27);
}

TEST_F(MovesTest, QueenBlockedByFriendlyPieces)
{
	fenrir::Board board("8/8/8/2PPP3/2PQP3/2PPP3/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *queen = board.getPiece(3, 3);

	ASSERT_NE(queen, nullptr);
	ASSERT_EQ(std::tolower(queen->getAlias()), 'q');

	fenrir::Moves::getInstance().generateMoves(queen, &board, moves);

	EXPECT_TRUE(moves.empty());
	EXPECT_EQ(moves.size(), 0);
}

TEST_F(MovesTest, QueenCaptureEnemyPieces)
{
	fenrir::Board board("8/8/8/2ppp3/2pQp3/2ppp3/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *queen = board.getPiece(3, 3);

	ASSERT_NE(queen, nullptr);
	ASSERT_EQ(std::tolower(queen->getAlias()), 'q');

	fenrir::Moves::getInstance().generateMoves(queen, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "c3"));
	EXPECT_TRUE(moveExists(moves, "d4", "c4"));
	EXPECT_TRUE(moveExists(moves, "d4", "c5"));
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
	EXPECT_TRUE(moveExists(moves, "d4", "d5"));
	EXPECT_TRUE(moveExists(moves, "d4", "e3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e4"));
	EXPECT_TRUE(moveExists(moves, "d4", "e5"));
	EXPECT_EQ(moves.size(), 8);
}

TEST_F(MovesTest, QueenCornerPosition)
{
	fenrir::Board board("Q7/8/8/8/8/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *queen = board.getPiece(7, 0);

	ASSERT_NE(queen, nullptr);
	ASSERT_EQ(std::tolower(queen->getAlias()), 'q');

	fenrir::Moves::getInstance().generateMoves(queen, &board, moves);

	EXPECT_TRUE(moveExists(moves, "a8", "a1"));
	EXPECT_TRUE(moveExists(moves, "a8", "h8"));
	EXPECT_TRUE(moveExists(moves, "a8", "h1"));
	EXPECT_EQ(moves.size(), 21);
}

TEST_F(MovesTest, BlackQueenMovement)
{
	fenrir::Board board("8/8/8/8/3q4/8/8/8 b - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *queen = board.getPiece(3, 3);

	ASSERT_NE(queen, nullptr);
	ASSERT_EQ(std::tolower(queen->getAlias()), 'q');

	fenrir::Moves::getInstance().generateMoves(queen, &board, moves);

	EXPECT_EQ(moves.size(), 27);
	EXPECT_TRUE(moveExists(moves, "d4", "d1"));
	EXPECT_TRUE(moveExists(moves, "d4", "d8"));
	EXPECT_TRUE(moveExists(moves, "d4", "a4"));
	EXPECT_TRUE(moveExists(moves, "d4", "h4"));
}

TEST_F(MovesTest, BishopPiece)
{
	fenrir::Board board(standard_position);
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *bishop = board.getPiece(0, 2);

	ASSERT_NE(bishop, nullptr);
	ASSERT_EQ(std::tolower(bishop->getAlias()), 'b');

	fenrir::Moves::getInstance().generateMoves(bishop, &board, moves);

	EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, QueenPiece)
{
	fenrir::Board board(standard_position);
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *queen = board.getPiece(0, 3);

	ASSERT_NE(queen, nullptr);
	ASSERT_EQ(std::tolower(queen->getAlias()), 'q');

	fenrir::Moves::getInstance().generateMoves(queen, &board, moves);

	EXPECT_TRUE(moves.empty());
}

/* Knight movement tests */
TEST_F(MovesTest, KnightLShapeMovement)
{
	fenrir::Board board("8/8/8/8/3N4/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *knight = board.getPiece(3, 3);

	ASSERT_NE(knight, nullptr);
	ASSERT_EQ(std::tolower(knight->getAlias()), 'n');

	fenrir::Moves::getInstance().generateMoves(knight, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "f5"));
	EXPECT_TRUE(moveExists(moves, "d4", "f3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e6"));
	EXPECT_TRUE(moveExists(moves, "d4", "e2"));
	EXPECT_TRUE(moveExists(moves, "d4", "c6"));
	EXPECT_TRUE(moveExists(moves, "d4", "c2"));
	EXPECT_TRUE(moveExists(moves, "d4", "b5"));
	EXPECT_TRUE(moveExists(moves, "d4", "b3"));
	EXPECT_EQ(moves.size(), 8);
}

TEST_F(MovesTest, KnightBlockedByFriendlyPieces)
{
	fenrir::Board board("8/8/2P1P3/1P3P2/3N4/1P3P2/2P1P3/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *knight = board.getPiece(3, 3);

	ASSERT_NE(knight, nullptr);
	ASSERT_EQ(std::tolower(knight->getAlias()), 'n');

	fenrir::Moves::getInstance().generateMoves(knight, &board, moves);

	EXPECT_FALSE(moveExists(moves, "d4", "f5"));
	EXPECT_FALSE(moveExists(moves, "d4", "f3"));
	EXPECT_FALSE(moveExists(moves, "d4", "e6"));
	EXPECT_FALSE(moveExists(moves, "d4", "e2"));
	EXPECT_FALSE(moveExists(moves, "d4", "c6"));
	EXPECT_FALSE(moveExists(moves, "d4", "c2"));
	EXPECT_FALSE(moveExists(moves, "d4", "b5"));
	EXPECT_FALSE(moveExists(moves, "d4", "b3"));
	EXPECT_EQ(moves.size(), 0);
}

TEST_F(MovesTest, KnightCaptureEnemyPieces)
{
	fenrir::Board board("8/8/2p1p3/1p3p2/3N4/1p3p2/2p1p3/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *knight = board.getPiece(3, 3);

	ASSERT_NE(knight, nullptr);
	ASSERT_EQ(std::tolower(knight->getAlias()), 'n');

	fenrir::Moves::getInstance().generateMoves(knight, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "f5"));
	EXPECT_TRUE(moveExists(moves, "d4", "f3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e6"));
	EXPECT_TRUE(moveExists(moves, "d4", "e2"));
	EXPECT_TRUE(moveExists(moves, "d4", "c6"));
	EXPECT_TRUE(moveExists(moves, "d4", "c2"));
	EXPECT_TRUE(moveExists(moves, "d4", "b5"));
	EXPECT_TRUE(moveExists(moves, "d4", "b3"));
	EXPECT_EQ(moves.size(), 8);
}

TEST_F(MovesTest, KnightCornerPosition)
{
	fenrir::Board board("N7/8/8/8/8/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *knight = board.getPiece(7, 0);

	ASSERT_NE(knight, nullptr);
	ASSERT_EQ(std::tolower(knight->getAlias()), 'n');

	fenrir::Moves::getInstance().generateMoves(knight, &board, moves);

	EXPECT_TRUE(moveExists(moves, "a8", "b6"));
	EXPECT_TRUE(moveExists(moves, "a8", "c7"));
	EXPECT_EQ(moves.size(), 2);
}

TEST_F(MovesTest, KnightEdgePosition)
{
	fenrir::Board board("8/8/8/8/N7/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *knight = board.getPiece(3, 0);

	ASSERT_NE(knight, nullptr);
	ASSERT_EQ(std::tolower(knight->getAlias()), 'n');

	fenrir::Moves::getInstance().generateMoves(knight, &board, moves);

	EXPECT_TRUE(moveExists(moves, "a4", "b6"));
	EXPECT_TRUE(moveExists(moves, "a4", "b2"));
	EXPECT_TRUE(moveExists(moves, "a4", "c5"));
	EXPECT_TRUE(moveExists(moves, "a4", "c3"));
	EXPECT_EQ(moves.size(), 4);
}

TEST_F(MovesTest, KnightNearEdgePosition)
{
	fenrir::Board board("8/8/8/8/1N6/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *knight = board.getPiece(3, 1);

	ASSERT_NE(knight, nullptr);
	ASSERT_EQ(std::tolower(knight->getAlias()), 'n');

	fenrir::Moves::getInstance().generateMoves(knight, &board, moves);

	EXPECT_TRUE(moveExists(moves, "b4", "a6"));
	EXPECT_TRUE(moveExists(moves, "b4", "a2"));
	EXPECT_TRUE(moveExists(moves, "b4", "c6"));
	EXPECT_TRUE(moveExists(moves, "b4", "c2"));
	EXPECT_TRUE(moveExists(moves, "b4", "d5"));
	EXPECT_TRUE(moveExists(moves, "b4", "d3"));
	EXPECT_EQ(moves.size(), 6);
}

TEST_F(MovesTest, BlackKnightMovement)
{
	fenrir::Board board("8/8/8/8/3n4/8/8/8 b - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *knight = board.getPiece(3, 3);

	ASSERT_NE(knight, nullptr);
	ASSERT_EQ(std::tolower(knight->getAlias()), 'n');

	fenrir::Moves::getInstance().generateMoves(knight, &board, moves);

	EXPECT_EQ(moves.size(), 8);
	EXPECT_TRUE(moveExists(moves, "d4", "f5"));
	EXPECT_TRUE(moveExists(moves, "d4", "f3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e6"));
	EXPECT_TRUE(moveExists(moves, "d4", "e2"));
}

TEST_F(MovesTest, KnightJumpOverPieces)
{
	fenrir::Board board("8/8/8/3p4/2pNp3/3p4/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *knight = board.getPiece(3, 3);

	ASSERT_NE(knight, nullptr);
	ASSERT_EQ(std::tolower(knight->getAlias()), 'n');

	fenrir::Moves::getInstance().generateMoves(knight, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "f5"));
	EXPECT_TRUE(moveExists(moves, "d4", "f3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e6"));
	EXPECT_TRUE(moveExists(moves, "d4", "e2"));
	EXPECT_TRUE(moveExists(moves, "d4", "c6"));
	EXPECT_TRUE(moveExists(moves, "d4", "c2"));
	EXPECT_TRUE(moveExists(moves, "d4", "b5"));
	EXPECT_TRUE(moveExists(moves, "d4", "b3"));
	EXPECT_EQ(moves.size(), 8);
}

TEST_F(MovesTest, KingPiece)
{
	fenrir::Board board(standard_position);
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *king = board.getPiece(0, 4);

	ASSERT_NE(king, nullptr);
	ASSERT_EQ(std::tolower(king->getAlias()), 'k');

	fenrir::Moves::getInstance().generateMoves(king, &board, moves);

	EXPECT_EQ(moves.size(), 0);
}

TEST_F(MovesTest, KingSingleSquareMovement)
{
	fenrir::Board board("8/8/8/8/3K4/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *king = board.getPiece(3, 3);

	ASSERT_NE(king, nullptr);
	ASSERT_EQ(std::tolower(king->getAlias()), 'k');

	fenrir::Moves::getInstance().generateMoves(king, &board, moves);

	// All 8 directions
	EXPECT_TRUE(moveExists(moves, "d4", "c3"));
	EXPECT_TRUE(moveExists(moves, "d4", "c4"));
	EXPECT_TRUE(moveExists(moves, "d4", "c5"));
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
	EXPECT_TRUE(moveExists(moves, "d4", "d5"));
	EXPECT_TRUE(moveExists(moves, "d4", "e3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e4"));
	EXPECT_TRUE(moveExists(moves, "d4", "e5"));

	EXPECT_EQ(moves.size(), 8);
}

TEST_F(MovesTest, KingBlockedByFriendlyPieces)
{
	fenrir::Board board("8/8/8/2PPP3/2PKP3/2PPP3/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *king = board.getPiece(3, 3);

	ASSERT_NE(king, nullptr);
	ASSERT_EQ(std::tolower(king->getAlias()), 'k');

	fenrir::Moves::getInstance().generateMoves(king, &board, moves);

	EXPECT_EQ(moves.size(), 0);
}

TEST_F(MovesTest, KingCaptureEnemyPieces)
{
	fenrir::Board board("8/8/8/2ppp3/2pKp3/2ppp3/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *king = board.getPiece(3, 3);

	ASSERT_NE(king, nullptr);
	ASSERT_EQ(std::tolower(king->getAlias()), 'k');

	fenrir::Moves::getInstance().generateMoves(king, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "c3"));
	EXPECT_TRUE(moveExists(moves, "d4", "c4"));
	EXPECT_TRUE(moveExists(moves, "d4", "c5"));
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
	EXPECT_TRUE(moveExists(moves, "d4", "d5"));
	EXPECT_TRUE(moveExists(moves, "d4", "e3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e4"));
	EXPECT_TRUE(moveExists(moves, "d4", "e5"));

	EXPECT_EQ(moves.size(), 8);
}

TEST_F(MovesTest, KingCornerPosition)
{
	fenrir::Board board("8/8/8/8/8/8/8/K7 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *king = board.getPiece(0, 0);

	ASSERT_NE(king, nullptr);
	ASSERT_EQ(std::tolower(king->getAlias()), 'k');

	fenrir::Moves::getInstance().generateMoves(king, &board, moves);

	EXPECT_TRUE(moveExists(moves, "a1", "a2"));
	EXPECT_TRUE(moveExists(moves, "a1", "b1"));
	EXPECT_TRUE(moveExists(moves, "a1", "b2"));

	EXPECT_EQ(moves.size(), 3);
}

TEST_F(MovesTest, KingEdgePosition)
{
	fenrir::Board board("8/8/8/8/K7/8/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *king = board.getPiece(3, 0);

	ASSERT_NE(king, nullptr);
	ASSERT_EQ(std::tolower(king->getAlias()), 'k');

	fenrir::Moves::getInstance().generateMoves(king, &board, moves);

	EXPECT_TRUE(moveExists(moves, "a4", "a3"));
	EXPECT_TRUE(moveExists(moves, "a4", "a5"));
	EXPECT_TRUE(moveExists(moves, "a4", "b3"));
	EXPECT_TRUE(moveExists(moves, "a4", "b4"));
	EXPECT_TRUE(moveExists(moves, "a4", "b5"));

	EXPECT_EQ(moves.size(), 5);
}

TEST_F(MovesTest, BlackKingMovement)
{
	fenrir::Board board("8/8/8/8/3k4/8/8/8 b - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *king = board.getPiece(3, 3);

	ASSERT_NE(king, nullptr);
	ASSERT_EQ(std::tolower(king->getAlias()), 'k');

	fenrir::Moves::getInstance().generateMoves(king, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "c3"));
	EXPECT_TRUE(moveExists(moves, "d4", "c4"));
	EXPECT_TRUE(moveExists(moves, "d4", "c5"));
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
	EXPECT_TRUE(moveExists(moves, "d4", "d5"));
	EXPECT_TRUE(moveExists(moves, "d4", "e3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e4"));
	EXPECT_TRUE(moveExists(moves, "d4", "e5"));

	EXPECT_EQ(moves.size(), 8);
}

TEST_F(MovesTest, KingMixedBlocking)
{
	fenrir::Board board("8/8/8/2Pp4/2pK1P2/4P3/8/8 w - - 0 1");
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *king = board.getPiece(3, 3);

	ASSERT_NE(king, nullptr);
	ASSERT_EQ(std::tolower(king->getAlias()), 'k');

	fenrir::Moves::getInstance().generateMoves(king, &board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "c3"));
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e4"));
	EXPECT_TRUE(moveExists(moves, "d4", "e5"));
	EXPECT_TRUE(moveExists(moves, "d4", "c4"));
	EXPECT_TRUE(moveExists(moves, "d4", "d5"));

	EXPECT_FALSE(moveExists(moves, "d4", "c5"));
	EXPECT_FALSE(moveExists(moves, "d4", "f4"));
	EXPECT_FALSE(moveExists(moves, "d4", "e3"));

	EXPECT_EQ(moves.size(), 6);
}

TEST_F(MovesTest, KingInitialPosition)
{
	fenrir::Board board(standard_position);
	std::vector<std::pair<const std::string, const std::string>> moves;

	const fenrir::Piece *king = board.getPiece(0, 4);

	ASSERT_NE(king, nullptr);
	ASSERT_EQ(std::tolower(king->getAlias()), 'k');

	fenrir::Moves::getInstance().generateMoves(king, &board, moves);

	EXPECT_EQ(moves.size(), 0);
}

/* Stress test */
TEST_F(MovesTest, StressTestGenerateMoves)
{
	if (!test::CI || std::string(test::CI) != "true")
	{
		GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
	}

	fenrir::Board board(standard_position);
	std::vector<std::vector<fenrir::Piece *>> board_state = board.getBoard();

	for (int i = 0; i < 100000; i++)
	{
		const fenrir::Piece *pawn = board_state[1][0];
		if (pawn && pawn->getAlias() == 'P')
		{
			std::vector<std::pair<const std::string, const std::string>> moves;
			fenrir::Moves::getInstance().generateMoves(pawn, &board, moves);
		}
	}
}
