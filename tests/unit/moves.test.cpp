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
    fenrir::Moves &moves1 = fenrir::Moves::get_instance();
    fenrir::Moves &moves2 = fenrir::Moves::get_instance();

    EXPECT_EQ(&moves1, &moves2);
}

/* Pawn movement tests */
TEST_F(MovesTest, WhitePawnSingleMove)
{
    fenrir::Board board(pawn_position);
    std::vector<std::pair<const std::string, const std::string>> moves;
    const fenrir::Piece *pawn = board.get_piece(2, 4);

    ASSERT_NE(pawn, nullptr);
    EXPECT_EQ(pawn->get_alias(), 'P');

    fenrir::Moves::get_instance().generate_moves(pawn, &board, moves);

    EXPECT_FALSE(moves.empty());
    EXPECT_TRUE(moveExists(moves, "e3", "e4"));
}

TEST_F(MovesTest, BlackPawnSingleMove)
{
    fenrir::Board board(pawn_position);
    std::vector<std::pair<const std::string, const std::string>> moves;
    const fenrir::Piece *pawn = board.get_piece(3, 3);

    ASSERT_NE(pawn, nullptr);
    EXPECT_EQ(pawn->get_alias(), 'p');

    fenrir::Moves::get_instance().generate_moves(pawn, &board, moves);

    EXPECT_FALSE(moves.empty());
    EXPECT_TRUE(moveExists(moves, "d4", "d3"));
}

TEST_F(MovesTest, WhitePawnDoubleMove)
{
    fenrir::Board board(standard_position);
    std::vector<std::pair<const std::string, const std::string>> moves;
    const fenrir::Piece *pawn = board.get_piece(1, 4);

    ASSERT_NE(pawn, nullptr);
    EXPECT_EQ(pawn->get_alias(), 'P');

    fenrir::Moves::get_instance().generate_moves(pawn, &board, moves);

    EXPECT_EQ(moves.size(), 2);
    EXPECT_TRUE(moveExists(moves, "e2", "e3"));
    EXPECT_TRUE(moveExists(moves, "e2", "e4"));
}

TEST_F(MovesTest, BlackPawnDoubleMove)
{
    fenrir::Board board(standard_position);
    std::vector<std::pair<const std::string, const std::string>> moves;
    const fenrir::Piece *pawn = board.get_piece(6, 4);

    ASSERT_NE(pawn, nullptr);
    EXPECT_EQ(pawn->get_alias(), 'p');

    fenrir::Moves::get_instance().generate_moves(pawn, &board, moves);

    EXPECT_EQ(moves.size(), 2);
    EXPECT_TRUE(moveExists(moves, "e7", "e6"));
    EXPECT_TRUE(moveExists(moves, "e7", "e5"));
}

TEST_F(MovesTest, PawnCapture)
{

    fenrir::Board board("8/8/8/3p4/2P5/8/8/8 w KQkq - 0 1");
    std::vector<std::pair<const std::string, const std::string>> moves;
    const fenrir::Piece *pawn = board.get_piece(3, 2);

    ASSERT_NE(pawn, nullptr);
    EXPECT_EQ(pawn->get_alias(), 'P');

    fenrir::Moves::get_instance().generate_moves(pawn, &board, moves);

    EXPECT_TRUE(moveExists(moves, "c4", "d5"));
}

TEST_F(MovesTest, EnPassantCapture)
{
    fenrir::Board board(en_passant_position);
    EXPECT_EQ(board.get_en_passant(), "a6");

    std::vector<std::pair<const std::string, const std::string>> moves;
    const fenrir::Piece *pawn = board.get_piece(4, 1);

    ASSERT_NE(pawn, nullptr);
    EXPECT_EQ(pawn->get_alias(), 'P');

    fenrir::Moves::get_instance().generate_moves(pawn, &board, moves);

    EXPECT_TRUE(moveExists(moves, "b5", "a6"));
}

TEST_F(MovesTest, BlockedPawn)
{

    fenrir::Board board("8/8/8/3p4/3P4/8/8/8 w KQkq - 0 1");
    std::vector<std::pair<const std::string, const std::string>> moves;

    const fenrir::Piece *pawn = board.get_piece(3, 3);

    ASSERT_NE(pawn, nullptr);
    EXPECT_EQ(pawn->get_alias(), 'P');

    fenrir::Moves::get_instance().generate_moves(pawn, &board, moves);

    EXPECT_FALSE(moveExists(moves, "d4", "d5"));
}

TEST_F(MovesTest, PawnAtEdge)
{

    fenrir::Board board("P7/8/8/8/8/8/8/8 w KQkq - 0 1");
    std::vector<std::pair<const std::string, const std::string>> moves;

    const fenrir::Piece *pawn = board.get_piece(7, 0);

    ASSERT_NE(pawn, nullptr);
    EXPECT_EQ(pawn->get_alias(), 'P');

    fenrir::Moves::get_instance().generate_moves(pawn, &board, moves);

    EXPECT_TRUE(moves.empty());
}

/* Edge cases */
TEST_F(MovesTest, NullPiece)
{
    fenrir::Board board(standard_position);
    std::vector<std::pair<const std::string, const std::string>> moves;

    fenrir::Moves::get_instance().generate_moves(nullptr, &board, moves);

    EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, EmptySquare)
{
    fenrir::Board board(standard_position);
    std::vector<std::pair<const std::string, const std::string>> moves;

    const fenrir::Piece *empty = board.get_piece(3, 4);

    ASSERT_EQ(empty, nullptr);

    fenrir::Moves::get_instance().generate_moves(empty, &board, moves);

    EXPECT_TRUE(moves.empty());
}

/* Utility test */
TEST_F(MovesTest, LogGeneratedMoves)
{

    fenrir::Board board(standard_position);
    std::vector<std::pair<const std::string, const std::string>> moves;
    const fenrir::Piece *pawn = board.get_piece(1, 4);

    ASSERT_NE(pawn, nullptr);

    fenrir::Moves::get_instance().generate_moves(pawn, &board, moves);

    SUCCEED();
}

/* Test non-pawn piece to cover default case */
TEST_F(MovesTest, NonPawnPiece)
{
    fenrir::Board board(standard_position);
    std::vector<std::pair<const std::string, const std::string>> moves;

    const fenrir::Piece *knight = board.get_piece(0, 1);

    ASSERT_NE(knight, nullptr);
    ASSERT_EQ(std::tolower(knight->get_alias()), 'n');

    fenrir::Moves::get_instance().generate_moves(knight, &board, moves);

    EXPECT_TRUE(moves.empty());
}

/* Test with rook piece to ensure default case coverage */
TEST_F(MovesTest, RookPiece)
{
    fenrir::Board board(standard_position);
    std::vector<std::pair<const std::string, const std::string>> moves;

    const fenrir::Piece *rook = board.get_piece(0, 0);

    ASSERT_NE(rook, nullptr);
    ASSERT_EQ(std::tolower(rook->get_alias()), 'r');

    fenrir::Moves::get_instance().generate_moves(rook, &board, moves);

    EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, RookVerticalMovement)
{
    fenrir::Board board("8/8/8/8/3R4/8/8/8 w - - 0 1");
    std::vector<std::pair<const std::string, const std::string>> moves;

    const fenrir::Piece *rook = board.get_piece(3, 3);

    ASSERT_NE(rook, nullptr);
    ASSERT_EQ(std::tolower(rook->get_alias()), 'r');

    fenrir::Moves::get_instance().generate_moves(rook, &board, moves);

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

    const fenrir::Piece *rook = board.get_piece(3, 3);

    ASSERT_NE(rook, nullptr);
    ASSERT_EQ(std::tolower(rook->get_alias()), 'r');

    fenrir::Moves::get_instance().generate_moves(rook, &board, moves);

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

    const fenrir::Piece *rook = board.get_piece(3, 3);

    ASSERT_NE(rook, nullptr);
    ASSERT_EQ(std::tolower(rook->get_alias()), 'r');

    fenrir::Moves::get_instance().generate_moves(rook, &board, moves);

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

    const fenrir::Piece *rook = board.get_piece(3, 3);

    ASSERT_NE(rook, nullptr);
    ASSERT_EQ(std::tolower(rook->get_alias()), 'r');

    fenrir::Moves::get_instance().generate_moves(rook, &board, moves);

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

    const fenrir::Piece *rook = board.get_piece(3, 3);

    ASSERT_NE(rook, nullptr);
    ASSERT_EQ(std::tolower(rook->get_alias()), 'r');

    fenrir::Moves::get_instance().generate_moves(rook, &board, moves);

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

    const fenrir::Piece *rook = board.get_piece(7, 0);

    ASSERT_NE(rook, nullptr);
    ASSERT_EQ(std::tolower(rook->get_alias()), 'r');

    fenrir::Moves::get_instance().generate_moves(rook, &board, moves);

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

    const fenrir::Piece *rook = board.get_piece(3, 0);

    ASSERT_NE(rook, nullptr);
    ASSERT_EQ(std::tolower(rook->get_alias()), 'r');

    fenrir::Moves::get_instance().generate_moves(rook, &board, moves);

    EXPECT_EQ(moves.size(), 14);
    EXPECT_TRUE(moveExists(moves, "a4", "a8"));
    EXPECT_TRUE(moveExists(moves, "a4", "a1"));
    EXPECT_TRUE(moveExists(moves, "a4", "h4"));
}

TEST_F(MovesTest, BlackRookMovement)
{
    fenrir::Board board("8/8/8/8/3r4/8/8/8 b - - 0 1");
    std::vector<std::pair<const std::string, const std::string>> moves;

    const fenrir::Piece *rook = board.get_piece(3, 3);

    ASSERT_NE(rook, nullptr);
    ASSERT_EQ(std::tolower(rook->get_alias()), 'r');

    fenrir::Moves::get_instance().generate_moves(rook, &board, moves);

    EXPECT_EQ(moves.size(), 14);
    EXPECT_TRUE(moveExists(moves, "d4", "d1"));
    EXPECT_TRUE(moveExists(moves, "d4", "d8"));
    EXPECT_TRUE(moveExists(moves, "d4", "a4"));
    EXPECT_TRUE(moveExists(moves, "d4", "h4"));
}

/* Stress test */
TEST_F(MovesTest, StressTestGenerateMoves)
{
    if (!test::CI || std::string(test::CI) != "true")
    {
        GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
    }

    fenrir::Board board(standard_position);
    std::vector<std::vector<fenrir::Piece *>> board_state = board.get_board();

    for (int i = 0; i < 100000; i++)
    {
        const fenrir::Piece *pawn = board_state[1][0];
        if (pawn && pawn->get_alias() == 'P')
        {
            std::vector<std::pair<const std::string, const std::string>> moves;
            fenrir::Moves::get_instance().generate_moves(pawn, &board, moves);
        }
    }
}