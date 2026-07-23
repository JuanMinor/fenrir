/*
 *   Copyright (c) 2026 Juan Minor
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "include/chess/move.h"
#include <gtest/gtest.h>

using namespace std;
using namespace chess;

class MoveTest : public ::testing::Test
{
};

TEST_F(MoveTest, NormalMoveConstructor)
{
    chess::Move move("e2", "e4");
    EXPECT_EQ(move.get_from(), "e2");
    EXPECT_EQ(move.get_to(), "e4");
    EXPECT_EQ(move.get_move_type(), chess::MoveType::NORMAL);
    EXPECT_EQ(move.get_promotion_piece(), '\0');
}

TEST_F(MoveTest, CaptureMoveConstructor)
{
    chess::Move move("e4", "d5", chess::MoveType::CAPTURE);
    EXPECT_EQ(move.get_from(), "e4");
    EXPECT_EQ(move.get_to(), "d5");
    EXPECT_EQ(move.get_move_type(), chess::MoveType::CAPTURE);
    EXPECT_EQ(move.get_promotion_piece(), '\0');
}

TEST_F(MoveTest, PromotionMoveConstructor)
{
    chess::Move move("e7", "e8", chess::MoveType::PROMOTION, 'Q');
    EXPECT_EQ(move.get_from(), "e7");
    EXPECT_EQ(move.get_to(), "e8");
    EXPECT_EQ(move.get_move_type(), chess::MoveType::PROMOTION);
    EXPECT_EQ(move.get_promotion_piece(), 'Q');
}

TEST_F(MoveTest, EnPassantMoveConstructor)
{
    chess::Move move("e5", "d6", chess::MoveType::EN_PASSANT);
    EXPECT_EQ(move.get_from(), "e5");
    EXPECT_EQ(move.get_to(), "d6");
    EXPECT_EQ(move.get_move_type(), chess::MoveType::EN_PASSANT);
}

TEST_F(MoveTest, CastleKingsideMoveConstructor)
{
    chess::Move move("e1", "g1", chess::MoveType::CASTLE_KINGSIDE);
    EXPECT_EQ(move.get_from(), "e1");
    EXPECT_EQ(move.get_to(), "g1");
    EXPECT_EQ(move.get_move_type(), chess::MoveType::CASTLE_KINGSIDE);
}

TEST_F(MoveTest, CastleQueensideMoveConstructor)
{
    chess::Move move("e1", "c1", chess::MoveType::CASTLE_QUEENSIDE);
    EXPECT_EQ(move.get_from(), "e1");
    EXPECT_EQ(move.get_to(), "c1");
    EXPECT_EQ(move.get_move_type(), chess::MoveType::CASTLE_QUEENSIDE);
}

TEST_F(MoveTest, IsCaptureForCaptureMove)
{
    chess::Move move("e4", "d5", chess::MoveType::CAPTURE);
    EXPECT_TRUE(move.is_capture());
}

TEST_F(MoveTest, IsCaptureForEnPassant)
{
    chess::Move move("e5", "d6", chess::MoveType::EN_PASSANT);
    EXPECT_TRUE(move.is_capture());
}

TEST_F(MoveTest, IsCaptureForNormalMove)
{
    chess::Move move("e2", "e4");
    EXPECT_FALSE(move.is_capture());
}

TEST_F(MoveTest, IsCaptureForPromotion)
{
    chess::Move move("e7", "e8", chess::MoveType::PROMOTION, 'Q');
    EXPECT_FALSE(move.is_capture());
}

TEST_F(MoveTest, IsPromotionForPromotionMove)
{
    chess::Move move("e7", "e8", chess::MoveType::PROMOTION, 'Q');
    EXPECT_TRUE(move.is_promotion());
}

TEST_F(MoveTest, IsPromotionForNormalMove)
{
    chess::Move move("e2", "e4");
    EXPECT_FALSE(move.is_promotion());
}

TEST_F(MoveTest, IsCastlingForKingsideCastle)
{
    chess::Move move("e1", "g1", chess::MoveType::CASTLE_KINGSIDE);
    EXPECT_TRUE(move.is_castling());
}

TEST_F(MoveTest, IsCastlingForQueensideCastle)
{
    chess::Move move("e1", "c1", chess::MoveType::CASTLE_QUEENSIDE);
    EXPECT_TRUE(move.is_castling());
}

TEST_F(MoveTest, IsCastlingForNormalMove)
{
    chess::Move move("e2", "e4");
    EXPECT_FALSE(move.is_castling());
}

TEST_F(MoveTest, ToStringNormalMove)
{
    chess::Move move("e2", "e4");
    EXPECT_EQ(move.to_string(), "(Normal move)e2->e4");
}

TEST_F(MoveTest, ToStringCaptureMove)
{
    chess::Move move("e4", "d5", chess::MoveType::CAPTURE);
    EXPECT_EQ(move.to_string(), "(Capture)e4->d5");
}

TEST_F(MoveTest, ToStringEnPassantMove)
{
    chess::Move move("e5", "d6", chess::MoveType::EN_PASSANT);
    EXPECT_EQ(move.to_string(), "(En passant)e5->d6");
}

TEST_F(MoveTest, ToStringCastleKingside)
{
    chess::Move move("e1", "g1", chess::MoveType::CASTLE_KINGSIDE);
    EXPECT_EQ(move.to_string(), "(Castling king side)e1->g1");
}

TEST_F(MoveTest, ToStringCastleQueenside)
{
    chess::Move move("e1", "c1", chess::MoveType::CASTLE_QUEENSIDE);
    EXPECT_EQ(move.to_string(), "(Castling queen side)e1->c1");
}

TEST_F(MoveTest, ToStringPromotionQueen)
{
    chess::Move move("e7", "e8", chess::MoveType::PROMOTION, 'Q');
    EXPECT_EQ(move.to_string(), "(Promotion to Q)e7->e8");
}

TEST_F(MoveTest, ToStringPromotionRook)
{
    chess::Move move("a7", "a8", chess::MoveType::PROMOTION, 'R');
    EXPECT_EQ(move.to_string(), "(Promotion to R)a7->a8");
}

TEST_F(MoveTest, ToStringPromotionBishop)
{
    chess::Move move("b7", "b8", chess::MoveType::PROMOTION, 'B');
    EXPECT_EQ(move.to_string(), "(Promotion to B)b7->b8");
}

TEST_F(MoveTest, ToStringPromotionKnight)
{
    chess::Move move("c7", "c8", chess::MoveType::PROMOTION, 'N');
    EXPECT_EQ(move.to_string(), "(Promotion to N)c7->c8");
}

TEST_F(MoveTest, ToStringPromotionNoPiece)
{
    chess::Move move("d7", "d8", chess::MoveType::PROMOTION, '\0');
    EXPECT_EQ(move.to_string(), "(Promotion)d7->d8");
}

TEST_F(MoveTest, ToUCINotationNormalMove)
{
    chess::Move move("e2", "e4");
    EXPECT_EQ(move.to_uci_notation(), "e2e4");
}

TEST_F(MoveTest, ToUCINotationCaptureMove)
{
    chess::Move move("e4", "d5", chess::MoveType::CAPTURE);
    EXPECT_EQ(move.to_uci_notation(), "e4d5");
}

TEST_F(MoveTest, ToUCINotationEnPassant)
{
    chess::Move move("e5", "d6", chess::MoveType::EN_PASSANT);
    EXPECT_EQ(move.to_uci_notation(), "e5d6");
}

TEST_F(MoveTest, ToUCINotationCastleKingside)
{
    chess::Move move("e1", "g1", chess::MoveType::CASTLE_KINGSIDE);
    EXPECT_EQ(move.to_uci_notation(), "e1g1");
}

TEST_F(MoveTest, ToUCINotationCastleQueenside)
{
    chess::Move move("e1", "c1", chess::MoveType::CASTLE_QUEENSIDE);
    EXPECT_EQ(move.to_uci_notation(), "e1c1");
}

TEST_F(MoveTest, ToUCINotationPromotionQueen)
{
    chess::Move move("e7", "e8", chess::MoveType::PROMOTION, 'Q');
    EXPECT_EQ(move.to_uci_notation(), "e7e8q");
}

TEST_F(MoveTest, ToUCINotationPromotionRook)
{
    chess::Move move("a7", "a8", chess::MoveType::PROMOTION, 'R');
    EXPECT_EQ(move.to_uci_notation(), "a7a8r");
}

TEST_F(MoveTest, ToUCINotationPromotionBishop)
{
    chess::Move move("b7", "b8", chess::MoveType::PROMOTION, 'B');
    EXPECT_EQ(move.to_uci_notation(), "b7b8b");
}

TEST_F(MoveTest, ToUCINotationPromotionKnight)
{
    chess::Move move("c7", "c8", chess::MoveType::PROMOTION, 'N');
    EXPECT_EQ(move.to_uci_notation(), "c7c8n");
}

TEST_F(MoveTest, ToAlgebraicNotationNormalMove)
{
    chess::Move move("e2", "e4");
    EXPECT_EQ(move.to_algebraic_notation(), "e2e4");
}

TEST_F(MoveTest, ToAlgebraicNotationCaptureMove)
{
    chess::Move move("e4", "d5", chess::MoveType::CAPTURE);
    EXPECT_EQ(move.to_algebraic_notation(), "e4d5");
}

TEST_F(MoveTest, ToAlgebraicNotationEnPassant)
{
    chess::Move move("e5", "d6", chess::MoveType::EN_PASSANT);
    EXPECT_EQ(move.to_algebraic_notation(), "e5d6");
}

TEST_F(MoveTest, ToAlgebraicNotationCastleKingside)
{
    chess::Move move("e1", "g1", chess::MoveType::CASTLE_KINGSIDE);
    EXPECT_EQ(move.to_algebraic_notation(), "e1g1");
}

TEST_F(MoveTest, ToAlgebraicNotationCastleQueenside)
{
    chess::Move move("e1", "c1", chess::MoveType::CASTLE_QUEENSIDE);
    EXPECT_EQ(move.to_algebraic_notation(), "e1c1");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionQueen)
{
    chess::Move move("e7", "e8", chess::MoveType::PROMOTION, 'Q');
    EXPECT_EQ(move.to_algebraic_notation(), "e7e8");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionRook)
{
    chess::Move move("a7", "a8", chess::MoveType::PROMOTION, 'R');
    EXPECT_EQ(move.to_algebraic_notation(), "a7a8");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionBishop)
{
    chess::Move move("b7", "b8", chess::MoveType::PROMOTION, 'B');
    EXPECT_EQ(move.to_algebraic_notation(), "b7b8");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionKnight)
{
    chess::Move move("c7", "c8", chess::MoveType::PROMOTION, 'N');
    EXPECT_EQ(move.to_algebraic_notation(), "c7c8");
}

TEST_F(MoveTest, EmptySquareNames)
{
    chess::Move move("", "");
    EXPECT_EQ(move.get_from(), "");
    EXPECT_EQ(move.get_to(), "");
}

TEST_F(MoveTest, LongSquareNames)
{
    chess::Move move("e2e2e2", "e4e4e4");
    EXPECT_EQ(move.get_from(), "e2e2e2");
    EXPECT_EQ(move.get_to(), "e4e4e4");
}

TEST_F(MoveTest, SpecialCharactersInSquareNames)
{
    chess::Move move("e-2", "e-4");
    EXPECT_EQ(move.get_from(), "e-2");
    EXPECT_EQ(move.get_to(), "e-4");
}

TEST_F(MoveTest, ToStringInvalidMoveType)
{
    chess::Move move("e2", "e4", static_cast<chess::MoveType>(99));
    std::string result = move.to_string();
    EXPECT_NE(result.find("Invalid action"), std::string::npos);
}

TEST_F(MoveTest, CopySemanticsWithInvalidSquares)
{
    chess::Move move1("", "");
    chess::Move move2(move1);
    EXPECT_EQ(move2.get_from(), "");
    EXPECT_EQ(move2.get_to(), "");

    chess::Move move3("e2", "e4");
    move3 = move1;
    EXPECT_EQ(move3.get_from(), "");
    EXPECT_EQ(move3.get_to(), "");

    chess::Move move4("a7", "a8");
    chess::Move move5("h2", "h4");
    move5 = move4;
    EXPECT_EQ(move5.get_from(), "a7");
    EXPECT_EQ(move5.get_to(), "a8");

    move5 = move5;
    EXPECT_EQ(move5.get_from(), "a7");
    EXPECT_EQ(move5.get_to(), "a8");
}

TEST_F(MoveTest, GetFromAndToSquares)
{
    chess::Move move("e2", "e4");
    EXPECT_EQ(move.get_from_square(), 12);
    EXPECT_EQ(move.get_to_square(), 28);
}

TEST_F(MoveTest, ConstructorSquareIndexAndPromotion)
{
    chess::Move move(12, 28, 'Q');
    EXPECT_EQ(move.get_from_square(), 12);
    EXPECT_EQ(move.get_to_square(), 28);
    EXPECT_EQ(move.get_promotion_piece(), 'Q');
    EXPECT_EQ(move.get_move_type(), chess::MoveType::PROMOTION);
}
