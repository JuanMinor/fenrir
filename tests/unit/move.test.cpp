#include "include/chess/move.h"
#include <gtest/gtest.h>

using namespace std;
using namespace fenrir;

class MoveTest : public ::testing::Test
{
};

TEST_F(MoveTest, NormalMoveConstructor)
{
	Move move("e2", "e4");
	EXPECT_EQ(move.get_from(), "e2");
	EXPECT_EQ(move.get_to(), "e4");
	EXPECT_EQ(move.get_move_type(), MoveType::NORMAL);
	EXPECT_EQ(move.get_promotion_piece(), '\0');
}

TEST_F(MoveTest, CaptureMoveConstructor)
{
	Move move("e4", "d5", MoveType::CAPTURE);
	EXPECT_EQ(move.get_from(), "e4");
	EXPECT_EQ(move.get_to(), "d5");
	EXPECT_EQ(move.get_move_type(), MoveType::CAPTURE);
	EXPECT_EQ(move.get_promotion_piece(), '\0');
}

TEST_F(MoveTest, PromotionMoveConstructor)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_EQ(move.get_from(), "e7");
	EXPECT_EQ(move.get_to(), "e8");
	EXPECT_EQ(move.get_move_type(), MoveType::PROMOTION);
	EXPECT_EQ(move.get_promotion_piece(), 'Q');
}

TEST_F(MoveTest, EnPassantMoveConstructor)
{
	Move move("e5", "d6", MoveType::EN_PASSANT);
	EXPECT_EQ(move.get_from(), "e5");
	EXPECT_EQ(move.get_to(), "d6");
	EXPECT_EQ(move.get_move_type(), MoveType::EN_PASSANT);
}

TEST_F(MoveTest, CastleKingsideMoveConstructor)
{
	Move move("e1", "g1", MoveType::CASTLE_KINGSIDE);
	EXPECT_EQ(move.get_from(), "e1");
	EXPECT_EQ(move.get_to(), "g1");
	EXPECT_EQ(move.get_move_type(), MoveType::CASTLE_KINGSIDE);
}

TEST_F(MoveTest, CastleQueensideMoveConstructor)
{
	Move move("e1", "c1", MoveType::CASTLE_QUEENSIDE);
	EXPECT_EQ(move.get_from(), "e1");
	EXPECT_EQ(move.get_to(), "c1");
	EXPECT_EQ(move.get_move_type(), MoveType::CASTLE_QUEENSIDE);
}

TEST_F(MoveTest, IsCaptureForCaptureMove)
{
	Move move("e4", "d5", MoveType::CAPTURE);
	EXPECT_TRUE(move.is_capture());
}

TEST_F(MoveTest, IsCaptureForEnPassant)
{
	Move move("e5", "d6", MoveType::EN_PASSANT);
	EXPECT_TRUE(move.is_capture());
}

TEST_F(MoveTest, IsCaptureForNormalMove)
{
	Move move("e2", "e4");
	EXPECT_FALSE(move.is_capture());
}

TEST_F(MoveTest, IsCaptureForPromotion)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_FALSE(move.is_capture());
}

TEST_F(MoveTest, IsPromotionForPromotionMove)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_TRUE(move.is_promotion());
}

TEST_F(MoveTest, IsPromotionForNormalMove)
{
	Move move("e2", "e4");
	EXPECT_FALSE(move.is_promotion());
}

TEST_F(MoveTest, IsCastlingForKingsideCastle)
{
	Move move("e1", "g1", MoveType::CASTLE_KINGSIDE);
	EXPECT_TRUE(move.is_castling());
}

TEST_F(MoveTest, IsCastlingForQueensideCastle)
{
	Move move("e1", "c1", MoveType::CASTLE_QUEENSIDE);
	EXPECT_TRUE(move.is_castling());
}

TEST_F(MoveTest, IsCastlingForNormalMove)
{
	Move move("e2", "e4");
	EXPECT_FALSE(move.is_castling());
}

TEST_F(MoveTest, ToStringNormalMove)
{
	Move move("e2", "e4");
	EXPECT_EQ(move.to_string(), "(Normal move)e2->e4");
}

TEST_F(MoveTest, ToStringCaptureMove)
{
	Move move("e4", "d5", MoveType::CAPTURE);
	EXPECT_EQ(move.to_string(), "(Capture)e4->d5");
}

TEST_F(MoveTest, ToStringEnPassantMove)
{
	Move move("e5", "d6", MoveType::EN_PASSANT);
	EXPECT_EQ(move.to_string(), "(En passant)e5->d6");
}

TEST_F(MoveTest, ToStringCastleKingside)
{
	Move move("e1", "g1", MoveType::CASTLE_KINGSIDE);
	EXPECT_EQ(move.to_string(), "(Castling king side)e1->g1");
}

TEST_F(MoveTest, ToStringCastleQueenside)
{
	Move move("e1", "c1", MoveType::CASTLE_QUEENSIDE);
	EXPECT_EQ(move.to_string(), "(Castling queen side)e1->c1");
}

TEST_F(MoveTest, ToStringPromotionQueen)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_EQ(move.to_string(), "(Promotion to Q)e7->e8");
}

TEST_F(MoveTest, ToStringPromotionRook)
{
	Move move("a7", "a8", MoveType::PROMOTION, 'R');
	EXPECT_EQ(move.to_string(), "(Promotion to R)a7->a8");
}

TEST_F(MoveTest, ToStringPromotionBishop)
{
	Move move("b7", "b8", MoveType::PROMOTION, 'B');
	EXPECT_EQ(move.to_string(), "(Promotion to B)b7->b8");
}

TEST_F(MoveTest, ToStringPromotionKnight)
{
	Move move("c7", "c8", MoveType::PROMOTION, 'N');
	EXPECT_EQ(move.to_string(), "(Promotion to N)c7->c8");
}

TEST_F(MoveTest, ToStringPromotionNoPiece)
{
	Move move("d7", "d8", MoveType::PROMOTION, '\0');
	EXPECT_EQ(move.to_string(), "(Promotion)d7->d8");
}

TEST_F(MoveTest, ToUCINotationNormalMove)
{
	Move move("e2", "e4");
	EXPECT_EQ(move.to_uci_notation(), "e2e4");
}

TEST_F(MoveTest, ToUCINotationCaptureMove)
{
	Move move("e4", "d5", MoveType::CAPTURE);
	EXPECT_EQ(move.to_uci_notation(), "e4d5");
}

TEST_F(MoveTest, ToUCINotationEnPassant)
{
	Move move("e5", "d6", MoveType::EN_PASSANT);
	EXPECT_EQ(move.to_uci_notation(), "e5d6");
}

TEST_F(MoveTest, ToUCINotationCastleKingside)
{
	Move move("e1", "g1", MoveType::CASTLE_KINGSIDE);
	EXPECT_EQ(move.to_uci_notation(), "e1g1");
}

TEST_F(MoveTest, ToUCINotationCastleQueenside)
{
	Move move("e1", "c1", MoveType::CASTLE_QUEENSIDE);
	EXPECT_EQ(move.to_uci_notation(), "e1c1");
}

TEST_F(MoveTest, ToUCINotationPromotionQueen)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_EQ(move.to_uci_notation(), "e7e8q");
}

TEST_F(MoveTest, ToUCINotationPromotionRook)
{
	Move move("a7", "a8", MoveType::PROMOTION, 'R');
	EXPECT_EQ(move.to_uci_notation(), "a7a8r");
}

TEST_F(MoveTest, ToUCINotationPromotionBishop)
{
	Move move("b7", "b8", MoveType::PROMOTION, 'B');
	EXPECT_EQ(move.to_uci_notation(), "b7b8b");
}

TEST_F(MoveTest, ToUCINotationPromotionKnight)
{
	Move move("c7", "c8", MoveType::PROMOTION, 'N');
	EXPECT_EQ(move.to_uci_notation(), "c7c8n");
}

TEST_F(MoveTest, ToAlgebraicNotationNormalMove)
{
	Move move("e2", "e4");
	EXPECT_EQ(move.to_algebraic_notation(), "e2e4");
}

TEST_F(MoveTest, ToAlgebraicNotationCaptureMove)
{
	Move move("e4", "d5", MoveType::CAPTURE);
	EXPECT_EQ(move.to_algebraic_notation(), "e4d5");
}

TEST_F(MoveTest, ToAlgebraicNotationEnPassant)
{
	Move move("e5", "d6", MoveType::EN_PASSANT);
	EXPECT_EQ(move.to_algebraic_notation(), "e5d6");
}

TEST_F(MoveTest, ToAlgebraicNotationCastleKingside)
{
	Move move("e1", "g1", MoveType::CASTLE_KINGSIDE);
	EXPECT_EQ(move.to_algebraic_notation(), "e1g1");
}

TEST_F(MoveTest, ToAlgebraicNotationCastleQueenside)
{
	Move move("e1", "c1", MoveType::CASTLE_QUEENSIDE);
	EXPECT_EQ(move.to_algebraic_notation(), "e1c1");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionQueen)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_EQ(move.to_algebraic_notation(), "e7e8");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionRook)
{
	Move move("a7", "a8", MoveType::PROMOTION, 'R');
	EXPECT_EQ(move.to_algebraic_notation(), "a7a8");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionBishop)
{
	Move move("b7", "b8", MoveType::PROMOTION, 'B');
	EXPECT_EQ(move.to_algebraic_notation(), "b7b8");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionKnight)
{
	Move move("c7", "c8", MoveType::PROMOTION, 'N');
	EXPECT_EQ(move.to_algebraic_notation(), "c7c8");
}

TEST_F(MoveTest, EmptySquareNames)
{
	Move move("", "");
	EXPECT_EQ(move.get_from(), "");
	EXPECT_EQ(move.get_to(), "");
}

TEST_F(MoveTest, LongSquareNames)
{
	Move move("e2e2e2", "e4e4e4");
	EXPECT_EQ(move.get_from(), "e2e2e2");
	EXPECT_EQ(move.get_to(), "e4e4e4");
}

TEST_F(MoveTest, SpecialCharactersInSquareNames)
{
	Move move("e-2", "e-4");
	EXPECT_EQ(move.get_from(), "e-2");
	EXPECT_EQ(move.get_to(), "e-4");
}

TEST_F(MoveTest, ToStringInvalidMoveType)
{
	Move move("e2", "e4", static_cast<MoveType>(99));
	std::string result = move.to_string();
	EXPECT_NE(result.find("Invalid action"), std::string::npos);
}

TEST_F(MoveTest, CopySemanticsWithInvalidSquares)
{
	Move move1("", "");
	Move move2(move1);
	EXPECT_EQ(move2.get_from(), "");
	EXPECT_EQ(move2.get_to(), "");

	Move move3("e2", "e4");
	move3 = move1;
	EXPECT_EQ(move3.get_from(), "");
	EXPECT_EQ(move3.get_to(), "");

	Move move4("a7", "a8");
	Move move5("h2", "h4");
	move5 = move4;
	EXPECT_EQ(move5.get_from(), "a7");
	EXPECT_EQ(move5.get_to(), "a8");

	move5 = move5;
	EXPECT_EQ(move5.get_from(), "a7");
	EXPECT_EQ(move5.get_to(), "a8");
}

TEST_F(MoveTest, GetFromAndToSquares)
{
	Move move("e2", "e4");
	EXPECT_EQ(move.get_from_square(), 12);
	EXPECT_EQ(move.get_to_square(), 28);
}

TEST_F(MoveTest, ConstructorSquareIndexAndPromotion)
{
	Move move(12, 28, 'Q');
	EXPECT_EQ(move.get_from_square(), 12);
	EXPECT_EQ(move.get_to_square(), 28);
	EXPECT_EQ(move.get_promotion_piece(), 'Q');
	EXPECT_EQ(move.get_move_type(), MoveType::PROMOTION);
}

