/*
 *   Copyright (c) 2025 Juan Minor
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

#include <gtest/gtest.h>
#include <regex>
#include "include/chess/board.h"
#include "include/core/core.h"
#include "include/utils/utils.h"

class BoardTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		valid_fen_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq a3 0 1";
	}

	static void TearDownTestSuite() {}

	static std::string valid_fen_position;
};

std::string BoardTest::valid_fen_position = "";

/* Parsing tests */
TEST_F(BoardTest, ParseValidFENPosition)
{
	fenrir::Board board(valid_fen_position);

	/* Check pieces via get_piece() */
	/* White rook */
	char rook = board.get_piece(0, 0);
	EXPECT_EQ(rook, 'R');

	/* Black king */
	char king = board.get_piece(7, 4);
	EXPECT_EQ(king, 'k');
}

TEST_F(BoardTest, SetEnPassant)
{
	fenrir::Board board(valid_fen_position);

	EXPECT_EQ(board.get_en_passant(), "a3");
}

TEST_F(BoardTest, GetPiece_AtValidPosition)
{
	fenrir::Board board(valid_fen_position);
	char piece = board.get_piece(0, 0);

	EXPECT_EQ(piece, 'R');
}

TEST_F(BoardTest, GetPiece_AtInvalidPosition)
{
	fenrir::Board board(valid_fen_position);
	char piece = board.get_piece(8, 0);

	EXPECT_EQ(piece, '\0');
}

/* Move tests */
TEST_F(BoardTest, MovePiece)
{
	fenrir::Board board(valid_fen_position);

	/* Move the pawn from a2 to a4 */
	board.apply_move(fenrir::Move("a2", "a4"));

	EXPECT_EQ(board.get_piece(3, 0), 'P');
	EXPECT_EQ(board.get_piece(1, 0), '\0');
}

TEST_F(BoardTest, MovePawnAndEnPassantIsSet)
{
	fenrir::Board board(valid_fen_position);

	/* Move the pawn to a4 */
	board.apply_move(fenrir::Move("a2", "a4"));

	EXPECT_EQ(board.get_en_passant(), "a3");
}

TEST_F(BoardTest, MovePawnAndEnPassantIsCleared)
{
	fenrir::Board board(valid_fen_position);

	/* Move the pawn to a4 */
	board.apply_move(fenrir::Move("a2", "a4"));

	/* Move the pawn to a5 */
	board.apply_move(fenrir::Move("a4", "a5"));

	EXPECT_EQ(board.get_en_passant(), "");
}

/* Error handling tests for FEN parsing */
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

/* FEN generation tests */
TEST_F(BoardTest, GetFenAfterCreation)
{
	fenrir::Board board(valid_fen_position);

	EXPECT_EQ(board.get_fen(), valid_fen_position);
}

TEST_F(BoardTest, GetFenAfterMove)
{
	fenrir::Board board("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
	board.apply_move(fenrir::Move("d7", "d5"));
	EXPECT_EQ(board.get_fen(), "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
}

TEST_F(BoardTest, GetFenAfterPawnDoubleMove)
{
	fenrir::Board board(valid_fen_position);
	board.apply_move(fenrir::Move("a2", "a4"));
	EXPECT_EQ(board.get_fen(), "rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR b KQkq a3 0 1");
}

TEST_F(BoardTest, CapturePiece)
{
	fenrir::Board board("4k3/8/8/4p3/3P4/8/8/4K3 w - - 0 1");
	EXPECT_EQ(board.get_piece(3, 3), 'P');
	EXPECT_EQ(board.get_piece(4, 4), 'p');

	board.apply_move(fenrir::Move("d4", "e5", fenrir::MoveType::CAPTURE));

	EXPECT_EQ(board.get_piece(3, 3), '\0');
	EXPECT_EQ(board.get_piece(4, 4), 'P');
}

TEST_F(BoardTest, CapturePieceBlackCapturesWhite)
{
	fenrir::Board board("4k3/8/8/4p3/3P4/8/8/4K3 b - - 0 1");
	EXPECT_EQ(board.get_piece(3, 3), 'P');
	EXPECT_EQ(board.get_piece(4, 4), 'p');

	board.apply_move(fenrir::Move("e5", "d4", fenrir::MoveType::CAPTURE));

	EXPECT_EQ(board.get_piece(4, 4), '\0');
	EXPECT_EQ(board.get_piece(3, 3), 'p');
}

/* =====================================================================
 * New v0.3.0 Board Tests
 * ===================================================================== */

TEST_F(BoardTest, GetColor_Initial)
{
	fenrir::Board board(valid_fen_position);
	EXPECT_EQ(board.get_color(), fenrir::WHITE);
}

TEST_F(BoardTest, GetCastlingRights_Initial)
{
	fenrir::Board board(valid_fen_position);
	EXPECT_EQ(board.get_castling_rights(), "KQkq");
}

TEST_F(BoardTest, GetCastlingRights_NoCastling)
{
	fenrir::Board board("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
	EXPECT_EQ(board.get_castling_rights(), "-");
}

TEST_F(BoardTest, GetOccupancy_White)
{
	fenrir::Board board(valid_fen_position);
	uint64_t whiteOcc = board.get_occupancy(fenrir::WHITE);
	EXPECT_NE(whiteOcc, 0ULL);
	EXPECT_EQ(whiteOcc, board.get_white_occupancy());
}

TEST_F(BoardTest, GetOccupancy_Black)
{
	fenrir::Board board(valid_fen_position);
	uint64_t blackOcc = board.get_occupancy(fenrir::BLACK);
	EXPECT_NE(blackOcc, 0ULL);
	EXPECT_EQ(blackOcc, board.get_black_occupancy());
}

TEST_F(BoardTest, GetCombinedOccupancy_Initial)
{
	fenrir::Board board(valid_fen_position);
	uint64_t combined = board.get_combined_occupancy();
	/* 32 pieces in starting position */
	EXPECT_EQ(__builtin_popcountll(combined), 32);
}

TEST_F(BoardTest, GetEnPassantBB_Empty)
{
	fenrir::Board board("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
	EXPECT_EQ(board.get_en_passant_bb(), 0ULL);
}

TEST_F(BoardTest, GetEnPassantBB_Set)
{
	fenrir::Board board(valid_fen_position); /* has a3 en passant */
	uint64_t epBB = board.get_en_passant_bb();
	EXPECT_NE(epBB, 0ULL);
	/* a3 = rank 2, file 0, square 16 */
	EXPECT_EQ(epBB, (1ULL << 16));
}

TEST_F(BoardTest, GetBitboard_WhiteKing)
{
	fenrir::Board board(valid_fen_position);
	/* White king at e1 = rank 0, file 4, square 4 */
	uint64_t kingBB = board.get_bitboard(5); /* index 5 = white king */
	EXPECT_NE(kingBB, 0ULL);
	EXPECT_TRUE((kingBB & (1ULL << 4)) != 0); /* bit 4 = e1 */
}

TEST_F(BoardTest, ApplyMove_SimpleMove)
{
	fenrir::Board board(valid_fen_position);
	fenrir::Move move("e2", "e4");
	auto undo = board.apply_move(move);

	EXPECT_EQ(board.get_piece(3, 4), 'P'); /* e4 */
	EXPECT_EQ(board.get_piece(1, 4), '\0'); /* e2 empty */

	/* Undo and verify restored */
	board.undo_move(undo);
	EXPECT_EQ(board.get_piece(1, 4), 'P'); /* e2 back */
	EXPECT_EQ(board.get_piece(3, 4), '\0');
}

TEST_F(BoardTest, ApplyMove_Capture)
{
	fenrir::Board board("4k3/8/8/4p3/3P4/8/8/4K3 w - - 0 1");
	fenrir::Move move("d4", "e5", fenrir::MoveType::CAPTURE);
	auto undo = board.apply_move(move);

	EXPECT_EQ(board.get_piece(4, 4), 'P'); /* e5 has white pawn */
	EXPECT_EQ(board.get_piece(3, 3), '\0');

	board.undo_move(undo);
	EXPECT_EQ(board.get_piece(3, 3), 'P'); /* d4 back */
	EXPECT_EQ(board.get_piece(4, 4), 'p'); /* e5 black pawn back */
}

TEST_F(BoardTest, ApplyMove_EnPassant)
{
	/* White pawn at e5, black pawn just moved to d5 with ep square at d6 */
	fenrir::Board board("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");
	fenrir::Move move("e5", "d6", fenrir::MoveType::EN_PASSANT);
	auto undo = board.apply_move(move);

	EXPECT_EQ(board.get_piece(5, 3), 'P'); /* d6 */
	EXPECT_EQ(board.get_piece(4, 4), '\0'); /* e5 empty */
	EXPECT_EQ(board.get_piece(4, 3), '\0'); /* d5 black pawn removed */

	board.undo_move(undo);
	EXPECT_EQ(board.get_piece(4, 4), 'P'); /* e5 back */
	EXPECT_EQ(board.get_piece(4, 3), 'p'); /* d5 black pawn back */
}

TEST_F(BoardTest, ApplyMove_Promotion)
{
	fenrir::Board board("4k3/4P3/8/8/8/8/8/4K3 w - - 0 1");
	fenrir::Move move("e7", "e8", fenrir::MoveType::PROMOTION, 'Q');
	auto undo = board.apply_move(move);

	EXPECT_EQ(board.get_piece(7, 4), 'Q'); /* e8 has queen */
	EXPECT_EQ(board.get_piece(6, 4), '\0');

	board.undo_move(undo);
	EXPECT_EQ(board.get_piece(6, 4), 'P');
	EXPECT_EQ(board.get_piece(7, 4), 'k');
}

TEST_F(BoardTest, ApplyMove_CastleKingside)
{
	fenrir::Board board("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
	fenrir::Move move("e1", "g1", fenrir::MoveType::CASTLE_KINGSIDE);
	auto undo = board.apply_move(move);

	EXPECT_EQ(board.get_piece(0, 6), 'K'); /* g1 */
	EXPECT_EQ(board.get_piece(0, 5), 'R'); /* f1 */
	EXPECT_EQ(board.get_piece(0, 4), '\0'); /* e1 empty */
	EXPECT_EQ(board.get_piece(0, 7), '\0'); /* h1 empty */

	board.undo_move(undo);
	EXPECT_EQ(board.get_piece(0, 4), 'K');
	EXPECT_EQ(board.get_piece(0, 7), 'R');
}

TEST_F(BoardTest, ApplyMove_CastleQueenside)
{
	fenrir::Board board("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
	fenrir::Move move("e1", "c1", fenrir::MoveType::CASTLE_QUEENSIDE);
	auto undo = board.apply_move(move);

	EXPECT_EQ(board.get_piece(0, 2), 'K'); /* c1 */
	EXPECT_EQ(board.get_piece(0, 3), 'R'); /* d1 */
	EXPECT_EQ(board.get_piece(0, 4), '\0');
	EXPECT_EQ(board.get_piece(0, 0), '\0');

	board.undo_move(undo);
	EXPECT_EQ(board.get_piece(0, 4), 'K');
	EXPECT_EQ(board.get_piece(0, 0), 'R');
}

TEST_F(BoardTest, IsInCheck_NotInCheck)
{
	fenrir::Board board(valid_fen_position);
	EXPECT_FALSE(board.is_in_check(fenrir::WHITE));
	EXPECT_FALSE(board.is_in_check(fenrir::BLACK));
}

TEST_F(BoardTest, IsInCheck_RookCheck)
{
	/* White king at e1, black rook at e8 giving check, black king at a8 */
	fenrir::Board board("k3r3/8/8/8/8/8/8/4K3 w - - 0 1");
	EXPECT_TRUE(board.is_in_check(fenrir::WHITE));
	EXPECT_FALSE(board.is_in_check(fenrir::BLACK));
}

TEST_F(BoardTest, IsInCheck_BishopCheck)
{
	/* White king at e1, black bishop at h4 giving check along diagonal, black king at a8 */
	fenrir::Board board("k7/8/8/8/7b/8/8/4K3 w - - 0 1");
	EXPECT_TRUE(board.is_in_check(fenrir::WHITE));
}

TEST_F(BoardTest, IsInCheck_KnightCheck)
{
	/* White king at e1, black knight at d3 giving check, black king at a8 */
	fenrir::Board board("k7/8/8/8/8/3n4/8/4K3 w - - 0 1");
	EXPECT_TRUE(board.is_in_check(fenrir::WHITE));
}

TEST_F(BoardTest, IsInCheck_PawnCheck)
{
	/* White king at e4, black pawn at d5 giving check, black king at a8 */
	fenrir::Board board("k7/8/8/3p4/4K3/8/8/8 w - - 0 1");
	EXPECT_TRUE(board.is_in_check(fenrir::WHITE));
}

TEST_F(BoardTest, IsInCheck_QueenCheck)
{
	/* White king at e1, black queen at e8 giving check, black king at a8 */
	fenrir::Board board("k3q3/8/8/8/8/8/8/4K3 w - - 0 1");
	EXPECT_TRUE(board.is_in_check(fenrir::WHITE));
}

TEST_F(BoardTest, IsInCheck_KingCheck_Blocked)
{
	/* White king at e1, rook at e4, black rook at e8 - blocked by white rook, black king at a8 */
	fenrir::Board board("k3r3/8/8/8/4R3/8/8/4K3 w - - 0 1");
	EXPECT_FALSE(board.is_in_check(fenrir::WHITE));
}

TEST_F(BoardTest, IsSquareAttackedBy_Knight)
{
	/* White knight at d3, black king at a8, white king at e1 */
	fenrir::Board board("k7/8/8/8/8/3N4/8/4K3 w - - 0 1");
	/* White knight at d3 (rank=2, file=3) attacks e5 (rank=4, file=4) */
	uint8_t sq = static_cast<uint8_t>(4 * 8 + 4);
	EXPECT_TRUE(board.is_square_attacked_by(sq, fenrir::WHITE));
}

TEST_F(BoardTest, IsSquareAttackedBy_NotAttacked)
{
	/* White knight at d3, black king at a8, white king at e1 */
	fenrir::Board board("k7/8/8/8/8/3N4/8/4K3 w - - 0 1");
	/* h8 is not attacked */
	uint8_t sq = static_cast<uint8_t>(7 * 8 + 7);
	EXPECT_FALSE(board.is_square_attacked_by(sq, fenrir::WHITE));
}

TEST_F(BoardTest, CastlingRightsUpdated_AfterKingMove)
{
	fenrir::Board board("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
	fenrir::Move move("e1", "e2");
	board.apply_move(move);
	/* After king moves, castling rights should have K and Q removed */
	EXPECT_EQ(board.get_castling_rights().find('K'), std::string::npos);
	EXPECT_EQ(board.get_castling_rights().find('Q'), std::string::npos);
	/* Black's rights should remain */
	EXPECT_NE(board.get_castling_rights().find('k'), std::string::npos);
}

TEST_F(BoardTest, CastlingRightsUpdated_AfterRookMove)
{
	fenrir::Board board("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
	fenrir::Move move("h1", "h3");
	board.apply_move(move);
	/* After white kingside rook moves, K castling right removed */
	EXPECT_EQ(board.get_castling_rights().find('K'), std::string::npos);
	EXPECT_NE(board.get_castling_rights().find('Q'), std::string::npos);
}

TEST_F(BoardTest, ApplyMove_TogglesColor)
{
	fenrir::Board board(valid_fen_position);
	EXPECT_EQ(board.get_color(), fenrir::WHITE);
	fenrir::Move move("e2", "e4");
	auto undo = board.apply_move(move);
	EXPECT_EQ(board.get_color(), fenrir::BLACK);
	board.undo_move(undo);
	EXPECT_EQ(board.get_color(), fenrir::WHITE);
}

TEST_F(BoardTest, ApplyMove_EmptySquareNoOp)
{
	/* Move from empty square - should return state without crashing */
	fenrir::Board board(valid_fen_position);
	fenrir::Move move("e4", "e5"); /* empty square */
	EXPECT_NO_THROW(board.apply_move(move));
}

TEST_F(BoardTest, ResetEnPassant)
{
	fenrir::Board board(valid_fen_position);
	board.reset("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
	EXPECT_EQ(board.get_en_passant(), "e3");
}

TEST_F(BoardTest, ApplyMove_EnPassantBlack)
{
	/* Black pawn at d4, white pawn just moved to e4 with ep square at e3 */
	fenrir::Board board("4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1");
	fenrir::Move move("d4", "e3", fenrir::MoveType::EN_PASSANT);
	auto undo = board.apply_move(move);

	EXPECT_EQ(board.get_piece(2, 4), 'p'); /* e3 has black pawn */
	EXPECT_EQ(board.get_piece(3, 3), '\0'); /* d4 empty */
	EXPECT_EQ(board.get_piece(3, 4), '\0'); /* e4 white pawn removed */

	board.undo_move(undo);
	EXPECT_EQ(board.get_piece(3, 3), 'p'); /* d4 back */
	EXPECT_EQ(board.get_piece(3, 4), 'P'); /* e4 back */
}

TEST_F(BoardTest, ApplyMove_CastlingBlack)
{
	/* Kingside */
	{
		fenrir::Board board("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1");
		fenrir::Move move("e8", "g8", fenrir::MoveType::CASTLE_KINGSIDE);
		auto undo = board.apply_move(move);
		EXPECT_EQ(board.get_piece(7, 6), 'k'); /* g8 */
		EXPECT_EQ(board.get_piece(7, 5), 'r'); /* f8 */
		board.undo_move(undo);
	}
	/* Queenside */
	{
		fenrir::Board board("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1");
		fenrir::Move move("e8", "c8", fenrir::MoveType::CASTLE_QUEENSIDE);
		auto undo = board.apply_move(move);
		EXPECT_EQ(board.get_piece(7, 2), 'k'); /* c8 */
		EXPECT_EQ(board.get_piece(7, 3), 'r'); /* d8 */
		board.undo_move(undo);
	}
}

TEST_F(BoardTest, ApplyMove_PromotionCaptureBlack)
{
	/* Black pawn at d2, white knight at c1 */
	fenrir::Board board("4k3/8/8/8/8/8/3p4/2N1K3 b - - 0 1");
	fenrir::Move move("d2", "c1", fenrir::MoveType::PROMOTION, 'q');
	auto undo = board.apply_move(move);

	EXPECT_EQ(board.get_piece(0, 2), 'q'); /* c1 promoted to black queen */
	EXPECT_EQ(board.get_piece(1, 3), '\0'); /* d2 empty */

	board.undo_move(undo);
	EXPECT_EQ(board.get_piece(1, 3), 'p'); /* d2 pawn back */
	EXPECT_EQ(board.get_piece(0, 2), 'N'); /* c1 knight back */
}

TEST_F(BoardTest, CastlingRightsUpdated_BlackRook)
{
	/* Black kingside rook move */
	{
		fenrir::Board board("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1");
		fenrir::Move move("h8", "h6");
		board.apply_move(move);
		EXPECT_EQ(board.get_castling_rights().find('k'), std::string::npos);
		EXPECT_NE(board.get_castling_rights().find('q'), std::string::npos);
	}
	/* Black queenside rook move */
	{
		fenrir::Board board("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1");
		fenrir::Move move("a8", "a6");
		board.apply_move(move);
		EXPECT_EQ(board.get_castling_rights().find('q'), std::string::npos);
		EXPECT_NE(board.get_castling_rights().find('k'), std::string::npos);
	}
}

TEST_F(BoardTest, IsInCheck_NoKing)
{
	/* Set up position where white rook can capture black king (includes both kings to pass FEN validation) */
	fenrir::Board board("4k3/8/8/8/8/8/8/K3R3 w - - 0 1");
	fenrir::Move move("e1", "e8", fenrir::MoveType::CAPTURE);
	board.apply_move(move);

	/* Now black king is captured (removed from board) */
	EXPECT_FALSE(board.is_in_check(fenrir::BLACK));
}
