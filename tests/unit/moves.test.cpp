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
#include "include/chess/attacks.h"
#include "include/core/core.h"
#include "include/utils/utils.h"

namespace chess
{
    class Piece
    {
    private:
        char alias;
        uint8_t rank;
        uint8_t file;

    public:
        Piece(char a, uint8_t r, uint8_t f) : alias(a), rank(r), file(f) {}
        char getAlias() const { return alias; }
        uint8_t getRank() const { return rank; }
        uint8_t getFile() const { return file; }
        uint8_t get_color() const { return std::isupper(static_cast<unsigned char>(alias)) ? chess::WHITE : chess::BLACK; }
    };

    using RealMoves = chess::Moves;

    class MovesWrapper
    {
    public:
        static MovesWrapper &get_instance()
        {
            static MovesWrapper instance;
            return instance;
        }
        void generate_moves(const Piece *piece, const chess::AbstractBoard &board, std::vector<chess::Move> &moves) const
        {
            if (!piece)
            {
                logger::ERROR("Piece is null. Moves cannot be generated 😢");
                return;
            }
            ::chess::Moves::generate_moves(piece->getRank(), piece->getFile(), board, moves);
        }
    };
}

// #define Moves MovesWrapper  // Commented out to avoid macro conflicts with chess::Moves::

class MockBoard : public chess::AbstractBoard
{
public:
    std::map<std::pair<uint8_t, uint8_t>, std::unique_ptr<chess::Piece>> pieces;
    std::string en_passant;
    std::string castling;
    uint8_t color_to_move;

    MockBoard() : en_passant(""), castling("KQkq"), color_to_move(chess::WHITE) {}

    char get_piece(uint8_t rank, uint8_t file) const override
    {
        auto it = pieces.find({rank, file});
        if (it != pieces.end())
        {
            return it->second->getAlias();
        }
        return '\0';
    }

    chess::Piece *getPiecePtr(uint8_t rank, uint8_t file) const
    {
        auto it = pieces.find({rank, file});
        if (it != pieces.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    const std::string &get_en_passant(void) const override
    {
        return en_passant;
    }

    uint8_t get_color(void) const override
    {
        return color_to_move;
    }

    const std::string &get_castling_rights(void) const override
    {
        return castling;
    }

    uint64_t get_occupancy(uint8_t clr) const override
    {
        uint64_t occ = 0ULL;
        for (const auto &kv : pieces)
        {
            char alias = kv.second->getAlias();
            bool is_white = std::isupper(static_cast<unsigned char>(alias));
            if ((clr == chess::WHITE) == is_white)
            {
                uint8_t sq = static_cast<uint8_t>(kv.first.first * 8 + kv.first.second);
                occ |= (1ULL << sq);
            }
        }
        return occ;
    }

    uint64_t get_combined_occupancy(void) const override
    {
        uint64_t occ = 0ULL;
        for (const auto &kv : pieces)
        {
            uint8_t sq = static_cast<uint8_t>(kv.first.first * 8 + kv.first.second);
            occ |= (1ULL << sq);
        }
        return occ;
    }

    uint64_t get_en_passant_bb(void) const override
    {
        if (en_passant.empty())
            return 0ULL;
        uint8_t epRank = 0, epFile = 0;
        utils::parse_algebraic_notation(en_passant.c_str(), epRank, epFile);
        return (1ULL << (epRank * 8 + epFile));
    }

    uint64_t get_bitboard(int /*index*/) const override
    {
        return 0ULL; /* MockBoard doesn't track per-piece bitboards */
    }

    uint8_t get_half_move_clock(void) const override
    {
        return 0;
    }

    chess::Piece *addPiece(uint8_t rank, uint8_t file, char alias)
    {
        pieces[{rank, file}] = std::make_unique<chess::Piece>(alias, rank, file);
        return pieces[{rank, file}].get();
    }

    void set_en_passant(const std::string &ep)
    {
        en_passant = ep;
    }

    void set_castling(const std::string &c)
    {
        castling = c;
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

    bool moveExists(const std::vector<chess::Move> &moves,
                    const std::string &from, const std::string &to) const
    {
        for (const auto &move : moves)
        {
            if (move.get_from() == from && move.get_to() == to)
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
    chess::MovesWrapper &moves1 = chess::MovesWrapper::get_instance();
    chess::MovesWrapper &moves2 = chess::MovesWrapper::get_instance();

    EXPECT_EQ(&moves1, &moves2);
}

/* Pawn movement tests */
TEST_F(MovesTest, WhitePawnSingleMove)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *pawn = board.addPiece(2, 4, 'P');

    chess::MovesWrapper::get_instance().generate_moves(pawn, board, moves);

    EXPECT_FALSE(moves.empty());
    EXPECT_TRUE(moveExists(moves, "e3", "e4"));
}

TEST_F(MovesTest, BlackPawnSingleMove)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *pawn = board.addPiece(3, 3, 'p');

    chess::MovesWrapper::get_instance().generate_moves(pawn, board, moves);

    EXPECT_FALSE(moves.empty());
    EXPECT_TRUE(moveExists(moves, "d4", "d3"));
}

TEST_F(MovesTest, WhitePawnDoubleMove)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *pawn = board.addPiece(1, 4, 'P');

    chess::MovesWrapper::get_instance().generate_moves(pawn, board, moves);

    EXPECT_EQ(moves.size(), 2);
    EXPECT_TRUE(moveExists(moves, "e2", "e3"));
    EXPECT_TRUE(moveExists(moves, "e2", "e4"));
}

TEST_F(MovesTest, BlackPawnDoubleMove)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *pawn = board.addPiece(6, 4, 'p');

    chess::MovesWrapper::get_instance().generate_moves(pawn, board, moves);

    EXPECT_EQ(moves.size(), 2);
    EXPECT_TRUE(moveExists(moves, "e7", "e6"));
    EXPECT_TRUE(moveExists(moves, "e7", "e5"));
}

TEST_F(MovesTest, PawnCapture)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *pawn = board.addPiece(3, 2, 'P');
    board.addPiece(4, 3, 'p');

    chess::MovesWrapper::get_instance().generate_moves(pawn, board, moves);

    EXPECT_TRUE(moveExists(moves, "c4", "d5"));
}

TEST_F(MovesTest, PawnCaptureLeft)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *pawn = board.addPiece(3, 2, 'P');
    board.addPiece(4, 1, 'p');

    chess::MovesWrapper::get_instance().generate_moves(pawn, board, moves);

    EXPECT_TRUE(moveExists(moves, "c4", "b5"));
}

TEST_F(MovesTest, EnPassantCapture)
{
    MockBoard board;
    board.set_en_passant("a6");
    std::vector<chess::Move> moves;
    const chess::Piece *pawn = board.addPiece(4, 1, 'P');
    board.addPiece(4, 0, 'p');

    chess::MovesWrapper::get_instance().generate_moves(pawn, board, moves);

    EXPECT_TRUE(moveExists(moves, "b5", "a6"));
}

TEST_F(MovesTest, BlockedPawn)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *pawn = board.addPiece(3, 3, 'P');
    board.addPiece(4, 3, 'p');

    chess::MovesWrapper::get_instance().generate_moves(pawn, board, moves);

    EXPECT_FALSE(moveExists(moves, "d4", "d5"));
}

TEST_F(MovesTest, PawnAtEdge)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *pawn = board.addPiece(7, 0, 'P');

    chess::MovesWrapper::get_instance().generate_moves(pawn, board, moves);

    EXPECT_TRUE(moves.empty());
}

/* Edge cases */
TEST_F(MovesTest, NullPiece)
{
    MockBoard board;
    std::vector<chess::Move> moves;

    chess::MovesWrapper::get_instance().generate_moves(nullptr, board, moves);

    EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, EmptySquare)
{
    MockBoard board;
    std::vector<chess::Move> moves;

    const chess::Piece *empty = board.getPiecePtr(3, 4);

    chess::MovesWrapper::get_instance().generate_moves(empty, board, moves);

    /* Directly invoke RealMoves to cover null/empty checks */
    ::chess::Moves::generate_moves(3, 4, board, moves);

    EXPECT_TRUE(moves.empty());
}

/* Utility test */
TEST_F(MovesTest, LogGeneratedMoves)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *pawn = board.addPiece(1, 4, 'P');

    chess::MovesWrapper::get_instance().generate_moves(pawn, board, moves);

    SUCCEED();
}

/* Test non-pawn piece to cover default case */
TEST_F(MovesTest, NonPawnPiece)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *knight = board.addPiece(0, 1, 'N');
    board.addPiece(1, 3, 'P');

    chess::MovesWrapper::get_instance().generate_moves(knight, board, moves);

    EXPECT_EQ(moves.size(), 2);
    EXPECT_TRUE(moveExists(moves, "b1", "a3"));
    EXPECT_TRUE(moveExists(moves, "b1", "c3"));
}

/* Test with rook piece to ensure default case coverage */
TEST_F(MovesTest, RookPiece)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *rook = board.addPiece(0, 0, 'R');
    board.addPiece(1, 0, 'P');
    board.addPiece(0, 1, 'N');

    chess::MovesWrapper::get_instance().generate_moves(rook, board, moves);

    EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, RookVerticalMovement)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *rook = board.addPiece(3, 3, 'R');

    chess::MovesWrapper::get_instance().generate_moves(rook, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *rook = board.addPiece(3, 3, 'R');

    chess::MovesWrapper::get_instance().generate_moves(rook, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *rook = board.addPiece(3, 3, 'R');
    board.addPiece(4, 3, 'P');

    chess::MovesWrapper::get_instance().generate_moves(rook, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *rook = board.addPiece(3, 3, 'R');
    board.addPiece(4, 3, 'p');

    chess::MovesWrapper::get_instance().generate_moves(rook, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *rook = board.addPiece(3, 3, 'R');
    board.addPiece(4, 3, 'P');

    chess::MovesWrapper::get_instance().generate_moves(rook, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *rook = board.addPiece(7, 0, 'R');

    chess::MovesWrapper::get_instance().generate_moves(rook, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *rook = board.addPiece(3, 0, 'R');

    chess::MovesWrapper::get_instance().generate_moves(rook, board, moves);

    EXPECT_EQ(moves.size(), 14);
    EXPECT_TRUE(moveExists(moves, "a4", "a8"));
    EXPECT_TRUE(moveExists(moves, "a4", "a1"));
    EXPECT_TRUE(moveExists(moves, "a4", "h4"));
}

TEST_F(MovesTest, BlackRookMovement)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *rook = board.addPiece(3, 3, 'r');

    chess::MovesWrapper::get_instance().generate_moves(rook, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *bishop = board.addPiece(3, 3, 'B');

    chess::MovesWrapper::get_instance().generate_moves(bishop, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *bishop = board.addPiece(3, 3, 'B');
    board.addPiece(4, 2, 'P');
    board.addPiece(4, 4, 'P');

    chess::MovesWrapper::get_instance().generate_moves(bishop, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *bishop = board.addPiece(3, 3, 'B');
    board.addPiece(4, 2, 'p');
    board.addPiece(4, 4, 'p');

    chess::MovesWrapper::get_instance().generate_moves(bishop, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *bishop = board.addPiece(7, 0, 'B');

    chess::MovesWrapper::get_instance().generate_moves(bishop, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *bishop = board.addPiece(3, 0, 'B');

    chess::MovesWrapper::get_instance().generate_moves(bishop, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *bishop = board.addPiece(3, 3, 'b');

    chess::MovesWrapper::get_instance().generate_moves(bishop, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *queen = board.addPiece(3, 3, 'Q');

    chess::MovesWrapper::get_instance().generate_moves(queen, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *queen = board.addPiece(3, 3, 'Q');
    board.addPiece(4, 2, 'P');
    board.addPiece(4, 3, 'P');
    board.addPiece(4, 4, 'P');
    board.addPiece(3, 2, 'P');
    board.addPiece(3, 4, 'P');
    board.addPiece(2, 2, 'P');
    board.addPiece(2, 3, 'P');
    board.addPiece(2, 4, 'P');

    chess::MovesWrapper::get_instance().generate_moves(queen, board, moves);

    EXPECT_TRUE(moves.empty());
    EXPECT_EQ(moves.size(), 0);
}

TEST_F(MovesTest, QueenCaptureEnemyPieces)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *queen = board.addPiece(3, 3, 'Q');
    board.addPiece(4, 2, 'p');
    board.addPiece(4, 3, 'p');
    board.addPiece(4, 4, 'p');
    board.addPiece(3, 2, 'p');
    board.addPiece(3, 4, 'p');
    board.addPiece(2, 2, 'p');
    board.addPiece(2, 3, 'p');
    board.addPiece(2, 4, 'p');

    chess::MovesWrapper::get_instance().generate_moves(queen, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *queen = board.addPiece(7, 0, 'Q');

    chess::MovesWrapper::get_instance().generate_moves(queen, board, moves);

    EXPECT_TRUE(moveExists(moves, "a8", "a1"));
    EXPECT_TRUE(moveExists(moves, "a8", "h8"));
    EXPECT_TRUE(moveExists(moves, "a8", "h1"));
    EXPECT_EQ(moves.size(), 21);
}

TEST_F(MovesTest, BlackQueenMovement)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *queen = board.addPiece(3, 3, 'q');

    chess::MovesWrapper::get_instance().generate_moves(queen, board, moves);

    EXPECT_EQ(moves.size(), 27);
    EXPECT_TRUE(moveExists(moves, "d4", "d1"));
    EXPECT_TRUE(moveExists(moves, "d4", "d8"));
    EXPECT_TRUE(moveExists(moves, "d4", "a4"));
    EXPECT_TRUE(moveExists(moves, "d4", "h4"));
}

TEST_F(MovesTest, BishopPiece)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *bishop = board.addPiece(0, 2, 'B');
    board.addPiece(1, 1, 'P');
    board.addPiece(1, 3, 'P');

    chess::MovesWrapper::get_instance().generate_moves(bishop, board, moves);

    EXPECT_TRUE(moves.empty());
}

TEST_F(MovesTest, QueenPiece)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *queen = board.addPiece(0, 3, 'Q');
    board.addPiece(0, 2, 'B');
    board.addPiece(1, 2, 'P');
    board.addPiece(1, 3, 'P');
    board.addPiece(1, 4, 'P');
    board.addPiece(0, 4, 'K');

    chess::MovesWrapper::get_instance().generate_moves(queen, board, moves);

    EXPECT_TRUE(moves.empty());
}

/* Knight movement tests */
TEST_F(MovesTest, KnightLShapeMovement)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *knight = board.addPiece(3, 3, 'N');

    chess::MovesWrapper::get_instance().generate_moves(knight, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *knight = board.addPiece(3, 3, 'N');

    board.addPiece(4, 5, 'P');
    board.addPiece(2, 5, 'P');
    board.addPiece(5, 4, 'P');
    board.addPiece(1, 4, 'P');
    board.addPiece(5, 2, 'P');
    board.addPiece(1, 2, 'P');
    board.addPiece(4, 1, 'P');
    board.addPiece(2, 1, 'P');

    chess::MovesWrapper::get_instance().generate_moves(knight, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *knight = board.addPiece(3, 3, 'N');
    board.addPiece(4, 5, 'p');
    board.addPiece(2, 5, 'p');
    board.addPiece(5, 4, 'p');
    board.addPiece(1, 4, 'p');
    board.addPiece(5, 2, 'p');
    board.addPiece(1, 2, 'p');
    board.addPiece(4, 1, 'p');
    board.addPiece(2, 1, 'p');

    chess::MovesWrapper::get_instance().generate_moves(knight, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *knight = board.addPiece(7, 0, 'N');

    chess::MovesWrapper::get_instance().generate_moves(knight, board, moves);

    EXPECT_TRUE(moveExists(moves, "a8", "b6"));
    EXPECT_TRUE(moveExists(moves, "a8", "c7"));
    EXPECT_EQ(moves.size(), 2);
}

TEST_F(MovesTest, KnightEdgePosition)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *knight = board.addPiece(3, 0, 'N');

    chess::MovesWrapper::get_instance().generate_moves(knight, board, moves);

    EXPECT_TRUE(moveExists(moves, "a4", "b6"));
    EXPECT_TRUE(moveExists(moves, "a4", "b2"));
    EXPECT_TRUE(moveExists(moves, "a4", "c5"));
    EXPECT_TRUE(moveExists(moves, "a4", "c3"));
    EXPECT_EQ(moves.size(), 4);
}

TEST_F(MovesTest, KnightNearEdgePosition)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *knight = board.addPiece(3, 1, 'N');

    chess::MovesWrapper::get_instance().generate_moves(knight, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *knight = board.addPiece(3, 3, 'n');

    chess::MovesWrapper::get_instance().generate_moves(knight, board, moves);

    EXPECT_EQ(moves.size(), 8);
    EXPECT_TRUE(moveExists(moves, "d4", "f5"));
    EXPECT_TRUE(moveExists(moves, "d4", "f3"));
    EXPECT_TRUE(moveExists(moves, "d4", "e6"));
    EXPECT_TRUE(moveExists(moves, "d4", "e2"));
}

TEST_F(MovesTest, KnightJumpOverPieces)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *knight = board.addPiece(3, 3, 'N');

    board.addPiece(4, 3, 'p');
    board.addPiece(3, 4, 'p');
    board.addPiece(2, 3, 'p');
    board.addPiece(3, 2, 'p');

    chess::MovesWrapper::get_instance().generate_moves(knight, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *king = board.addPiece(0, 4, 'K');
    board.addPiece(0, 3, 'Q');
    board.addPiece(1, 3, 'P');
    board.addPiece(1, 4, 'P');
    board.addPiece(1, 5, 'P');
    board.addPiece(0, 5, 'B');

    chess::MovesWrapper::get_instance().generate_moves(king, board, moves);

    EXPECT_EQ(moves.size(), 0);
}

TEST_F(MovesTest, KingSingleSquareMovement)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *king = board.addPiece(3, 3, 'K');

    chess::MovesWrapper::get_instance().generate_moves(king, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *king = board.addPiece(3, 3, 'K');
    board.addPiece(4, 2, 'P');
    board.addPiece(4, 3, 'P');
    board.addPiece(4, 4, 'P');
    board.addPiece(3, 2, 'P');
    board.addPiece(3, 4, 'P');
    board.addPiece(2, 2, 'P');
    board.addPiece(2, 3, 'P');
    board.addPiece(2, 4, 'P');

    chess::MovesWrapper::get_instance().generate_moves(king, board, moves);

    EXPECT_EQ(moves.size(), 0);
}

TEST_F(MovesTest, KingCaptureEnemyPieces)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *king = board.addPiece(3, 3, 'K');
    board.addPiece(4, 2, 'p');
    board.addPiece(4, 3, 'p');
    board.addPiece(4, 4, 'p');
    board.addPiece(3, 2, 'p');
    board.addPiece(3, 4, 'p');
    board.addPiece(2, 2, 'p');
    board.addPiece(2, 3, 'p');
    board.addPiece(2, 4, 'p');

    chess::MovesWrapper::get_instance().generate_moves(king, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *king = board.addPiece(0, 0, 'K');

    chess::MovesWrapper::get_instance().generate_moves(king, board, moves);

    EXPECT_TRUE(moveExists(moves, "a1", "a2"));
    EXPECT_TRUE(moveExists(moves, "a1", "b1"));
    EXPECT_TRUE(moveExists(moves, "a1", "b2"));

    EXPECT_EQ(moves.size(), 3);
}

TEST_F(MovesTest, KingEdgePosition)
{
    MockBoard board;
    std::vector<chess::Move> moves;
    const chess::Piece *king = board.addPiece(3, 0, 'K');

    chess::MovesWrapper::get_instance().generate_moves(king, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *king = board.addPiece(3, 3, 'k');

    chess::MovesWrapper::get_instance().generate_moves(king, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *king = board.addPiece(3, 3, 'K');
    board.addPiece(3, 2, 'P');
    board.addPiece(3, 5, 'P');
    board.addPiece(2, 4, 'P');
    board.addPiece(4, 2, 'p');

    chess::MovesWrapper::get_instance().generate_moves(king, board, moves);

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
    std::vector<chess::Move> moves;
    const chess::Piece *king = board.addPiece(0, 4, 'K');
    board.addPiece(0, 3, 'Q');
    board.addPiece(1, 3, 'P');
    board.addPiece(1, 4, 'P');
    board.addPiece(1, 5, 'P');
    board.addPiece(0, 5, 'B');

    chess::MovesWrapper::get_instance().generate_moves(king, board, moves);

    EXPECT_EQ(moves.size(), 0);
}

/* Stress test */
TEST_F(MovesTest, StressTestGenerateMoves)
{
    if (!test::get_ci() || std::string(test::get_ci()) != "true")

    {
        GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
    }

    chess::Board board(standard_position);

    for (int i = 0; i < 100000; i++)
    {
        char pawn = board.get_piece(1, 0);
        if (pawn == 'P')
        {
            std::vector<chess::Move> moves;
            ::chess::Moves::generate_moves(1, 0, board, moves);
        }
    }
}

/* =====================================================================
 * New v0.3.0 Moves Tests
 * ===================================================================== */

TEST_F(MovesTest, CastlingKingside_WhiteKing)
{
    MockBoard board;
    board.addPiece(0, 4, 'K'); /* e1 */
    board.addPiece(0, 7, 'R'); /* h1 */
    board.set_castling("KQkq");

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(0, 4, board, moves);

    bool foundKingside = false;
    for (const auto &m : moves)
    {
        if (m.get_move_type() == chess::MoveType::CASTLE_KINGSIDE)
        {
            foundKingside = true;
        }
    }
    EXPECT_TRUE(foundKingside);
}

TEST_F(MovesTest, CastlingQueenside_WhiteKing)
{
    MockBoard board;
    board.addPiece(0, 4, 'K'); /* e1 */
    board.addPiece(0, 0, 'R'); /* a1 */
    board.set_castling("KQkq");

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(0, 4, board, moves);

    bool foundQueenside = false;
    for (const auto &m : moves)
    {
        if (m.get_move_type() == chess::MoveType::CASTLE_QUEENSIDE)
        {
            foundQueenside = true;
        }
    }
    EXPECT_TRUE(foundQueenside);
}

TEST_F(MovesTest, CastlingBlocked_PieceInPath)
{
    MockBoard board;
    board.addPiece(0, 4, 'K'); /* e1 */
    board.addPiece(0, 5, 'B'); /* f1 blocks kingside */
    board.addPiece(0, 7, 'R'); /* h1 */
    board.set_castling("KQkq");

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(0, 4, board, moves);

    bool foundKingside = false;
    for (const auto &m : moves)
    {
        if (m.get_move_type() == chess::MoveType::CASTLE_KINGSIDE)
        {
            foundKingside = true;
        }
    }
    EXPECT_FALSE(foundKingside);
}

TEST_F(MovesTest, CastlingNoCastlingRights)
{
    MockBoard board;
    board.addPiece(0, 4, 'K'); /* e1 */
    board.addPiece(0, 7, 'R'); /* h1 */
    board.set_castling("-");   /* no castling rights */

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(0, 4, board, moves);

    bool foundCastling = false;
    for (const auto &m : moves)
    {
        if (m.is_castling())
        {
            foundCastling = true;
        }
    }
    EXPECT_FALSE(foundCastling);
}

TEST_F(MovesTest, CastlingBlackKing_Kingside)
{
    MockBoard board;
    board.addPiece(7, 4, 'k'); /* e8 */
    board.addPiece(7, 7, 'r'); /* h8 */
    board.set_castling("kq");

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(7, 4, board, moves);

    bool foundKingside = false;
    for (const auto &m : moves)
    {
        if (m.get_move_type() == chess::MoveType::CASTLE_KINGSIDE)
        {
            foundKingside = true;
        }
    }
    EXPECT_TRUE(foundKingside);
}

TEST_F(MovesTest, CastlingBlackKing_Queenside)
{
    MockBoard board;
    board.addPiece(7, 4, 'k'); /* e8 */
    board.addPiece(7, 0, 'r'); /* a8 */
    board.set_castling("kq");

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(7, 4, board, moves);

    bool foundQueenside = false;
    for (const auto &m : moves)
    {
        if (m.get_move_type() == chess::MoveType::CASTLE_QUEENSIDE)
        {
            foundQueenside = true;
        }
    }
    EXPECT_TRUE(foundQueenside);
}

TEST_F(MovesTest, CastlingKing_NotOnStartSquare)
{
    /* King not on e1/e8, no castling generated */
    MockBoard board;
    board.addPiece(3, 4, 'K'); /* King on e4 */
    board.addPiece(3, 7, 'R');
    board.set_castling("KQkq");

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(3, 4, board, moves);

    bool foundCastling = false;
    for (const auto &m : moves)
    {
        if (m.is_castling())
        {
            foundCastling = true;
        }
    }
    EXPECT_FALSE(foundCastling);
}

TEST_F(MovesTest, CastlingKingside_NoRookPresent)
{
    MockBoard board;
    board.addPiece(0, 4, 'K'); /* e1 */
    /* No rook at h1 */
    board.set_castling("KQkq");

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(0, 4, board, moves);

    bool foundKingside = false;
    for (const auto &m : moves)
    {
        if (m.get_move_type() == chess::MoveType::CASTLE_KINGSIDE)
        {
            foundKingside = true;
        }
    }
    EXPECT_FALSE(foundKingside);
}

TEST_F(MovesTest, Promotion_WhitePawnAtRank6)
{
    MockBoard board;
    board.addPiece(6, 4, 'P'); /* White pawn at e7 (rank 6) */

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(6, 4, board, moves);

    int promotionCount = 0;
    for (const auto &m : moves)
    {
        if (m.get_move_type() == chess::MoveType::PROMOTION)
        {
            promotionCount++;
        }
    }
    EXPECT_EQ(promotionCount, 4);

    /* Verify all 4 pieces */
    bool hasQ = false, hasR = false, hasB = false, hasN = false;
    for (const auto &m : moves)
    {
        if (m.get_move_type() == chess::MoveType::PROMOTION)
        {
            char p = m.get_promotion_piece();
            if (p == 'Q')
                hasQ = true;
            if (p == 'R')
                hasR = true;
            if (p == 'B')
                hasB = true;
            if (p == 'N')
                hasN = true;
        }
    }
    EXPECT_TRUE(hasQ);
    EXPECT_TRUE(hasR);
    EXPECT_TRUE(hasB);
    EXPECT_TRUE(hasN);
}

TEST_F(MovesTest, Promotion_BlackPawnAtRank1)
{
    MockBoard board;
    board.addPiece(1, 4, 'p'); /* Black pawn at e2 (rank 1) */

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(1, 4, board, moves);

    int promotionCount = 0;
    for (const auto &m : moves)
    {
        if (m.get_move_type() == chess::MoveType::PROMOTION)
        {
            promotionCount++;
        }
    }
    EXPECT_EQ(promotionCount, 4);
}

TEST_F(MovesTest, Promotion_CapturePromotion)
{
    MockBoard board;
    board.addPiece(6, 4, 'P'); /* White pawn at e7 */
    board.addPiece(7, 3, 'r'); /* Black rook at d8 */
    board.addPiece(7, 5, 'n'); /* Black knight at f8 */

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(6, 4, board, moves);

    /* Should have 4 straight promotions + 4 + 4 capture promotions */
    int promotionCount = 0;
    for (const auto &m : moves)
    {
        if (m.get_move_type() == chess::MoveType::PROMOTION)
        {
            promotionCount++;
        }
    }
    EXPECT_EQ(promotionCount, 12); /* 4 straight + 4 left capture + 4 right capture */
}

TEST_F(MovesTest, Promotion_NotAtPromotionRank)
{
    MockBoard board;
    board.addPiece(3, 4, 'P'); /* Pawn at e4, not promotion rank */

    std::vector<chess::Move> moves;
    ::chess::Moves::generate_moves(3, 4, board, moves);

    for (const auto &m : moves)
    {
        EXPECT_NE(m.get_move_type(), chess::MoveType::PROMOTION);
    }
}

/* Attacks table tests */
TEST_F(MovesTest, AttackTables_KnightAttacks)
{
    chess::init_attack_tables();
    /* Knight at e4 (rank 3, file 4, sq 28) should attack 8 squares if not on edge */
    EXPECT_EQ(__builtin_popcountll(chess::KNIGHT_ATTACKS[28]), 8);
}

TEST_F(MovesTest, AttackTables_KingAttacks)
{
    chess::init_attack_tables();
    /* King at e4 (sq 28) should attack 8 squares */
    EXPECT_EQ(__builtin_popcountll(chess::KING_ATTACKS[28]), 8);
}

TEST_F(MovesTest, AttackTables_KingAtCorner)
{
    chess::init_attack_tables();
    /* King at a1 (sq 0) should attack 3 squares */
    EXPECT_EQ(__builtin_popcountll(chess::KING_ATTACKS[0]), 3);
}

TEST_F(MovesTest, AttackTables_PawnAttacks_White)
{
    chess::init_attack_tables();
    /* White pawn at e4 (sq 28) attacks d5 (sq 35) and f5 (sq 37) */
    EXPECT_EQ(__builtin_popcountll(chess::PAWN_ATTACKS[chess::WHITE][28]), 2);
}

TEST_F(MovesTest, AttackTables_PawnAttacks_Black)
{
    chess::init_attack_tables();
    /* Black pawn at e5 (sq 36) attacks d4 (sq 27) and f4 (sq 29) */
    EXPECT_EQ(__builtin_popcountll(chess::PAWN_ATTACKS[chess::BLACK][36]), 2);
}

TEST_F(MovesTest, AttackTables_RayTable)
{
    chess::init_attack_tables();
    /* Ray north from a1 (sq 0) should have 7 squares (a2-a8) */
    EXPECT_EQ(__builtin_popcountll(chess::RAY[0][chess::RAY_N]), 7);
    /* Ray east from a1 should have 7 squares (b1-h1) */
    EXPECT_EQ(__builtin_popcountll(chess::RAY[0][chess::RAY_E]), 7);
    /* Ray northeast from a1 should have 7 squares (b2-h8) */
    EXPECT_EQ(__builtin_popcountll(chess::RAY[0][chess::RAY_NE]), 7);
    /* No rays south/west/southwest from a1 */
    EXPECT_EQ(__builtin_popcountll(chess::RAY[0][chess::RAY_S]), 0);
    EXPECT_EQ(__builtin_popcountll(chess::RAY[0][chess::RAY_W]), 0);
}
