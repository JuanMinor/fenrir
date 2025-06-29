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
    std::vector<std::vector<fenrir::Piece *>> board_state = board.get_board();

    ASSERT_EQ(board_state.size(), fenrir::BOARD_SIZE);

    /* Check pieces */
    /* White rook */
    EXPECT_NE(board_state[0][0], nullptr);
    EXPECT_EQ(board_state[0][0]->get_alias(), 'R');
    EXPECT_EQ(board_state[0][0]->get_color(), fenrir::WHITE);

    /* Black king */
    EXPECT_NE(board_state[7][4], nullptr);
    EXPECT_EQ(board_state[7][4]->get_alias(), 'k');
    EXPECT_EQ(board_state[7][4]->get_color(), fenrir::BLACK);
}

TEST_F(BoardTest, ParseEmptyFENPosition)
{
    fenrir::Board board(empty_fen_position);
    std::vector<std::vector<fenrir::Piece *>> board_state = board.get_board();

    for (const std::vector<fenrir::Piece *> &rank : board_state)
    {
        for (const fenrir::Piece *const &square : rank)
        {
            EXPECT_EQ(square, nullptr);
        }
    }
}

TEST_F(BoardTest, SetEnPassant)
{
    fenrir::Board board(valid_fen_position);

    EXPECT_EQ(board.get_en_passant(), "a3");
}

TEST_F(BoardTest, GetPiece_AtValidPosition)
{
    fenrir::Board board(valid_fen_position);
    fenrir::Piece *piece = board.get_piece(0, 0);

    ASSERT_NE(piece, nullptr);
    EXPECT_EQ(piece->get_alias(), 'R');
    EXPECT_EQ(piece->get_color(), fenrir::WHITE);
}

TEST_F(BoardTest, GetPiece_AtInvalidPosition)
{
    fenrir::Board board(valid_fen_position);
    fenrir::Piece *piece = board.get_piece(8, 0);

    EXPECT_EQ(piece, nullptr);
}

/* Move tests */
TEST_F(BoardTest, MovePiece)
{
    fenrir::Board board(valid_fen_position);
    std::vector<std::vector<fenrir::Piece *>> board_state = board.get_board();

    /* White pawn @ a2 */
    fenrir::Piece *pawn = board_state[1][0];

    ASSERT_NE(pawn, nullptr);
    EXPECT_EQ(pawn->get_alias(), 'P');
    EXPECT_EQ(pawn->get_color(), fenrir::WHITE);

    /* Move the pawn from a2 to a4 */
    board.move(pawn, 3, 0);

    board_state = board.get_board();
    EXPECT_EQ(board_state[3][0], pawn);
    EXPECT_EQ(board_state[1][0], nullptr);
    EXPECT_EQ(pawn->get_rank(), 3);
    EXPECT_EQ(pawn->get_file(), 0);
}

TEST_F(BoardTest, MoveToOccupiedSquare)
{
    fenrir::Board board(valid_fen_position);
    std::vector<std::vector<fenrir::Piece *>> board_state = board.get_board();

    /* White pawn @ a2 */
    fenrir::Piece *pawn = board_state[1][0];
    ASSERT_NE(pawn, nullptr);

    /* Attempt to move the pawn to an occupied square (a1) */
    board.move(pawn, 0, 0);

    /* Ensure the move was not performed */
    board_state = board.get_board();
    EXPECT_EQ(board_state[1][0], pawn);
    EXPECT_NE(board_state[0][0], nullptr);
}

TEST_F(BoardTest, MoveOutOfBounds)
{
    fenrir::Board board(valid_fen_position);
    std::vector<std::vector<fenrir::Piece *>> board_state = board.get_board();

    /* White pawn @ a2 */
    fenrir::Piece *pawn = board_state[6][0];

    ASSERT_NE(pawn, nullptr);

    /* Attempt to move out of bounds */
    board.move(pawn, 8, 0);

    board_state = board.get_board();
    EXPECT_EQ(board_state[6][0], pawn);
}

TEST_F(BoardTest, MovePawnAndEnPassantIsSet)
{
    fenrir::Board board(valid_fen_position);
    std::vector<std::vector<fenrir::Piece *>> board_state = board.get_board();

    /* White pawn @ a2 */
    fenrir::Piece *pawn = board_state[1][0];
    ASSERT_NE(pawn, nullptr);

    /* Move the pawn to a4 */
    board.move(pawn, 3, 0);

    EXPECT_EQ(board.get_en_passant(), "a3");
}

TEST_F(BoardTest, MovePawnAndEnPassantIsCleared)
{
    fenrir::Board board(valid_fen_position);
    std::vector<std::vector<fenrir::Piece *>> board_state = board.get_board();

    /* White pawn @ a2 */
    fenrir::Piece *pawn = board_state[1][0];
    ASSERT_NE(pawn, nullptr);

    /* Move the pawn to a4 */
    board.move(pawn, 3, 0);

    /* Move the pawn to a5 */
    board.move(pawn, 4, 0);

    EXPECT_EQ(board.get_en_passant(), "");
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

    fenrir::Piece *invalid_piece = new fenrir::Piece('P', 4, 4);

    board.move(invalid_piece, 5, 4);

    std::vector<std::vector<fenrir::Piece *>> board_state = board.get_board();
    EXPECT_EQ(board_state[4][4], nullptr);
    EXPECT_EQ(board_state[5][4], nullptr);

    delete invalid_piece;
}

TEST_F(BoardTest, StressTestManyMoves)
{
    if (!test::CI || std::string(test::CI) != "true")
    {
        GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
    }

    fenrir::Board board(valid_fen_position);
    std::vector<std::vector<fenrir::Piece *>> board_state = board.get_board();

    ASSERT_EQ(board_state.size(), fenrir::BOARD_SIZE);

    /* Track the positions of the white and black pawns */
    uint8_t white_pawn_rank = 1, white_pawn_file = 0;
    uint8_t black_pawn_rank = 6, black_pawn_file = 0;
    uint8_t white_pawn_direction = 1, black_pawn_direction = -1;

    /* Perform a large number of moves */
    for (int i = 0; i < 100000; ++i)
    {
        if (i % 2 == 0)
        {
            /* Move the white pawn forward if possible */
            fenrir::Piece *pawn = board_state[white_pawn_rank][white_pawn_file];
            ASSERT_NE(pawn, nullptr);
            ASSERT_EQ(pawn->get_alias(), 'P');
            ASSERT_EQ(pawn->get_color(), fenrir::WHITE);

            uint8_t target_rank = white_pawn_rank + white_pawn_direction;
            if (board_state[target_rank][white_pawn_file] == nullptr)
            {
                board.move(pawn, target_rank, white_pawn_file);
                board_state = board.get_board();
                EXPECT_EQ(board_state[target_rank][white_pawn_file], pawn);
                EXPECT_EQ(board_state[white_pawn_rank][white_pawn_file], nullptr);
                white_pawn_rank = target_rank;
            }
            if (white_pawn_rank == 0 || white_pawn_rank == fenrir::BOARD_SIZE - 1)
            {
                white_pawn_direction *= -1;
            }
        }
        else
        {
            /* Move the black pawn forward if possible */
            fenrir::Piece *pawn = board_state[black_pawn_rank][black_pawn_file];
            ASSERT_NE(pawn, nullptr);
            ASSERT_EQ(pawn->get_alias(), 'p');
            ASSERT_EQ(pawn->get_color(), fenrir::BLACK);

            uint8_t target_rank = black_pawn_rank + black_pawn_direction;
            if (board_state[target_rank][black_pawn_file] == nullptr)
            {
                board.move(pawn, target_rank, black_pawn_file);
                board_state = board.get_board();
                EXPECT_EQ(board_state[target_rank][black_pawn_file], pawn);
                EXPECT_EQ(board_state[black_pawn_rank][black_pawn_file], nullptr);
                black_pawn_rank = target_rank;
            }
            if (black_pawn_rank == 0 || black_pawn_rank == fenrir::BOARD_SIZE - 1)
            {
                black_pawn_direction *= -1;
            }
        }
    }

    /* Ensure the board is still valid after all moves */
    for (const std::vector<fenrir::Piece *> &rank : board_state)
    {
        for (const fenrir::Piece *const &square : rank)
        {
            if (square != nullptr)
            {
                EXPECT_GE(square->get_rank(), 0);
                EXPECT_LT(square->get_rank(), fenrir::BOARD_SIZE);
                EXPECT_GE(square->get_file(), 0);
                EXPECT_LT(square->get_file(), fenrir::BOARD_SIZE);
            }
        }
    }
}

/* FEN generation tests */
TEST_F(BoardTest, GetFenAfterCreation)
{
    fenrir::Board board(valid_fen_position);

    EXPECT_EQ(board.get_fen(), valid_fen_position);
}

TEST_F(BoardTest, GetFenAfterMove)
{
    fenrir::Board board("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    fenrir::Piece *black_pawn = board.get_piece(6, 3);
    board.move(black_pawn, 4, 3);
    EXPECT_EQ(board.get_fen(), "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR b KQkq d6 0 1");
}

TEST_F(BoardTest, GetFenEmptyBoard)
{
    fenrir::Board board(empty_fen_position);

    EXPECT_EQ(board.get_fen(), empty_fen_position);
}

TEST_F(BoardTest, GetFenWithNoCastling)
{
    std::string fen_no_castling = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1";
    fenrir::Board board(fen_no_castling);

    EXPECT_EQ(board.get_fen(), fen_no_castling);
}

TEST_F(BoardTest, GetFenWithPartialCastling)
{
    std::string fen_partial_castling = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kq - 0 1";
    fenrir::Board board(fen_partial_castling);

    EXPECT_EQ(board.get_fen(), fen_partial_castling);
}

TEST_F(BoardTest, GetFenAfterPawnDoubleMove)
{
    fenrir::Board board(valid_fen_position);
    fenrir::Piece *pawn = board.get_piece(1, 0);
    board.move(pawn, 3, 0);
    EXPECT_EQ(board.get_fen(), "rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR w KQkq a3 0 1");
}