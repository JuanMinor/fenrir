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
#include "include/chess/piece.h"
#include "include/core/core.h"

/* Test suite for the fenrir::Piece class */
class PieceTest : public ::testing::Test
{
protected:
	fenrir::Piece createWhitePawn()
	{
		return fenrir::Piece('P', 1, 1);
	}

	fenrir::Piece createBlackPawn()
	{
		return fenrir::Piece('p', 6, 1);
	}
};

/* Test the constructor and basic getters */
TEST_F(PieceTest, ConstructorAndGetters)
{
	/* Create a white pawn */
	fenrir::Piece pawn = createWhitePawn();

	EXPECT_EQ(pawn.getAlias(), 'P');
	EXPECT_EQ(pawn.getValue(), 1);
	EXPECT_EQ(pawn.getColor(), fenrir::WHITE);
	EXPECT_EQ(pawn.getRank(), 1);
	EXPECT_EQ(pawn.getFile(), 1);
	EXPECT_FALSE(pawn.getMoved());
}

TEST_F(PieceTest, SetRankAndMoved)
{
	/* Create a white pawn */
	fenrir::Piece pawn = createWhitePawn();

	pawn.setRank(2);
	EXPECT_EQ(pawn.getRank(), 2);
	EXPECT_TRUE(pawn.getMoved());
}

TEST_F(PieceTest, SetRankOutOfRange)
{
	fenrir::Piece pawn = createWhitePawn();

	EXPECT_THROW(pawn.setRank(8), std::runtime_error);
	EXPECT_THROW(pawn.setRank(-1), std::runtime_error);
}

TEST_F(PieceTest, SetFile)
{
	fenrir::Piece pawn = createWhitePawn();

	pawn.setFile(2);
	EXPECT_EQ(pawn.getFile(), 2);
}

TEST_F(PieceTest, SetFileOutOfRange)
{
	fenrir::Piece pawn = createWhitePawn();

	EXPECT_THROW(pawn.setFile(8), std::runtime_error);
	EXPECT_THROW(pawn.setFile(-1), std::runtime_error);
}

TEST_F(PieceTest, SetMoved)
{
	fenrir::Piece pawn = createWhitePawn();

	pawn.setMoved(true);
	EXPECT_TRUE(pawn.getMoved());

	pawn.setMoved(false);
	EXPECT_FALSE(pawn.getMoved());
}

TEST_F(PieceTest, BlackPiece)
{
	fenrir::Piece pawn = createBlackPawn();

	EXPECT_EQ(pawn.getAlias(), 'p');
	EXPECT_EQ(pawn.getValue(), 1);
	EXPECT_EQ(pawn.getColor(), fenrir::BLACK);
	EXPECT_EQ(pawn.getRank(), 6);
	EXPECT_EQ(pawn.getFile(), 1);
	EXPECT_FALSE(pawn.getMoved());
}

TEST_F(PieceTest, DestructorTest)
{
	{
		fenrir::Piece pawn = createWhitePawn();

		EXPECT_EQ(pawn.getAlias(), 'P');
		EXPECT_EQ(pawn.getValue(), 1);
		EXPECT_EQ(pawn.getColor(), fenrir::WHITE);
		EXPECT_EQ(pawn.getRank(), 1);
		EXPECT_EQ(pawn.getFile(), 1);
		EXPECT_FALSE(pawn.getMoved());
	}

	SUCCEED();
}

TEST_F(PieceTest, StressTest)
{
	if (!test::CI || std::string(test::CI) != "true")
	{
		GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
	}
	const int numPieces = 100000;
	std::vector<fenrir::Piece> pieces;

	/* Create a large number of white pawns */
	for (int i = 0; i < numPieces; ++i)
	{
		pieces.emplace_back('P', 1, 1);
	}

	/* Verify that all pieces were created correctly */
	for (const fenrir::Piece &pawn : pieces)
	{
		EXPECT_EQ(pawn.getAlias(), 'P');
		EXPECT_EQ(pawn.getValue(), 1);
		EXPECT_EQ(pawn.getColor(), fenrir::WHITE);
		EXPECT_EQ(pawn.getRank(), 1);
		EXPECT_EQ(pawn.getFile(), 1);
		EXPECT_FALSE(pawn.getMoved());
	}

	/* Modify all pieces and verify the changes */
	for (int i = 0; i < numPieces; ++i)
	{
		pieces[i].setRank(2);
		pieces[i].setFile(2);
		EXPECT_EQ(pieces[i].getRank(), 2);
		EXPECT_EQ(pieces[i].getFile(), 2);
		EXPECT_TRUE(pieces[i].getMoved());
	}
}