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
	EXPECT_EQ(move.getFrom(), "e2");
	EXPECT_EQ(move.getTo(), "e4");
	EXPECT_EQ(move.getMoveType(), MoveType::NORMAL);
	EXPECT_EQ(move.getPromotionPiece(), '\0');
}

TEST_F(MoveTest, CaptureMoveConstructor)
{
	Move move("e4", "d5", MoveType::CAPTURE);
	EXPECT_EQ(move.getFrom(), "e4");
	EXPECT_EQ(move.getTo(), "d5");
	EXPECT_EQ(move.getMoveType(), MoveType::CAPTURE);
	EXPECT_EQ(move.getPromotionPiece(), '\0');
}

TEST_F(MoveTest, PromotionMoveConstructor)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_EQ(move.getFrom(), "e7");
	EXPECT_EQ(move.getTo(), "e8");
	EXPECT_EQ(move.getMoveType(), MoveType::PROMOTION);
	EXPECT_EQ(move.getPromotionPiece(), 'Q');
}

TEST_F(MoveTest, EnPassantMoveConstructor)
{
	Move move("e5", "d6", MoveType::EN_PASSANT);
	EXPECT_EQ(move.getFrom(), "e5");
	EXPECT_EQ(move.getTo(), "d6");
	EXPECT_EQ(move.getMoveType(), MoveType::EN_PASSANT);
}

TEST_F(MoveTest, CastleKingsideMoveConstructor)
{
	Move move("e1", "g1", MoveType::CASTLE_KINGSIDE);
	EXPECT_EQ(move.getFrom(), "e1");
	EXPECT_EQ(move.getTo(), "g1");
	EXPECT_EQ(move.getMoveType(), MoveType::CASTLE_KINGSIDE);
}

TEST_F(MoveTest, CastleQueensideMoveConstructor)
{
	Move move("e1", "c1", MoveType::CASTLE_QUEENSIDE);
	EXPECT_EQ(move.getFrom(), "e1");
	EXPECT_EQ(move.getTo(), "c1");
	EXPECT_EQ(move.getMoveType(), MoveType::CASTLE_QUEENSIDE);
}

TEST_F(MoveTest, IsCaptureForCaptureMove)
{
	Move move("e4", "d5", MoveType::CAPTURE);
	EXPECT_TRUE(move.isCapture());
}

TEST_F(MoveTest, IsCaptureForEnPassant)
{
	Move move("e5", "d6", MoveType::EN_PASSANT);
	EXPECT_TRUE(move.isCapture());
}

TEST_F(MoveTest, IsCaptureForNormalMove)
{
	Move move("e2", "e4");
	EXPECT_FALSE(move.isCapture());
}

TEST_F(MoveTest, IsCaptureForPromotion)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_FALSE(move.isCapture());
}

TEST_F(MoveTest, IsPromotionForPromotionMove)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_TRUE(move.isPromotion());
}

TEST_F(MoveTest, IsPromotionForNormalMove)
{
	Move move("e2", "e4");
	EXPECT_FALSE(move.isPromotion());
}

TEST_F(MoveTest, IsCastlingForKingsideCastle)
{
	Move move("e1", "g1", MoveType::CASTLE_KINGSIDE);
	EXPECT_TRUE(move.isCastling());
}

TEST_F(MoveTest, IsCastlingForQueensideCastle)
{
	Move move("e1", "c1", MoveType::CASTLE_QUEENSIDE);
	EXPECT_TRUE(move.isCastling());
}

TEST_F(MoveTest, IsCastlingForNormalMove)
{
	Move move("e2", "e4");
	EXPECT_FALSE(move.isCastling());
}

TEST_F(MoveTest, ToStringNormalMove)
{
	Move move("e2", "e4");
	EXPECT_EQ(move.toString(), "(Normal move)e2->e4");
}

TEST_F(MoveTest, ToStringCaptureMove)
{
	Move move("e4", "d5", MoveType::CAPTURE);
	EXPECT_EQ(move.toString(), "(Capture)e4->d5");
}

TEST_F(MoveTest, ToStringEnPassantMove)
{
	Move move("e5", "d6", MoveType::EN_PASSANT);
	EXPECT_EQ(move.toString(), "(En passant)e5->d6");
}

TEST_F(MoveTest, ToStringCastleKingside)
{
	Move move("e1", "g1", MoveType::CASTLE_KINGSIDE);
	EXPECT_EQ(move.toString(), "(Castling king side)e1->g1");
}

TEST_F(MoveTest, ToStringCastleQueenside)
{
	Move move("e1", "c1", MoveType::CASTLE_QUEENSIDE);
	EXPECT_EQ(move.toString(), "(Castling queen side)e1->c1");
}

TEST_F(MoveTest, ToStringPromotionQueen)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_EQ(move.toString(), "(Promotion to Q)e7->e8");
}

TEST_F(MoveTest, ToStringPromotionRook)
{
	Move move("a7", "a8", MoveType::PROMOTION, 'R');
	EXPECT_EQ(move.toString(), "(Promotion to R)a7->a8");
}

TEST_F(MoveTest, ToStringPromotionBishop)
{
	Move move("b7", "b8", MoveType::PROMOTION, 'B');
	EXPECT_EQ(move.toString(), "(Promotion to B)b7->b8");
}

TEST_F(MoveTest, ToStringPromotionKnight)
{
	Move move("c7", "c8", MoveType::PROMOTION, 'N');
	EXPECT_EQ(move.toString(), "(Promotion to N)c7->c8");
}

TEST_F(MoveTest, ToStringPromotionNoPiece)
{
	Move move("d7", "d8", MoveType::PROMOTION, '\0');
	EXPECT_EQ(move.toString(), "(Promotion)d7->d8");
}

TEST_F(MoveTest, ToUCINotationNormalMove)
{
	Move move("e2", "e4");
	EXPECT_EQ(move.toUCINotation(), "e2e4");
}

TEST_F(MoveTest, ToUCINotationCaptureMove)
{
	Move move("e4", "d5", MoveType::CAPTURE);
	EXPECT_EQ(move.toUCINotation(), "e4d5");
}

TEST_F(MoveTest, ToUCINotationEnPassant)
{
	Move move("e5", "d6", MoveType::EN_PASSANT);
	EXPECT_EQ(move.toUCINotation(), "e5d6");
}

TEST_F(MoveTest, ToUCINotationCastleKingside)
{
	Move move("e1", "g1", MoveType::CASTLE_KINGSIDE);
	EXPECT_EQ(move.toUCINotation(), "e1g1");
}

TEST_F(MoveTest, ToUCINotationCastleQueenside)
{
	Move move("e1", "c1", MoveType::CASTLE_QUEENSIDE);
	EXPECT_EQ(move.toUCINotation(), "e1c1");
}

TEST_F(MoveTest, ToUCINotationPromotionQueen)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_EQ(move.toUCINotation(), "e7e8q");
}

TEST_F(MoveTest, ToUCINotationPromotionRook)
{
	Move move("a7", "a8", MoveType::PROMOTION, 'R');
	EXPECT_EQ(move.toUCINotation(), "a7a8r");
}

TEST_F(MoveTest, ToUCINotationPromotionBishop)
{
	Move move("b7", "b8", MoveType::PROMOTION, 'B');
	EXPECT_EQ(move.toUCINotation(), "b7b8b");
}

TEST_F(MoveTest, ToUCINotationPromotionKnight)
{
	Move move("c7", "c8", MoveType::PROMOTION, 'N');
	EXPECT_EQ(move.toUCINotation(), "c7c8n");
}

TEST_F(MoveTest, ToAlgebraicNotationNormalMove)
{
	Move move("e2", "e4");
	EXPECT_EQ(move.toAlgebraicNotation(), "e2e4");
}

TEST_F(MoveTest, ToAlgebraicNotationCaptureMove)
{
	Move move("e4", "d5", MoveType::CAPTURE);
	EXPECT_EQ(move.toAlgebraicNotation(), "e4d5");
}

TEST_F(MoveTest, ToAlgebraicNotationEnPassant)
{
	Move move("e5", "d6", MoveType::EN_PASSANT);
	EXPECT_EQ(move.toAlgebraicNotation(), "e5d6");
}

TEST_F(MoveTest, ToAlgebraicNotationCastleKingside)
{
	Move move("e1", "g1", MoveType::CASTLE_KINGSIDE);
	EXPECT_EQ(move.toAlgebraicNotation(), "e1g1");
}

TEST_F(MoveTest, ToAlgebraicNotationCastleQueenside)
{
	Move move("e1", "c1", MoveType::CASTLE_QUEENSIDE);
	EXPECT_EQ(move.toAlgebraicNotation(), "e1c1");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionQueen)
{
	Move move("e7", "e8", MoveType::PROMOTION, 'Q');
	EXPECT_EQ(move.toAlgebraicNotation(), "e7e8");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionRook)
{
	Move move("a7", "a8", MoveType::PROMOTION, 'R');
	EXPECT_EQ(move.toAlgebraicNotation(), "a7a8");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionBishop)
{
	Move move("b7", "b8", MoveType::PROMOTION, 'B');
	EXPECT_EQ(move.toAlgebraicNotation(), "b7b8");
}

TEST_F(MoveTest, ToAlgebraicNotationPromotionKnight)
{
	Move move("c7", "c8", MoveType::PROMOTION, 'N');
	EXPECT_EQ(move.toAlgebraicNotation(), "c7c8");
}

TEST_F(MoveTest, EmptySquareNames)
{
	Move move("", "");
	EXPECT_EQ(move.getFrom(), "");
	EXPECT_EQ(move.getTo(), "");
}

TEST_F(MoveTest, LongSquareNames)
{
	Move move("e2e2e2", "e4e4e4");
	EXPECT_EQ(move.getFrom(), "e2e2e2");
	EXPECT_EQ(move.getTo(), "e4e4e4");
}

TEST_F(MoveTest, SpecialCharactersInSquareNames)
{
	Move move("e-2", "e-4");
	EXPECT_EQ(move.getFrom(), "e-2");
	EXPECT_EQ(move.getTo(), "e-4");
}

TEST_F(MoveTest, ToStringInvalidMoveType)
{
	Move move("e2", "e4", static_cast<MoveType>(999));
	std::string result = move.toString();
	EXPECT_NE(result.find("Invalid action"), std::string::npos);
}
