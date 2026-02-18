/*
 *   Copyright (c) 2026 Juan Minor

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
#include <map>
#include <memory>
#include "include/chess/moves.h"
#include "include/chess/move.h"
#include "include/chess/board.h"
#include "include/chess/piece.h"
#include "include/core/core.h"

class MockBoard : public fenrir::AbstractBoard
{
public:
	std::map<std::pair<uint8_t, uint8_t>, std::unique_ptr<fenrir::Piece>> pieces;
	std::string enPassant;

	MockBoard() : enPassant("") {}

	fenrir::Piece *getPiece(const uint8_t &rank, const uint8_t &file) const override
	{
		auto it = pieces.find({rank, file});
		if (it != pieces.end())
		{
			return it->second.get();
		}
		return nullptr;
	}

	const std::string &getEnPassant(void) const override
	{
		return enPassant;
	}

	fenrir::Piece *addPiece(uint8_t rank, uint8_t file, char alias)
	{
		pieces[{rank, file}] = std::make_unique<fenrir::Piece>(alias, rank, file);
		return pieces[{rank, file}].get();
	}

	void setEnPassant(const std::string &ep)
	{
		enPassant = ep;
	}
};

class MovesTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		standard_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	}

	static void TearDownTestSuite() {}

	static std::string standard_position;

	bool moveExists(const std::vector<fenrir::Move> &moves,
					const std::string &from, const std::string &to) const
	{
		for (const auto &move : moves)
		{
			if (move.getFrom() == from && move.getTo() == to)
			{
				return true;
			}
		}
		return false;
	}
};

std::string MovesTest::standard_position = "";

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White pawn at e3 (2, 4)
	const fenrir::Piece *pawn = board.addPiece(2, 4, 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_FALSE(moves.empty());
	EXPECT_TRUE(moveExists(moves, "e3", "e4"));
}

TEST_F(MovesTest, BlackPawnSingleMove)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// Black pawn at d4 (3, 3)
	const fenrir::Piece *pawn = board.addPiece(3, 3, 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_FALSE(moves.empty());
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
}

TEST_F(MovesTest, WhitePawnDoubleMove)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White pawn at e2 (1, 4) - hasn't moved
	const fenrir::Piece *pawn = board.addPiece(1, 4, 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_EQ(moves.size(), 2);
	EXPECT_TRUE(moveExists(moves, "e2", "e3"));
	EXPECT_TRUE(moveExists(moves, "e2", "e4"));
}

TEST_F(MovesTest, BlackPawnDoubleMove)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// Black pawn at e7 (6, 4) - hasn't moved
	const fenrir::Piece *pawn = board.addPiece(6, 4, 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_EQ(moves.size(), 2);
	EXPECT_TRUE(moveExists(moves, "e7", "e6"));
	EXPECT_TRUE(moveExists(moves, "e7", "e5"));
}

TEST_F(MovesTest, PawnCapture)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White pawn at c4 (3, 2), Black pawn at d5 (4, 3)
	const fenrir::Piece *pawn = board.addPiece(3, 2, 'P');
	board.addPiece(4, 3, 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_TRUE(moveExists(moves, "c4", "d5"));
}

TEST_F(MovesTest, EnPassantCapture)
{
	MockBoard board;
	board.setEnPassant("a6");
	std::vector<fenrir::Move> moves;
	// White pawn at b5 (4, 1), Black pawn at a5 (4, 0) - just moved
	const fenrir::Piece *pawn = board.addPiece(4, 1, 'P');
	board.addPiece(4, 0, 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_TRUE(moveExists(moves, "b5", "a6"));
}

TEST_F(MovesTest, BlockedPawn)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White pawn at d4 (3, 3), blocked by Black pawn at d5 (4, 3)
	const fenrir::Piece *pawn = board.addPiece(3, 3, 'P');
	board.addPiece(4, 3, 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_FALSE(moveExists(moves, "d4", "d5"));
}

TEST_F(MovesTest, PawnAtEdge)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White pawn at a8 (7, 0) - end of board
	const fenrir::Piece *pawn = board.addPiece(7, 0, 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_TRUE(moves.empty());
}

/* Edge cases */
TEST_F(MovesTest, NullPiece)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;

	fenrir::Moves::getInstance().generateMoves(nullptr, board, moves);

	EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, EmptySquare)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;

	const fenrir::Piece *empty = board.getPiece(3, 4); // Should be null

	fenrir::Moves::getInstance().generateMoves(empty, board, moves);

	EXPECT_TRUE(moves.empty());
}

/* Utility test */
TEST_F(MovesTest, LogGeneratedMoves)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// Just need a piece to log
	const fenrir::Piece *pawn = board.addPiece(1, 4, 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	SUCCEED();
}

/* Test non-pawn piece to cover default case */
TEST_F(MovesTest, NonPawnPiece)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Knight at b1 (0, 1)
	const fenrir::Piece *knight = board.addPiece(0, 1, 'N');
	board.addPiece(1, 3, 'P'); // Block d2 (1, 3) to simulate start position

	fenrir::Moves::getInstance().generateMoves(knight, board, moves);

	// Knight on b1 should have 2 moves: a3 and c3
	EXPECT_EQ(moves.size(), 2);
	EXPECT_TRUE(moveExists(moves, "b1", "a3"));
	EXPECT_TRUE(moveExists(moves, "b1", "c3"));
}

/* Test with rook piece to ensure default case coverage */
TEST_F(MovesTest, RookPiece)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Rook at a1 (0, 0) blocked by neighbors (simulating start position)
	const fenrir::Piece *rook = board.addPiece(0, 0, 'R');
	board.addPiece(1, 0, 'P'); // Blocked by a2
	board.addPiece(0, 1, 'N'); // Blocked by b1

	fenrir::Moves::getInstance().generateMoves(rook, board, moves);

	EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, RookVerticalMovement)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Rook at d4 (3, 3)
	const fenrir::Piece *rook = board.addPiece(3, 3, 'R');

	fenrir::Moves::getInstance().generateMoves(rook, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Rook at d4 (3, 3)
	const fenrir::Piece *rook = board.addPiece(3, 3, 'R');

	fenrir::Moves::getInstance().generateMoves(rook, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Rook at d4 (3, 3), blocked by White Pawn at d5 (4, 3)
	const fenrir::Piece *rook = board.addPiece(3, 3, 'R');
	board.addPiece(4, 3, 'P');

	fenrir::Moves::getInstance().generateMoves(rook, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Rook at d4 (3, 3), enemy Black Pawn at d5 (4, 3)
	const fenrir::Piece *rook = board.addPiece(3, 3, 'R');
	board.addPiece(4, 3, 'p');

	fenrir::Moves::getInstance().generateMoves(rook, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Rook at d4 (3, 3), blocked by White Pawn at d5 (4, 3), enemy at c5 (4, 2), enemy at e5 (4, 4)
	const fenrir::Piece *rook = board.addPiece(3, 3, 'R');
	board.addPiece(4, 3, 'P');

	fenrir::Moves::getInstance().generateMoves(rook, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Rook at a8 (7, 0)
	const fenrir::Piece *rook = board.addPiece(7, 0, 'R');

	fenrir::Moves::getInstance().generateMoves(rook, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Rook at a4 (3, 0)
	const fenrir::Piece *rook = board.addPiece(3, 0, 'R');

	fenrir::Moves::getInstance().generateMoves(rook, board, moves);

	EXPECT_EQ(moves.size(), 14);
	EXPECT_TRUE(moveExists(moves, "a4", "a8"));
	EXPECT_TRUE(moveExists(moves, "a4", "a1"));
	EXPECT_TRUE(moveExists(moves, "a4", "h4"));
}

TEST_F(MovesTest, BlackRookMovement)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// Black Rook at d4 (3, 3)
	const fenrir::Piece *rook = board.addPiece(3, 3, 'r');

	fenrir::Moves::getInstance().generateMoves(rook, board, moves);

	EXPECT_EQ(moves.size(), 14);
	EXPECT_TRUE(moveExists(moves, "d4", "d1"));
	EXPECT_TRUE(moveExists(moves, "d4", "d8"));
	EXPECT_TRUE(moveExists(moves, "d4", "a4"));
	EXPECT_TRUE(moveExists(moves, "d4", "h4"));
}

/* Bishop movement tests */
TEST_F(MovesTest, BishopDiagonalMovement)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Bishop at d4 (3, 3)
	const fenrir::Piece *bishop = board.addPiece(3, 3, 'B');

	fenrir::Moves::getInstance().generateMoves(bishop, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Bishop at d4 (3, 3), blocked by White Pawns at c5 (4, 2) and e5 (4, 4)
	const fenrir::Piece *bishop = board.addPiece(3, 3, 'B');
	board.addPiece(4, 2, 'P');
	board.addPiece(4, 4, 'P');

	fenrir::Moves::getInstance().generateMoves(bishop, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Bishop at d4 (3, 3), enemy Black Pawns at c5 (4, 2) and e5 (4, 4)
	const fenrir::Piece *bishop = board.addPiece(3, 3, 'B');
	board.addPiece(4, 2, 'p');
	board.addPiece(4, 4, 'p');

	fenrir::Moves::getInstance().generateMoves(bishop, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Bishop at a8 (7, 0)
	const fenrir::Piece *bishop = board.addPiece(7, 0, 'B');

	fenrir::Moves::getInstance().generateMoves(bishop, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Bishop at a4 (3, 0)
	const fenrir::Piece *bishop = board.addPiece(3, 0, 'B');

	fenrir::Moves::getInstance().generateMoves(bishop, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// Black Bishop at d4 (3, 3)
	const fenrir::Piece *bishop = board.addPiece(3, 3, 'b');

	fenrir::Moves::getInstance().generateMoves(bishop, board, moves);

	EXPECT_EQ(moves.size(), 13);
	EXPECT_TRUE(moveExists(moves, "d4", "a1"));
	EXPECT_TRUE(moveExists(moves, "d4", "h8"));
	EXPECT_TRUE(moveExists(moves, "d4", "g1"));
	EXPECT_TRUE(moveExists(moves, "d4", "a7"));
}

/* Queen movement tests */
TEST_F(MovesTest, QueenCombinedMovement)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Queen at d4 (3, 3)
	const fenrir::Piece *queen = board.addPiece(3, 3, 'Q');

	fenrir::Moves::getInstance().generateMoves(queen, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Queen at d4 (3, 3), blocked by friendly pieces all around
	const fenrir::Piece *queen = board.addPiece(3, 3, 'Q');
	board.addPiece(4, 2, 'P');
	board.addPiece(4, 3, 'P');
	board.addPiece(4, 4, 'P');
	board.addPiece(3, 2, 'P');
	board.addPiece(3, 4, 'P');
	board.addPiece(2, 2, 'P');
	board.addPiece(2, 3, 'P');
	board.addPiece(2, 4, 'P');

	fenrir::Moves::getInstance().generateMoves(queen, board, moves);

	EXPECT_TRUE(moves.empty());
	EXPECT_EQ(moves.size(), 0);
}

TEST_F(MovesTest, QueenCaptureEnemyPieces)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Queen at d4 (3, 3), surrounded by enemy pieces
	const fenrir::Piece *queen = board.addPiece(3, 3, 'Q');
	board.addPiece(4, 2, 'p');
	board.addPiece(4, 3, 'p');
	board.addPiece(4, 4, 'p');
	board.addPiece(3, 2, 'p');
	board.addPiece(3, 4, 'p');
	board.addPiece(2, 2, 'p');
	board.addPiece(2, 3, 'p');
	board.addPiece(2, 4, 'p');

	fenrir::Moves::getInstance().generateMoves(queen, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Queen at a8 (7, 0)
	const fenrir::Piece *queen = board.addPiece(7, 0, 'Q');

	fenrir::Moves::getInstance().generateMoves(queen, board, moves);

	EXPECT_TRUE(moveExists(moves, "a8", "a1"));
	EXPECT_TRUE(moveExists(moves, "a8", "h8"));
	EXPECT_TRUE(moveExists(moves, "a8", "h1"));
	EXPECT_EQ(moves.size(), 21);
}

TEST_F(MovesTest, BlackQueenMovement)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// Black Queen at d4 (3, 3)
	const fenrir::Piece *queen = board.addPiece(3, 3, 'q');

	fenrir::Moves::getInstance().generateMoves(queen, board, moves);

	EXPECT_EQ(moves.size(), 27);
	EXPECT_TRUE(moveExists(moves, "d4", "d1"));
	EXPECT_TRUE(moveExists(moves, "d4", "d8"));
	EXPECT_TRUE(moveExists(moves, "d4", "a4"));
	EXPECT_TRUE(moveExists(moves, "d4", "h4"));
}

TEST_F(MovesTest, BishopPiece)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Bishop at c1 (0, 2) blocked by neighbors
	const fenrir::Piece *bishop = board.addPiece(0, 2, 'B');
	board.addPiece(1, 1, 'P'); // Blocked by b2
	board.addPiece(1, 3, 'P'); // Blocked by d2

	fenrir::Moves::getInstance().generateMoves(bishop, board, moves);

	EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, QueenPiece)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Queen at d1 (0, 3) blocked by neighbors
	const fenrir::Piece *queen = board.addPiece(0, 3, 'Q');
	board.addPiece(0, 2, 'B'); // c1
	board.addPiece(1, 2, 'P'); // c2
	board.addPiece(1, 3, 'P'); // d2
	board.addPiece(1, 4, 'P'); // e2
	board.addPiece(0, 4, 'K'); // e1

	fenrir::Moves::getInstance().generateMoves(queen, board, moves);

	EXPECT_TRUE(moves.empty());
}

/* Knight movement tests */
TEST_F(MovesTest, KnightLShapeMovement)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Knight at d4 (3, 3)
	const fenrir::Piece *knight = board.addPiece(3, 3, 'N');

	fenrir::Moves::getInstance().generateMoves(knight, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Knight at d4 (3, 3), blocked by friendly pieces
	const fenrir::Piece *knight = board.addPiece(3, 3, 'N');
	// Let's just add pieces at all 8 target squares.
	board.addPiece(4, 5, 'P');
	board.addPiece(2, 5, 'P');
	board.addPiece(5, 4, 'P');
	board.addPiece(1, 4, 'P');
	board.addPiece(5, 2, 'P');
	board.addPiece(1, 2, 'P');
	board.addPiece(4, 1, 'P');
	board.addPiece(2, 1, 'P');

	fenrir::Moves::getInstance().generateMoves(knight, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Knight at d4 (3, 3), enemy pieces at all 8 targets
	const fenrir::Piece *knight = board.addPiece(3, 3, 'N');
	board.addPiece(4, 5, 'p');
	board.addPiece(2, 5, 'p');
	board.addPiece(5, 4, 'p');
	board.addPiece(1, 4, 'p');
	board.addPiece(5, 2, 'p');
	board.addPiece(1, 2, 'p');
	board.addPiece(4, 1, 'p');
	board.addPiece(2, 1, 'p');

	fenrir::Moves::getInstance().generateMoves(knight, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Knight at a8 (7, 0)
	const fenrir::Piece *knight = board.addPiece(7, 0, 'N');

	fenrir::Moves::getInstance().generateMoves(knight, board, moves);

	EXPECT_TRUE(moveExists(moves, "a8", "b6"));
	EXPECT_TRUE(moveExists(moves, "a8", "c7"));
	EXPECT_EQ(moves.size(), 2);
}

TEST_F(MovesTest, KnightEdgePosition)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Knight at a4 (3, 0)
	const fenrir::Piece *knight = board.addPiece(3, 0, 'N');

	fenrir::Moves::getInstance().generateMoves(knight, board, moves);

	EXPECT_TRUE(moveExists(moves, "a4", "b6"));
	EXPECT_TRUE(moveExists(moves, "a4", "b2"));
	EXPECT_TRUE(moveExists(moves, "a4", "c5"));
	EXPECT_TRUE(moveExists(moves, "a4", "c3"));
	EXPECT_EQ(moves.size(), 4);
}

TEST_F(MovesTest, KnightNearEdgePosition)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Knight at b4 (3, 1)
	const fenrir::Piece *knight = board.addPiece(3, 1, 'N');

	fenrir::Moves::getInstance().generateMoves(knight, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// Black Knight at d4 (3, 3)
	const fenrir::Piece *knight = board.addPiece(3, 3, 'n');

	fenrir::Moves::getInstance().generateMoves(knight, board, moves);

	EXPECT_EQ(moves.size(), 8);
	EXPECT_TRUE(moveExists(moves, "d4", "f5"));
	EXPECT_TRUE(moveExists(moves, "d4", "f3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e6"));
	EXPECT_TRUE(moveExists(moves, "d4", "e2"));
}

TEST_F(MovesTest, KnightJumpOverPieces)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White Knight at d4 (3, 3), surrounded by blocking pieces immediately adjacent (which it jumps over)
	const fenrir::Piece *knight = board.addPiece(3, 3, 'N');
	// Add pieces directly around it (d5, e4, d3, c4)
	board.addPiece(4, 3, 'p');
	board.addPiece(3, 4, 'p');
	board.addPiece(2, 3, 'p');
	board.addPiece(3, 2, 'p');

	fenrir::Moves::getInstance().generateMoves(knight, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White King at e1 (0, 4) blocked by neighbors
	const fenrir::Piece *king = board.addPiece(0, 4, 'K');
	board.addPiece(0, 3, 'Q'); // d1
	board.addPiece(1, 3, 'P'); // d2
	board.addPiece(1, 4, 'P'); // e2
	board.addPiece(1, 5, 'P'); // f2
	board.addPiece(0, 5, 'B'); // f1

	fenrir::Moves::getInstance().generateMoves(king, board, moves);

	EXPECT_EQ(moves.size(), 0);
}

TEST_F(MovesTest, KingSingleSquareMovement)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White King at d4 (3, 3)
	const fenrir::Piece *king = board.addPiece(3, 3, 'K');

	fenrir::Moves::getInstance().generateMoves(king, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White King at d4 (3, 3), blocked by friendly pieces
	const fenrir::Piece *king = board.addPiece(3, 3, 'K');
	board.addPiece(4, 2, 'P');
	board.addPiece(4, 3, 'P');
	board.addPiece(4, 4, 'P');
	board.addPiece(3, 2, 'P');
	board.addPiece(3, 4, 'P');
	board.addPiece(2, 2, 'P');
	board.addPiece(2, 3, 'P');
	board.addPiece(2, 4, 'P');

	fenrir::Moves::getInstance().generateMoves(king, board, moves);

	EXPECT_EQ(moves.size(), 0);
}

TEST_F(MovesTest, KingCaptureEnemyPieces)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White King at d4 (3, 3), surrounded by enemy pieces
	const fenrir::Piece *king = board.addPiece(3, 3, 'K');
	board.addPiece(4, 2, 'p');
	board.addPiece(4, 3, 'p');
	board.addPiece(4, 4, 'p');
	board.addPiece(3, 2, 'p');
	board.addPiece(3, 4, 'p');
	board.addPiece(2, 2, 'p');
	board.addPiece(2, 3, 'p');
	board.addPiece(2, 4, 'p');

	fenrir::Moves::getInstance().generateMoves(king, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White King at a1 (0, 0)
	const fenrir::Piece *king = board.addPiece(0, 0, 'K');

	fenrir::Moves::getInstance().generateMoves(king, board, moves);

	EXPECT_TRUE(moveExists(moves, "a1", "a2"));
	EXPECT_TRUE(moveExists(moves, "a1", "b1"));
	EXPECT_TRUE(moveExists(moves, "a1", "b2"));

	EXPECT_EQ(moves.size(), 3);
}

TEST_F(MovesTest, KingEdgePosition)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White King at a4 (3, 0)
	const fenrir::Piece *king = board.addPiece(3, 0, 'K');

	fenrir::Moves::getInstance().generateMoves(king, board, moves);

	EXPECT_TRUE(moveExists(moves, "a4", "a3"));
	EXPECT_TRUE(moveExists(moves, "a4", "a5"));
	EXPECT_TRUE(moveExists(moves, "a4", "b3"));
	EXPECT_TRUE(moveExists(moves, "a4", "b4"));
	EXPECT_TRUE(moveExists(moves, "a4", "b5"));

	EXPECT_EQ(moves.size(), 5);
}

TEST_F(MovesTest, BlackKingMovement)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// Black King at d4 (3, 3)
	const fenrir::Piece *king = board.addPiece(3, 3, 'k');

	fenrir::Moves::getInstance().generateMoves(king, board, moves);

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
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White King at d4 (3, 3)
	const fenrir::Piece *king = board.addPiece(3, 3, 'K');
	// Blocked by friendly: c4 (3,2), f4 (3,5), e3 (2,4)
	board.addPiece(3, 2, 'P');
	board.addPiece(3, 5, 'P');
	board.addPiece(2, 4, 'P');
	// Enemy at c5 (4,2) - capture
	board.addPiece(4, 2, 'p');

	fenrir::Moves::getInstance().generateMoves(king, board, moves);

	EXPECT_TRUE(moveExists(moves, "d4", "c3"));
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
	EXPECT_TRUE(moveExists(moves, "d4", "e4"));
	EXPECT_TRUE(moveExists(moves, "d4", "e5"));
	EXPECT_FALSE(moveExists(moves, "d4", "c4"));
	EXPECT_TRUE(moveExists(moves, "d4", "d5"));

	EXPECT_TRUE(moveExists(moves, "d4", "c5"));
	EXPECT_FALSE(moveExists(moves, "d4", "f4"));
	EXPECT_FALSE(moveExists(moves, "d4", "e3"));

	EXPECT_EQ(moves.size(), 6);
}

TEST_F(MovesTest, KingInitialPosition)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	// White King at e1 (0, 4) blocked by neighbors
	const fenrir::Piece *king = board.addPiece(0, 4, 'K');
	board.addPiece(0, 3, 'Q'); // d1
	board.addPiece(1, 3, 'P'); // d2
	board.addPiece(1, 4, 'P'); // e2
	board.addPiece(1, 5, 'P'); // f2
	board.addPiece(0, 5, 'B'); // f1

	fenrir::Moves::getInstance().generateMoves(king, board, moves);

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
			std::vector<fenrir::Move> moves;
			fenrir::Moves::getInstance().generateMoves(pawn, board, moves);
		}
	}
}
