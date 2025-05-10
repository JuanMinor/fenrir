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
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <iostream>
#include <regex>
#include "include/chess/board.h"
#include "include/chess/piece.h"
#include "include/core/core.h"

class BoardTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::cout << "****************************************************" << std::endl
                  << "Board tests are running..." << std::endl
                  << "****************************************************" << std::endl;
        valid_fen_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
        empty_fen_position = "8/8/8/8/8/8/8/8";
    }

    static void TearDownTestSuite() {}

    static const char *valid_fen_position;
    static const char *empty_fen_position;
};

const char *BoardTest::valid_fen_position = nullptr;
const char *BoardTest::empty_fen_position = nullptr;

/* Parsing tests */
TEST_F(BoardTest, ParseValidFENPosition)
{
    loki::Board board(valid_fen_position);
    std::vector<std::vector<loki::Piece *>> board_state = board.get_board();

    ASSERT_EQ(board_state.size(), loki::BOARD_SIZE);

    /* Check pieces */
    /* White rook */
    EXPECT_NE(board_state[0][0], nullptr);
    EXPECT_EQ(board_state[0][0]->get_alias(), 'R');
    EXPECT_EQ(board_state[0][0]->get_color(), loki::WHITE);

    /* Black king */
    EXPECT_NE(board_state[7][4], nullptr);
    EXPECT_EQ(board_state[7][4]->get_alias(), 'k');
    EXPECT_EQ(board_state[7][4]->get_color(), loki::BLACK);
}

TEST_F(BoardTest, ParseEmptyFENPosition)
{
    loki::Board board(empty_fen_position);
    std::vector<std::vector<loki::Piece *>> board_state = board.get_board();

    for (const std::vector<loki::Piece *> &rank : board_state)
    {
        for (const loki::Piece *const &square : rank)
        {
            EXPECT_EQ(square, nullptr);
        }
    }
}

/* Move tests */
TEST_F(BoardTest, MovePiece)
{
    loki::Board board(valid_fen_position);
    std::vector<std::vector<loki::Piece *>> board_state = board.get_board();

    /* White pawn @ a2 */
    loki::Piece *pawn = board_state[1][0];

    ASSERT_NE(pawn, nullptr);
    EXPECT_EQ(pawn->get_alias(), 'P');
    EXPECT_EQ(pawn->get_color(), loki::WHITE);

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
    loki::Board board(valid_fen_position);
    std::vector<std::vector<loki::Piece *>> board_state = board.get_board();

    /* White pawn @ a2 */
    loki::Piece *pawn = board_state[1][0];
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
    loki::Board board(valid_fen_position);
    std::vector<std::vector<loki::Piece *>> board_state = board.get_board();

    /* White pawn @ a2 */
    loki::Piece *pawn = board_state[6][0];

    ASSERT_NE(pawn, nullptr);

    /* Attempt to move out of bounds */
    board.move(pawn, 8, 0);

    board_state = board.get_board();
    EXPECT_EQ(board_state[6][0], pawn);
}

/* Edge cases */
TEST_F(BoardTest, InvalidFENThrows)
{
    EXPECT_THROW(loki::Board("invalid_fen_position_string"), std::invalid_argument);
    EXPECT_THROW(loki::Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/8"), std::invalid_argument);
    EXPECT_THROW(loki::Board("rnbqkbnr/pppppppp/8/8/8/7/PPPPPPPP/RNBQKBN"), std::invalid_argument);
}

TEST_F(BoardTest, PrintBoard)
{
    loki::Board board(valid_fen_position);

    /* Redirect std::cout to a stringstream */
    std::stringstream buffer;
    std::streambuf *old_cout = std::cout.rdbuf(buffer.rdbuf());

    board.print();

    /* Restore std::cout */
    std::cout.rdbuf(old_cout);

    std::string output = buffer.str();

    /* Remove ANSI escape sequences using a regex */
    std::regex ansi_escape_regex("\033\\[[0-9;]*m");
    std::string stripped_output = std::regex_replace(output, ansi_escape_regex, "");

    /* Verify output */
    /* Black pieces should be at ranks 7, 8 */
    EXPECT_NE(stripped_output.find("r n b q k b n r"), std::string::npos);
    /* White pieces should be at ranks 1, 2 */
    EXPECT_NE(stripped_output.find("R N B Q K B N R"), std::string::npos);
}

TEST_F(BoardTest, DestructorCleansUp)
{
    loki::Board *board = new loki::Board(valid_fen_position);
    delete board;
}

TEST_F(BoardTest, StressTestManyMoves)
{
    if (!test::CI || std::string(test::CI) != "true")
    {
        GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
    }

    loki::Board board(valid_fen_position);
    std::vector<std::vector<loki::Piece *>> board_state = board.get_board();

    ASSERT_EQ(board_state.size(), loki::BOARD_SIZE);

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
            loki::Piece *pawn = board_state[white_pawn_rank][white_pawn_file];
            ASSERT_NE(pawn, nullptr);
            ASSERT_EQ(pawn->get_alias(), 'P');
            ASSERT_EQ(pawn->get_color(), loki::WHITE);

            uint8_t target_rank = white_pawn_rank + white_pawn_direction;
            if (board_state[target_rank][white_pawn_file] == nullptr)
            {
                board.move(pawn, target_rank, white_pawn_file);
                board_state = board.get_board();
                EXPECT_EQ(board_state[target_rank][white_pawn_file], pawn);
                EXPECT_EQ(board_state[white_pawn_rank][white_pawn_file], nullptr);
                white_pawn_rank = target_rank; // Update the white pawn's rank
            }
            if (white_pawn_rank == 0 || white_pawn_rank == loki::BOARD_SIZE - 1)
            {
                white_pawn_direction *= -1;
            }
        }
        else
        {
            /* Move the black pawn forward if possible */
            loki::Piece *pawn = board_state[black_pawn_rank][black_pawn_file];
            ASSERT_NE(pawn, nullptr);
            ASSERT_EQ(pawn->get_alias(), 'p');
            ASSERT_EQ(pawn->get_color(), loki::BLACK);

            uint8_t target_rank = black_pawn_rank + black_pawn_direction;
            if (board_state[target_rank][black_pawn_file] == nullptr)
            {
                board.move(pawn, target_rank, black_pawn_file);
                board_state = board.get_board();
                EXPECT_EQ(board_state[target_rank][black_pawn_file], pawn);
                EXPECT_EQ(board_state[black_pawn_rank][black_pawn_file], nullptr);
                black_pawn_rank = target_rank; // Update the black pawn's rank
            }
            if (black_pawn_rank == 0 || black_pawn_rank == loki::BOARD_SIZE - 1)
            {
                black_pawn_direction *= -1;
            }
        }
    }

    /* Ensure the board is still valid after all moves */
    for (const std::vector<loki::Piece *> &rank : board_state)
    {
        for (const loki::Piece *const &square : rank)
        {
            if (square != nullptr)
            {
                EXPECT_GE(square->get_rank(), 0);
                EXPECT_LT(square->get_rank(), loki::BOARD_SIZE);
                EXPECT_GE(square->get_file(), 0);
                EXPECT_LT(square->get_file(), loki::BOARD_SIZE);
            }
        }
    }
}