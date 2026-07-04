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

#include "include/chess/moves.h"
#include <locale>
#include <cctype>

namespace fenrir
{
	void Moves::generate_bishop_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves)
	{
		constexpr int8_t directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
		slide_in_directions(rank, file, piece_char, board, moves, directions, 4);
	}

	void Moves::add_capture_move(uint8_t from_rank, uint8_t from_file, char piece_char, uint8_t to_rank, uint8_t to_file, char target_char, std::vector<Move> &moves)
	{
		bool is_white = std::isupper(static_cast<unsigned char>(piece_char));
		bool target_white = std::isupper(static_cast<unsigned char>(target_char));
		if (is_white != target_white)
		{
			uint8_t from_sq = static_cast<uint8_t>(from_rank * 8 + from_file);
			uint8_t to_sq = static_cast<uint8_t>(to_rank * 8 + to_file);
			Move move = Move(from_sq, to_sq, MoveType::CAPTURE);
			moves.emplace_back(move);
		}
	}

	void Moves::generate_king_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves)
	{
		constexpr int8_t direction_vectors[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
		slide_in_directions(rank, file, piece_char, board, moves, direction_vectors, 8, true);

		/* Castling generation */
		bool is_white = std::isupper(static_cast<unsigned char>(piece_char));
		const std::string &castling = board.get_castling_rights();

		/* King must be on its starting square */
		uint8_t expected_rank = is_white ? 0U : 7U;
		uint8_t expected_file = 4U; /* e-file */

		if (rank != expected_rank || file != expected_file)
		{
			return;
		}

		uint8_t from_sq = static_cast<uint8_t>(rank * 8 + file);

		/* Kingside castling */
		char ks_right = is_white ? 'K' : 'k';
		if (castling.find(ks_right) != std::string::npos)
		{
			/* Check squares between king and rook are empty (f and g files) */
			uint8_t f_file = 5U, g_file = 6U;
			if (board.get_piece(rank, f_file) == '\0' && board.get_piece(rank, g_file) == '\0')
			{
				/* Rook must be present at h-file */
				char rook_char = is_white ? 'R' : 'r';
				if (board.get_piece(rank, 7) == rook_char)
				{
					uint8_t to_sq = static_cast<uint8_t>(rank * 8 + 6); /* g-file */
					moves.emplace_back(Move(from_sq, to_sq, MoveType::CASTLE_KINGSIDE));
				}
			}
		}

		/* Queenside castling */
		char qs_right = is_white ? 'Q' : 'q';
		if (castling.find(qs_right) != std::string::npos)
		{
			/* Check squares between king and rook are empty (d, c, b files) */
			uint8_t d_file = 3U, c_file = 2U, b_file = 1U;
			if (board.get_piece(rank, d_file) == '\0' &&
			    board.get_piece(rank, c_file) == '\0' &&
			    board.get_piece(rank, b_file) == '\0')
			{
				/* Rook must be present at a-file */
				char rook_char = is_white ? 'R' : 'r';
				if (board.get_piece(rank, 0) == rook_char)
				{
					uint8_t to_sq = static_cast<uint8_t>(rank * 8 + 2); /* c-file */
					moves.emplace_back(Move(from_sq, to_sq, MoveType::CASTLE_QUEENSIDE));
				}
			}
		}
	}

	void Moves::generate_knight_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves)
	{
		const uint8_t from_sq = static_cast<uint8_t>(rank * 8 + file);

		constexpr int8_t knight_moves[8][2] = {
			{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

		for (const auto &knight_move : knight_moves)
		{
			const int8_t new_rank = static_cast<int8_t>(static_cast<int>(rank) + knight_move[0]);
			const int8_t new_file = static_cast<int8_t>(static_cast<int>(file) + knight_move[1]);

			if (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
			{
				char target_piece = board.get_piece(static_cast<uint8_t>(new_rank), static_cast<uint8_t>(new_file));
				if (target_piece == '\0')
				{
					uint8_t to_sq = static_cast<uint8_t>(new_rank * 8 + new_file);
					moves.emplace_back(Move(from_sq, to_sq, MoveType::NORMAL));
					continue;
				}
				add_capture_move(rank, file, piece_char, static_cast<uint8_t>(new_rank), static_cast<uint8_t>(new_file), target_piece, moves);
			}
		}
	}

	/* Helper to add promotion moves for a pawn that can promote */
	static void addPromotionMoves(uint8_t from_sq, uint8_t to_sq, std::vector<Move> &moves)
	{
		/* Emit 4 promotion moves: Q, R, B, N */
		static const char promotion_pieces[4] = {'Q', 'R', 'B', 'N'};
		for (char p : promotion_pieces)
		{
			moves.emplace_back(Move(from_sq, to_sq, MoveType::PROMOTION, p));
		}
	}

	void Moves::generate_pawn_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves)
	{
		bool is_white = std::isupper(static_cast<unsigned char>(piece_char));
		int direction = is_white ? 1 : -1;
		uint8_t from_sq = static_cast<uint8_t>(rank * 8 + file);

		/* Promotion rank: white promotes at rank 7, black at rank 0 */
		uint8_t promotion_rank = is_white ? 7U : 0U;

		int new_rank_int = static_cast<int>(rank) + direction;
		if (new_rank_int >= 0 && new_rank_int < BOARD_SIZE && board.get_piece(static_cast<uint8_t>(new_rank_int), file) == '\0')
		{
			uint8_t to_sq = static_cast<uint8_t>(new_rank_int * 8 + file);
			uint8_t new_rank_u = static_cast<uint8_t>(new_rank_int);
			if (new_rank_u == promotion_rank)
			{
				addPromotionMoves(from_sq, to_sq, moves);
			}
			else
			{
				moves.emplace_back(Move(from_sq, to_sq, MoveType::NORMAL));
			}
		}

		/* Diagonal captures (left) */
		if (file > 0 && new_rank_int >= 0 && new_rank_int < BOARD_SIZE)
		{
			char target = board.get_piece(static_cast<uint8_t>(new_rank_int), static_cast<uint8_t>(file - 1));
			if (target != '\0')
			{
				bool target_white = std::isupper(static_cast<unsigned char>(target));
				if (is_white != target_white)
				{
					uint8_t to_sq = static_cast<uint8_t>(new_rank_int * 8 + (file - 1));
					uint8_t new_rank_u = static_cast<uint8_t>(new_rank_int);
					if (new_rank_u == promotion_rank)
					{
						addPromotionMoves(from_sq, to_sq, moves);
					}
					else
					{
						moves.emplace_back(Move(from_sq, to_sq, MoveType::CAPTURE));
					}
				}
			}
		}

		/* Diagonal captures (right) */
		if (file < BOARD_SIZE - 1 && new_rank_int >= 0 && new_rank_int < BOARD_SIZE)
		{
			char target = board.get_piece(static_cast<uint8_t>(new_rank_int), static_cast<uint8_t>(file + 1));
			if (target != '\0')
			{
				bool target_white = std::isupper(static_cast<unsigned char>(target));
				if (is_white != target_white)
				{
					uint8_t to_sq = static_cast<uint8_t>(new_rank_int * 8 + (file + 1));
					uint8_t new_rank_u = static_cast<uint8_t>(new_rank_int);
					if (new_rank_u == promotion_rank)
					{
						addPromotionMoves(from_sq, to_sq, moves);
					}
					else
					{
						moves.emplace_back(Move(from_sq, to_sq, MoveType::CAPTURE));
					}
				}
			}
		}

		/* Double push from starting rank */
		if ((is_white && rank == 1) || (!is_white && rank == 6))
		{
			if (board.get_piece(static_cast<uint8_t>(rank + direction), file) == '\0')
			{
				int double_rank = static_cast<int>(rank) + (2 * direction);
				if (double_rank >= 0 && double_rank < BOARD_SIZE && board.get_piece(static_cast<uint8_t>(double_rank), file) == '\0')
				{
					uint8_t to_sq = static_cast<uint8_t>(double_rank * 8 + file);
					moves.emplace_back(Move(from_sq, to_sq, MoveType::NORMAL));
				}
			}
		}

		/* En passant */
		const std::string &en_passant = board.get_en_passant();
		if (!en_passant.empty())
		{
			uint8_t en_passant_rank = 0, en_passant_file = 0;
			utils::parse_algebraic_notation(en_passant.c_str(), en_passant_rank, en_passant_file);
			if (en_passant_rank == static_cast<uint8_t>(static_cast<int>(rank) + direction) &&
			    std::abs(static_cast<int>(en_passant_file) - static_cast<int>(file)) == 1)
			{
				char target_piece = board.get_piece(rank, en_passant_file);
				if (target_piece != '\0' && std::tolower(static_cast<unsigned char>(target_piece)) == 'p')
				{
					bool target_white = std::isupper(static_cast<unsigned char>(target_piece));
					if (is_white != target_white)
					{
						uint8_t to_sq = static_cast<uint8_t>(en_passant_rank * 8 + en_passant_file);
						moves.emplace_back(Move(from_sq, to_sq, MoveType::EN_PASSANT));
					}
				}
			}
		}
	}

	void Moves::generate_queen_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves)
	{
		constexpr int8_t direction_vectors[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
		slide_in_directions(rank, file, piece_char, board, moves, direction_vectors, 8);
	}

	void Moves::generate_rook_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves)
	{
		constexpr int8_t direction_vectors[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
		slide_in_directions(rank, file, piece_char, board, moves, direction_vectors, 4);
	}

	void Moves::slide_in_directions(uint8_t from_rank, uint8_t from_file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves, const int8_t direction_vectors[][2], size_t num_directions, bool single_depth)
	{
		const uint8_t from_sq = static_cast<uint8_t>(from_rank * 8 + from_file);

		for (size_t i = 0; i < num_directions; ++i)
		{
			const int8_t rank_delta = direction_vectors[i][0];
			const int8_t file_delta = direction_vectors[i][1];

			int8_t new_rank = static_cast<int8_t>(static_cast<int>(from_rank) + rank_delta);
			int8_t new_file = static_cast<int8_t>(static_cast<int>(from_file) + file_delta);
			uint8_t depth = 0;

			while (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
			{
				if (single_depth && depth == 1)
				{
					break;
				}
				depth++;
				char target_piece = board.get_piece(static_cast<uint8_t>(new_rank), static_cast<uint8_t>(new_file));
				if (target_piece == '\0')
				{
					uint8_t to_sq = static_cast<uint8_t>(new_rank * 8 + new_file);
					moves.emplace_back(Move(from_sq, to_sq, MoveType::NORMAL));
					new_rank = static_cast<int8_t>(static_cast<int>(new_rank) + rank_delta);
					new_file = static_cast<int8_t>(static_cast<int>(new_file) + file_delta);
					continue;
				}
				add_capture_move(from_rank, from_file, piece_char, static_cast<uint8_t>(new_rank), static_cast<uint8_t>(new_file), target_piece, moves);
				break;
			}
		}
	}

	void Moves::generate_moves(uint8_t rank, uint8_t file, const AbstractBoard &board, std::vector<Move> &moves)
	{
		char piece_char = board.get_piece(rank, file);
		if (piece_char == '\0')
		{
			return;
		}
		switch (std::tolower(static_cast<unsigned char>(piece_char)))
		{
		case 'b':
			generate_bishop_moves(rank, file, piece_char, board, moves);
			break;
		case 'k':
			generate_king_moves(rank, file, piece_char, board, moves);
			break;
		case 'n':
			generate_knight_moves(rank, file, piece_char, board, moves);
			break;
		case 'p':
			generate_pawn_moves(rank, file, piece_char, board, moves);
			break;
		case 'q':
			generate_queen_moves(rank, file, piece_char, board, moves);
			break;
		case 'r':
			generate_rook_moves(rank, file, piece_char, board, moves);
			break;
		}
	}
}
