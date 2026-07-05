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

#include "include/chess/board.h"
#include "include/chess/attacks.h"
#include <atomic>

namespace fenrir
{
	Board::Board(const std::string &fen_string)
		: fen(fen_string)
	{
		/* Initialize attack tables (idempotent) */
		static std::atomic<bool> tables_initialized{false};
		if (!tables_initialized.exchange(true))
		{
			init_attack_tables();
		}

		std::string fen_castling = fen.get_castling();
		this->castling_rights = 0;
		if (fen_castling.find('K') != std::string::npos)
			this->castling_rights |= CASTLE_K;
		if (fen_castling.find('Q') != std::string::npos)
			this->castling_rights |= CASTLE_Q;
		if (fen_castling.find('k') != std::string::npos)
			this->castling_rights |= CASTLE_k;
		if (fen_castling.find('q') != std::string::npos)
			this->castling_rights |= CASTLE_q;

		std::string fen_ep = fen.get_en_passant();
		if (fen_ep == "-" || fen_ep.empty())
		{
			this->en_passant_square = 64;
		}
		else
		{
			uint8_t ep_rank = 0, ep_file = 0;
			utils::parse_algebraic_notation(fen_ep.c_str(), ep_rank, ep_file);
			this->en_passant_square = static_cast<uint8_t>(ep_rank * 8 + ep_file);
		}

		this->castling_dirty = true;
		this->en_passant_dirty = true;

		this->color = fen.get_color();
		this->half_move_clock = static_cast<uint8_t>(fen.get_half_move_clock());
		this->full_moves = static_cast<uint8_t>(fen.get_full_moves());

		this->build_board(fen.get_placement());

		logger::INFO("Board initialized with FEN: " + fen.get_placement());
	}

	/* Private */
	void Board::build_board(const std::string &placement)
	{
		size_t size = placement.size();
		uint8_t rank = BOARD_SIZE - 1, file = 0;

		/* Clear bitboards and occupancy */
		for (int i = 0; i < 12; ++i)
		{
			this->bitboards[i] = 0ULL;
		}
		this->white_occupancy = 0ULL;
		this->black_occupancy = 0ULL;
		this->combined_occupancy = 0ULL;

		for (size_t i = 0; i < size && rank < BOARD_SIZE; ++i)
		{
			char c = placement[i];
			if (c == '/')
			{
				rank--;
				file = 0;
				continue;
			}
			if (std::isdigit(c))
			{
				uint8_t empties = static_cast<uint8_t>(c - '0');
				file = static_cast<uint8_t>(file + empties);
				continue;
			}

			uint8_t square_idx = static_cast<uint8_t>(rank * 8 + file);
			int bb_idx = get_bitboard_index(c);
			if (bb_idx >= 0)
			{
				set_bit(this->bitboards[bb_idx], square_idx);
				if (std::isupper(static_cast<unsigned char>(c)))
				{
					set_bit(this->white_occupancy, square_idx);
				}
				else
				{
					set_bit(this->black_occupancy, square_idx);
				}
				set_bit(this->combined_occupancy, square_idx);
			}
			++file;
		}
	}

	std::string Board::generate_placement_from_board(void) const
	{
		std::string placement;
		placement.reserve(80);

		for (int rank = BOARD_SIZE - 1; rank >= 0; --rank)
		{
			uint8_t empty_squares = 0;
			for (uint8_t file = 0; file < BOARD_SIZE; ++file)
			{
				char c = this->get_piece(static_cast<uint8_t>(rank), file);
				if (c == '\0')
				{
					++empty_squares;
				}
				else
				{
					if (empty_squares)
					{
						placement += static_cast<char>('0' + empty_squares);
						empty_squares = 0;
					}
					placement += c;
				}
			}
			if (empty_squares)
			{
				placement += static_cast<char>('0' + empty_squares);
			}
			if (rank != 0)
			{
				placement += '/';
			}
		}
		return placement;
	}

	std::string Board::get_fen(void)
	{
		fen.set_placement(this->generate_placement_from_board());
		fen.set_castling(this->get_castling_rights().empty() ? "-" : this->get_castling_rights());
		fen.set_en_passant(this->get_en_passant().empty() ? "-" : this->get_en_passant());
		fen.set_color(this->color);
		fen.set_half_move_clock(this->half_move_clock);
		fen.set_full_moves(this->full_moves);

		return fen.generate_fen();
	}

	const std::string &Board::get_en_passant(void) const
	{
		if (this->en_passant_dirty)
		{
			if (this->en_passant_square == 64)
			{
				this->en_passant_str = "";
			}
			else
			{
				this->en_passant_str = utils::get_algebraic_notation(
					this->en_passant_square / 8,
					this->en_passant_square % 8);
			}
			this->en_passant_dirty = false;
		}
		return this->en_passant_str;
	}

	const std::string &Board::get_castling_rights(void) const
	{
		if (this->castling_dirty)
		{
			this->castling_str = "";
			if (this->castling_rights & CASTLE_K)
				this->castling_str += 'K';
			if (this->castling_rights & CASTLE_Q)
				this->castling_str += 'Q';
			if (this->castling_rights & CASTLE_k)
				this->castling_str += 'k';
			if (this->castling_rights & CASTLE_q)
				this->castling_str += 'q';
			if (this->castling_str.empty())
			{
				this->castling_str = "-";
			}
			this->castling_dirty = false;
		}
		return this->castling_str;
	}

	char Board::get_piece(uint8_t rank, uint8_t file) const
	{
		if (rank >= BOARD_SIZE || file >= BOARD_SIZE) return '\0';
		uint8_t square_idx = static_cast<uint8_t>(rank * 8 + file);

		if (test_bit(this->white_occupancy, square_idx))
		{
			for (int i = 0; i < 6; ++i)
			{
				if (test_bit(this->bitboards[i], square_idx))
				{
					static const char aliases[6] = {'P', 'N', 'B', 'R', 'Q', 'K'};
					return aliases[i];
				}
			}
		}
		else if (test_bit(this->black_occupancy, square_idx))
		{
			for (int i = 6; i < 12; ++i)
			{
				if (test_bit(this->bitboards[i], square_idx))
				{
					static const char aliases[6] = {'p', 'n', 'b', 'r', 'q', 'k'};
					return aliases[i - 6];
				}
			}
		}
		return '\0';
	}

	uint64_t Board::get_en_passant_bb() const
	{
		if (this->en_passant_square == 64)
		{
			return 0ULL;
		}
		return (1ULL << this->en_passant_square);
	}

	void Board::reset(const std::string &fen_string)
	{
		this->fen = Fen(fen_string);

		std::string fen_castling = fen.get_castling();
		this->castling_rights = 0;
		if (fen_castling.find('K') != std::string::npos)
			this->castling_rights |= CASTLE_K;
		if (fen_castling.find('Q') != std::string::npos)
			this->castling_rights |= CASTLE_Q;
		if (fen_castling.find('k') != std::string::npos)
			this->castling_rights |= CASTLE_k;
		if (fen_castling.find('q') != std::string::npos)
			this->castling_rights |= CASTLE_q;

		std::string fen_ep = fen.get_en_passant();
		if (fen_ep == "-" || fen_ep.empty())
		{
			this->en_passant_square = 64;
		}
		else
		{
			uint8_t ep_rank = 0, ep_file = 0;
			utils::parse_algebraic_notation(fen_ep.c_str(), ep_rank, ep_file);
			this->en_passant_square = static_cast<uint8_t>(ep_rank * 8 + ep_file);
		}

		this->castling_dirty = true;
		this->en_passant_dirty = true;

		this->color = fen.get_color();
		this->half_move_clock = static_cast<uint8_t>(fen.get_half_move_clock());
		this->full_moves = static_cast<uint8_t>(fen.get_full_moves());
		this->build_board(fen.get_placement());
		logger::INFO("Board reset to FEN: " + fen_string);
	}

	void Board::print(void) const
	{
		if (!DEBUG)
		{
			return;
		}
		std::cout << "  ";
		for (uint8_t j = 0; j < BOARD_SIZE; ++j)
		{
			std::cout << color::Modifier(color::Color::FG_YELLOW) << char(97 + j) << " ";
		}
		std::cout << color::Modifier(color::Color::RESET) << std::endl;
		for (int i = BOARD_SIZE - 1; i >= 0; i--)
		{
			std::cout << color::Modifier(color::Color::FG_YELLOW) << static_cast<unsigned int>(i + 1) << " "
					  << color::Modifier(color::Color::RESET);
			for (uint8_t j = 0; j < BOARD_SIZE; ++j)
			{
				char c = this->get_piece(static_cast<uint8_t>(i), j);
				if (c != '\0')
				{
					bool is_white = std::isupper(static_cast<unsigned char>(c));
					std::cout << color::Modifier(is_white ? color::Color::RESET : color::Color::FG_CYAN)
							  << c
							  << color::Modifier(color::Color::RESET) << " ";
				}
				else
				{
					std::cout << ". ";
				}
			}
			std::cout << color::Modifier(color::Color::FG_YELLOW) << static_cast<unsigned int>(i + 1)
					  << color::Modifier(color::Color::RESET) << std::endl;
		}

		std::cout << "  ";
		for (uint8_t j = 0; j < BOARD_SIZE; ++j)
		{
			std::cout << color::Modifier(color::Color::FG_YELLOW) << char(97 + j) << " ";
		}
		std::cout << color::Modifier(color::Color::RESET) << std::endl;
	}

	/* =========================================================
	 *  Make / Unmake implementation
	 * ========================================================= */

	UndoState Board::apply_move(const Move &move)
	{
		/* Save state snapshot */
		UndoState state;
		for (int i = 0; i < 12; ++i)
		{
			state.bitboards[i] = this->bitboards[i];
		}
		state.white_occupancy = this->white_occupancy;
		state.black_occupancy = this->black_occupancy;
		state.combined_occupancy = this->combined_occupancy;
		state.castling_rights = this->castling_rights;
		state.en_passant_square = this->en_passant_square;
		state.color = this->color;
		state.half_move_clock = this->half_move_clock;
		state.full_moves = this->full_moves;

		uint8_t from_sq = move.get_from_square();
		uint8_t to_sq = move.get_to_square();
		uint8_t from_rank = static_cast<uint8_t>(from_sq / 8);
		uint8_t from_file = static_cast<uint8_t>(from_sq % 8);
		uint8_t to_rank = static_cast<uint8_t>(to_sq / 8);
		uint8_t to_file = static_cast<uint8_t>(to_sq % 8);

		char active_piece = this->get_piece(from_rank, from_file);
		if (active_piece == '\0')
		{
			return state; /* No active piece */
		}

		bool active_is_white = std::isupper(static_cast<unsigned char>(active_piece));
		int active_bb_idx = get_bitboard_index(active_piece);

		/* Clear EP square */
		this->en_passant_square = 64;
		this->en_passant_dirty = true;

		MoveType mt = move.get_move_type();

		/* Remove captured piece from destination */
		if (mt == MoveType::CAPTURE)
		{
			char target_piece = this->get_piece(to_rank, to_file);
			if (target_piece != '\0')
			{
				int cap_bb_idx = get_bitboard_index(target_piece);
				if (cap_bb_idx >= 0)
				{
					clear_bit(this->bitboards[cap_bb_idx], to_sq);
					if (std::isupper(static_cast<unsigned char>(target_piece)))
					{
						clear_bit(this->white_occupancy, to_sq);
					}
					else
					{
						clear_bit(this->black_occupancy, to_sq);
					}
				}
			}
		}
		else if (mt == MoveType::EN_PASSANT)
		{
			/* Remove captured EP pawn */
			uint8_t ep_file = to_file;
			uint8_t ep_pawn_rank = from_rank;
			uint8_t ep_pawn_sq = static_cast<uint8_t>(ep_pawn_rank * 8 + ep_file);
			char captured_pawn = active_is_white ? 'p' : 'P';
			int cap_bb_idx = get_bitboard_index(captured_pawn);
			if (cap_bb_idx >= 0)
			{
				clear_bit(this->bitboards[cap_bb_idx], ep_pawn_sq);
				if (active_is_white)
				{
					clear_bit(this->black_occupancy, ep_pawn_sq);
				}
				else
				{
					clear_bit(this->white_occupancy, ep_pawn_sq);
				}
			}
		}
		else if (mt == MoveType::CASTLE_KINGSIDE)
		{
			/* Move rook: H to F (kingside castle) */
			uint8_t rook_from_sq = static_cast<uint8_t>(from_rank * 8 + 7); /* h-file */
			uint8_t rook_to_sq = static_cast<uint8_t>(from_rank * 8 + 5);	/* f-file */
			char rook_char = active_is_white ? 'R' : 'r';
			int rook_bb_idx = get_bitboard_index(rook_char);
			if (rook_bb_idx >= 0)
			{
				clear_bit(this->bitboards[rook_bb_idx], rook_from_sq);
				set_bit(this->bitboards[rook_bb_idx], rook_to_sq);
				if (active_is_white)
				{
					clear_bit(this->white_occupancy, rook_from_sq);
					set_bit(this->white_occupancy, rook_to_sq);
				}
				else
				{
					clear_bit(this->black_occupancy, rook_from_sq);
					set_bit(this->black_occupancy, rook_to_sq);
				}
			}
		}
		else if (mt == MoveType::CASTLE_QUEENSIDE)
		{
			/* Move rook: A to D (queenside castle) */
			uint8_t rook_from_sq = static_cast<uint8_t>(from_rank * 8 + 0); /* a-file */
			uint8_t rook_to_sq = static_cast<uint8_t>(from_rank * 8 + 3);	/* d-file */
			char rook_char = active_is_white ? 'R' : 'r';
			int rook_bb_idx = get_bitboard_index(rook_char);
			if (rook_bb_idx >= 0)
			{
				clear_bit(this->bitboards[rook_bb_idx], rook_from_sq);
				set_bit(this->bitboards[rook_bb_idx], rook_to_sq);
				if (active_is_white)
				{
					clear_bit(this->white_occupancy, rook_from_sq);
					set_bit(this->white_occupancy, rook_to_sq);
				}
				else
				{
					clear_bit(this->black_occupancy, rook_from_sq);
					set_bit(this->black_occupancy, rook_to_sq);
				}
			}
		}

		/* Move active piece */
		if (active_bb_idx >= 0)
		{
			/* Perform pawn promotion */
			if (mt == MoveType::PROMOTION)
			{
				char prom_char = move.get_promotion_piece();
				/* Match piece color case */
				if (active_is_white)
				{
					prom_char = static_cast<char>(std::toupper(static_cast<unsigned char>(prom_char)));
				}
				else
				{
					prom_char = static_cast<char>(std::tolower(static_cast<unsigned char>(prom_char)));
				}
				int prom_bb_idx = get_bitboard_index(prom_char);

				/* Remove pawn from source square */
				clear_bit(this->bitboards[active_bb_idx], from_sq);

				/* Clear destination square */
				char target_at_dest = this->get_piece(to_rank, to_file);
				if (target_at_dest != '\0')
				{
					int cap_bb_idx = get_bitboard_index(target_at_dest);
					if (cap_bb_idx >= 0)
					{
						clear_bit(this->bitboards[cap_bb_idx], to_sq);
						if (std::isupper(static_cast<unsigned char>(target_at_dest)))
						{
							clear_bit(this->white_occupancy, to_sq);
						}
						else
						{
							clear_bit(this->black_occupancy, to_sq);
						}
					}
				}

				/* Place promotion piece */
				if (prom_bb_idx >= 0)
				{
					set_bit(this->bitboards[prom_bb_idx], to_sq);
				}

				/* Update occupancy bitboards */
				if (active_is_white)
				{
					clear_bit(this->white_occupancy, from_sq);
					set_bit(this->white_occupancy, to_sq);
				}
				else
				{
					clear_bit(this->black_occupancy, from_sq);
					set_bit(this->black_occupancy, to_sq);
				}
			}
			else
			{
				clear_bit(this->bitboards[active_bb_idx], from_sq);
				set_bit(this->bitboards[active_bb_idx], to_sq);
				if (active_is_white)
				{
					clear_bit(this->white_occupancy, from_sq);
					set_bit(this->white_occupancy, to_sq);
				}
				else
				{
					clear_bit(this->black_occupancy, from_sq);
					set_bit(this->black_occupancy, to_sq);
				}
			}
		}

		/* Set EP square on double pawn push */
		if (std::tolower(static_cast<unsigned char>(active_piece)) == 'p' &&
			std::abs(static_cast<int>(to_rank) - static_cast<int>(from_rank)) == 2)
		{
			this->en_passant_square = static_cast<uint8_t>(((static_cast<int>(to_rank) + static_cast<int>(from_rank)) / 2) * 8 + from_file);
			this->en_passant_dirty = true;
		}

		/* Update castling rights */
		/* Revoke castling rights on king move */
		if (std::tolower(static_cast<unsigned char>(active_piece)) == 'k')
		{
			if (active_is_white)
			{
				this->castling_rights = static_cast<uint8_t>(this->castling_rights & ~(CASTLE_K | CASTLE_Q));
				this->castling_dirty = true;
			}
			else
			{
				this->castling_rights = static_cast<uint8_t>(this->castling_rights & ~(CASTLE_k | CASTLE_q));
				this->castling_dirty = true;
			}
		}

		/* Revoke castling rights on rook move or capture */
		{
			/* White kingside rook H1 */
			if (from_sq == 7 || to_sq == 7)
			{
				if (this->castling_rights & CASTLE_K)
				{
					this->castling_rights = static_cast<uint8_t>(this->castling_rights & ~CASTLE_K);
					this->castling_dirty = true;
				}
			}
			/* White queenside rook A1 */
			if (from_sq == 0 || to_sq == 0)
			{
				if (this->castling_rights & CASTLE_Q)
				{
					this->castling_rights = static_cast<uint8_t>(this->castling_rights & ~CASTLE_Q);
					this->castling_dirty = true;
				}
			}
			/* Black kingside rook H8 */
			if (from_sq == 63 || to_sq == 63)
			{
				if (this->castling_rights & CASTLE_k)
				{
					this->castling_rights = static_cast<uint8_t>(this->castling_rights & ~CASTLE_k);
					this->castling_dirty = true;
				}
			}
			/* Black queenside rook A8 */
			if (from_sq == 56 || to_sq == 56)
			{
				if (this->castling_rights & CASTLE_q)
				{
					this->castling_rights = static_cast<uint8_t>(this->castling_rights & ~CASTLE_q);
					this->castling_dirty = true;
				}
			}
		}

		this->combined_occupancy = this->white_occupancy | this->black_occupancy;

		/* Toggle active color */
		this->color = (this->color == WHITE) ? BLACK : WHITE;

		/* Update move counters */
		if (std::tolower(static_cast<unsigned char>(active_piece)) == 'p' ||
			mt == MoveType::CAPTURE || mt == MoveType::EN_PASSANT || mt == MoveType::PROMOTION)
		{
			this->half_move_clock = 0;
		}
		else
		{
			this->half_move_clock = static_cast<uint8_t>(this->half_move_clock + 1);
		}

		if (this->color == WHITE) /* after toggle, if now white it means black just moved */
		{
			this->full_moves = static_cast<uint8_t>(this->full_moves + 1);
		}

		return state;
	}

	void Board::undo_move(const UndoState &state)
	{
		for (int i = 0; i < 12; ++i)
		{
			this->bitboards[i] = state.bitboards[i];
		}
		this->white_occupancy = state.white_occupancy;
		this->black_occupancy = state.black_occupancy;
		this->combined_occupancy = state.combined_occupancy;
		this->castling_rights = state.castling_rights;
		this->en_passant_square = state.en_passant_square;
		this->castling_dirty = true;
		this->en_passant_dirty = true;
		this->color = state.color;
		this->half_move_clock = state.half_move_clock;
		this->full_moves = state.full_moves;
	}

	/* =========================================================
	 *  Check detection — uses precomputed attack tables
	 * ========================================================= */

	bool Board::is_square_attacked_by(uint8_t square, uint8_t attacker_color) const
	{
		/* Attacker piece indices */
		/* White: 0-5 (P, N, B, R, Q, K), Black: 6-11 (p, n, b, r, q, k) */
		int pawn_idx = (attacker_color == WHITE) ? 0 : 6;
		int knight_idx = (attacker_color == WHITE) ? 1 : 7;
		int bishop_idx = (attacker_color == WHITE) ? 2 : 8;
		int rook_idx = (attacker_color == WHITE) ? 3 : 9;
		int queen_idx = (attacker_color == WHITE) ? 4 : 10;
		int king_idx = (attacker_color == WHITE) ? 5 : 11;

		/* Pawn attacks */
		uint8_t defender_color = (attacker_color == WHITE) ? BLACK : WHITE;
		uint64_t pawn_attack_mask = PAWN_ATTACKS[defender_color][square];
		if (pawn_attack_mask & this->bitboards[pawn_idx])
		{
			return true;
		}

		/* Knight attacks */
		if (KNIGHT_ATTACKS[square] & this->bitboards[knight_idx])
		{
			return true;
		}

		/* King attacks */
		if (KING_ATTACKS[square] & this->bitboards[king_idx])
		{
			return true;
		}

		/* Bishop diagonal attacks */
		{
			uint64_t bishops = this->bitboards[bishop_idx] | this->bitboards[queen_idx];
			/* Check diagonal ray directions */
			for (int dir : {RAY_NE, RAY_SE, RAY_SW, RAY_NW})
			{
				uint64_t ray = RAY[square][dir];
				uint64_t blockers = ray & this->combined_occupancy;
				if (blockers)
				{
					/* Find first blocker on ray */
					uint8_t first_blocker;
					if (dir == RAY_NE || dir == RAY_N || dir == RAY_NW || dir == RAY_E)
					{
						first_blocker = fenrir::bitscan_forward(blockers);
					}
					else
					{
						first_blocker = fenrir::bitscan_reverse(blockers);
					}
					if ((1ULL << first_blocker) & bishops)
					{
						return true;
					}
				}
			}
		}

		/* Rook orthogonal attacks */
		{
			uint64_t rooks = this->bitboards[rook_idx] | this->bitboards[queen_idx];
			for (int dir : {RAY_N, RAY_E, RAY_S, RAY_W})
			{
				uint64_t ray = RAY[square][dir];
				uint64_t blockers = ray & this->combined_occupancy;
				if (blockers)
				{
					uint8_t first_blocker;
					if (dir == RAY_N || dir == RAY_E)
					{
						first_blocker = fenrir::bitscan_forward(blockers);
					}
					else
					{
						first_blocker = fenrir::bitscan_reverse(blockers);
					}
					if ((1ULL << first_blocker) & rooks)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	bool Board::is_in_check(uint8_t clr) const
	{
		/* Find king square */
		int king_bb_idx = (clr == WHITE) ? 5 : 11;
		uint64_t king_bb = this->bitboards[king_bb_idx];
		if (king_bb == 0ULL)
		{
			return false; /* No king on board */
		}
		uint8_t king_square = fenrir::bitscan_forward(king_bb);
		uint8_t opponent = (clr == WHITE) ? BLACK : WHITE;
		return is_square_attacked_by(king_square, opponent);
	}

} /* namespace fenrir */
