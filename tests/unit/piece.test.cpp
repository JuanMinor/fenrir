#include <gtest/gtest.h>
#include "include/chess/piece.h"
#include "include/core/core.h"

using namespace loki;

/* Test suite for the Piece class */
class PieceTest : public ::testing::Test
{
protected:
    Piece createWhitePawn()
    {
        return Piece('P', 1, 1);
    }

    Piece createBlackPawn()
    {
        return Piece('p', 6, 1);
    }
};

/* Test the constructor and basic getters */
TEST_F(PieceTest, ConstructorAndGetters)
{
    /* Create a white pawn */
    Piece pawn = createWhitePawn();

    EXPECT_EQ(pawn.get_alias(), 'P');
    EXPECT_EQ(pawn.get_value(), 1);
    EXPECT_EQ(pawn.get_color(), loki::WHITE);
    EXPECT_EQ(pawn.get_rank(), 1);
    EXPECT_EQ(pawn.get_file(), 1);
    EXPECT_FALSE(pawn.get_moved());
}

TEST_F(PieceTest, SetRankAndMoved)
{
    /* Create a white pawn */
    Piece pawn = createWhitePawn();

    pawn.set_rank(2);
    EXPECT_EQ(pawn.get_rank(), 2);
    EXPECT_TRUE(pawn.get_moved());
}

TEST_F(PieceTest, SetRankOutOfRange)
{
    Piece pawn = createWhitePawn();

    EXPECT_THROW(pawn.set_rank(8), std::out_of_range);
    EXPECT_THROW(pawn.set_rank(-1), std::out_of_range);
}

TEST_F(PieceTest, SetFile)
{
    Piece pawn = createWhitePawn();

    pawn.set_file(2);
    EXPECT_EQ(pawn.get_file(), 2);
}

TEST_F(PieceTest, SetFileOutOfRange)
{
    Piece pawn = createWhitePawn();

    EXPECT_THROW(pawn.set_file(8), std::out_of_range);
    EXPECT_THROW(pawn.set_file(-1), std::out_of_range);
}

TEST_F(PieceTest, SetMoved)
{
    Piece pawn = createWhitePawn();

    pawn.set_moved(true);
    EXPECT_TRUE(pawn.get_moved());

    pawn.set_moved(false);
    EXPECT_FALSE(pawn.get_moved());
}

TEST_F(PieceTest, BlackPiece)
{
    Piece pawn = createBlackPawn();

    EXPECT_EQ(pawn.get_alias(), 'p');
    EXPECT_EQ(pawn.get_value(), 1);
    EXPECT_EQ(pawn.get_color(), loki::BLACK);
    EXPECT_EQ(pawn.get_rank(), 6);
    EXPECT_EQ(pawn.get_file(), 1);
    EXPECT_FALSE(pawn.get_moved());
}

TEST_F(PieceTest, DestructorTest)
{
    {
        Piece pawn = createWhitePawn();

        EXPECT_EQ(pawn.get_alias(), 'P');
        EXPECT_EQ(pawn.get_value(), 1);
        EXPECT_EQ(pawn.get_color(), loki::WHITE);
        EXPECT_EQ(pawn.get_rank(), 1);
        EXPECT_EQ(pawn.get_file(), 1);
        EXPECT_FALSE(pawn.get_moved());
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
    std::vector<Piece> pieces;

    /* Create a large number of white pawns */
    for (int i = 0; i < numPieces; ++i)
    {
        pieces.emplace_back('P', 1, 1);
    }

    /* Verify that all pieces were created correctly */
    for (const Piece &pawn : pieces)
    {
        EXPECT_EQ(pawn.get_alias(), 'P');
        EXPECT_EQ(pawn.get_value(), 1);
        EXPECT_EQ(pawn.get_color(), loki::WHITE);
        EXPECT_EQ(pawn.get_rank(), 1);
        EXPECT_EQ(pawn.get_file(), 1);
        EXPECT_FALSE(pawn.get_moved());
    }

    /* Modify all pieces and verify the changes */
    for (int i = 0; i < numPieces; ++i)
    {
        pieces[i].set_rank(2);
        pieces[i].set_file(2);
        EXPECT_EQ(pieces[i].get_rank(), 2);
        EXPECT_EQ(pieces[i].get_file(), 2);
        EXPECT_TRUE(pieces[i].get_moved());
    }
}