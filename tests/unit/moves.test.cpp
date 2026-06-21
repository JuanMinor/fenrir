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
#include "include/core/core.h"

namespace fenrir {
	class Piece {
	private:
		char alias;
		uint8_t rank;
		uint8_t file;
	public:
		Piece(char a, uint8_t r, uint8_t f) : alias(a), rank(r), file(f) {}
		char getAlias() const { return alias; }
		uint8_t getRank() const { return rank; }
		uint8_t getFile() const { return file; }
		uint8_t getColor() const { return std::isupper(static_cast<unsigned char>(alias)) ? WHITE : BLACK; }
	};

	using RealMoves = Moves;

	class MovesWrapper {
	public:
		static MovesWrapper& getInstance() {
			static MovesWrapper instance;
			return instance;
		}
		void generateMoves(const Piece* piece, const AbstractBoard& board, std::vector<Move>& moves) const {
			if (!piece) {
				logger::ERROR("Piece is null. Moves cannot be generated 😢");
				return;
			}
			RealMoves::getInstance().generateMoves(piece->getRank(), piece->getFile(), board, moves);
		}
	};
}

#define Moves MovesWrapper

class MockBoard : public fenrir::AbstractBoard
{
public:
	std::map<std::pair<uint8_t, uint8_t>, std::unique_ptr<fenrir::Piece>> pieces;
	std::string enPassant;

	MockBoard() : enPassant("") {}

	char getPiece(uint8_t rank, uint8_t file) const override
	{
		auto it = pieces.find({rank, file});
		if (it != pieces.end())
		{
			return it->second->getAlias();
		}
		return '\0';
	}

	fenrir::Piece *getPiecePtr(uint8_t rank, uint8_t file) const
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
	const fenrir::Piece *pawn = board.addPiece(2, 4, 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_FALSE(moves.empty());
	EXPECT_TRUE(moveExists(moves, "e3", "e4"));
}

TEST_F(MovesTest, BlackPawnSingleMove)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	const fenrir::Piece *pawn = board.addPiece(3, 3, 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_FALSE(moves.empty());
	EXPECT_TRUE(moveExists(moves, "d4", "d3"));
}

TEST_F(MovesTest, WhitePawnDoubleMove)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
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
	const fenrir::Piece *pawn = board.addPiece(3, 2, 'P');
	board.addPiece(4, 3, 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_TRUE(moveExists(moves, "c4", "d5"));
}

TEST_F(MovesTest, PawnCaptureLeft)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	const fenrir::Piece *pawn = board.addPiece(3, 2, 'P');
	board.addPiece(4, 1, 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_TRUE(moveExists(moves, "c4", "b5"));
}

TEST_F(MovesTest, EnPassantCapture)
{
	MockBoard board;
	board.setEnPassant("a6");
	std::vector<fenrir::Move> moves;
	const fenrir::Piece *pawn = board.addPiece(4, 1, 'P');
	board.addPiece(4, 0, 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_TRUE(moveExists(moves, "b5", "a6"));
}

TEST_F(MovesTest, BlockedPawn)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	const fenrir::Piece *pawn = board.addPiece(3, 3, 'P');
	board.addPiece(4, 3, 'p');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	EXPECT_FALSE(moveExists(moves, "d4", "d5"));
}

TEST_F(MovesTest, PawnAtEdge)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
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

	const fenrir::Piece *empty = board.getPiecePtr(3, 4);

	fenrir::Moves::getInstance().generateMoves(empty, board, moves);

	// Directly invoke real Moves to cover null/empty checks
	fenrir::RealMoves::getInstance().generateMoves(3, 4, board, moves);

	EXPECT_TRUE(moves.empty());
}

/* Utility test */
TEST_F(MovesTest, LogGeneratedMoves)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	const fenrir::Piece *pawn = board.addPiece(1, 4, 'P');

	fenrir::Moves::getInstance().generateMoves(pawn, board, moves);

	SUCCEED();
}

/* Test non-pawn piece to cover default case */
TEST_F(MovesTest, NonPawnPiece)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	const fenrir::Piece *knight = board.addPiece(0, 1, 'N');
	board.addPiece(1, 3, 'P');

	fenrir::Moves::getInstance().generateMoves(knight, board, moves);

	EXPECT_EQ(moves.size(), 2);
	EXPECT_TRUE(moveExists(moves, "b1", "a3"));
	EXPECT_TRUE(moveExists(moves, "b1", "c3"));
}

/* Test with rook piece to ensure default case coverage */
TEST_F(MovesTest, RookPiece)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	const fenrir::Piece *rook = board.addPiece(0, 0, 'R');
	board.addPiece(1, 0, 'P');
	board.addPiece(0, 1, 'N');

	fenrir::Moves::getInstance().generateMoves(rook, board, moves);

	EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, RookVerticalMovement)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
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
	const fenrir::Piece *bishop = board.addPiece(0, 2, 'B');
	board.addPiece(1, 1, 'P');
	board.addPiece(1, 3, 'P');

	fenrir::Moves::getInstance().generateMoves(bishop, board, moves);

	EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, QueenPiece)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	const fenrir::Piece *queen = board.addPiece(0, 3, 'Q');
	board.addPiece(0, 2, 'B');
	board.addPiece(1, 2, 'P');
	board.addPiece(1, 3, 'P');
	board.addPiece(1, 4, 'P');
	board.addPiece(0, 4, 'K');

	fenrir::Moves::getInstance().generateMoves(queen, board, moves);

	EXPECT_TRUE(moves.empty());
}

/* Knight movement tests */
TEST_F(MovesTest, KnightLShapeMovement)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
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
	const fenrir::Piece *knight = board.addPiece(3, 3, 'N');

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
	const fenrir::Piece *knight = board.addPiece(3, 3, 'N');

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
	const fenrir::Piece *king = board.addPiece(0, 4, 'K');
	board.addPiece(0, 3, 'Q');
	board.addPiece(1, 3, 'P');
	board.addPiece(1, 4, 'P');
	board.addPiece(1, 5, 'P');
	board.addPiece(0, 5, 'B');

	fenrir::Moves::getInstance().generateMoves(king, board, moves);

	EXPECT_EQ(moves.size(), 0);
}

TEST_F(MovesTest, KingSingleSquareMovement)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
	const fenrir::Piece *king = board.addPiece(3, 3, 'K');

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

TEST_F(MovesTest, KingBlockedByFriendlyPieces)
{
	MockBoard board;
	std::vector<fenrir::Move> moves;
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
	const fenrir::Piece *king = board.addPiece(3, 3, 'K');
	board.addPiece(3, 2, 'P');
	board.addPiece(3, 5, 'P');
	board.addPiece(2, 4, 'P');
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
	const fenrir::Piece *king = board.addPiece(0, 4, 'K');
	board.addPiece(0, 3, 'Q');
	board.addPiece(1, 3, 'P');
	board.addPiece(1, 4, 'P');
	board.addPiece(1, 5, 'P');
	board.addPiece(0, 5, 'B');

	fenrir::Moves::getInstance().generateMoves(king, board, moves);

	EXPECT_EQ(moves.size(), 0);
}

/* Stress test */
TEST_F(MovesTest, StressTestGenerateMoves)
{
	if (!test::getCI() || std::string(test::getCI()) != "true")

	{
		GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
	}

	fenrir::Board board(standard_position);

	for (int i = 0; i < 100000; i++)
	{
		char pawn = board.getPiece(1, 0);
		if (pawn == 'P')
		{
			std::vector<fenrir::Move> moves;
			fenrir::RealMoves::getInstance().generateMoves(1, 0, board, moves);
		}
	}
}
