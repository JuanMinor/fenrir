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

#include "include/engine/engine.h"
#include "include/chess/attacks.h"

#ifndef FENRIR_VERSION
#define FENRIR_VERSION "0.0.0-unknown"
#endif

namespace fenrir
{

	Engine::Engine(const std::string &fen_string) : fen(fen_string), board(fen_string)
	{
		init_attack_tables();
		logger::INFO("Engine initialized with FEN: " + fen_string);
	}

	Engine::~Engine() {}

	const char* Engine::version()
	{
		return FENRIR_VERSION;
	}

	const AbstractBoard &Engine::get_board_view() const
	{
		return board;
	}

	std::vector<Move> Engine::generate_moves(const std::string &algebraic_address)
	{
		std::vector<Move> pseudo_legal;
		uint8_t rank, file;
		utils::parse_algebraic_notation(algebraic_address, rank, file);
		char piece_char = board.get_piece(rank, file);
		if (piece_char == '\0')
		{
			LOG_THROW_ERROR(
				(std::string("Board address ") + algebraic_address + " does not contain a piece").c_str(),
				false);
			return pseudo_legal;
		}

		Moves::generate_moves(rank, file, board, pseudo_legal);

		/* Legal move filtering: apply each pseudo-legal move, check if our king
		 * is left in check, undo immediately. Keep only legal moves. */
		bool active_is_white = std::isupper(static_cast<unsigned char>(piece_char));
		uint8_t active_color = active_is_white ? WHITE : BLACK;

		std::vector<Move> legal;
		legal.reserve(pseudo_legal.size());

		for (const Move &m : pseudo_legal)
		{
			UndoState undo = board.apply_move(m);
			bool in_check = board.is_in_check(active_color);
			board.undo_move(undo);
			if (!in_check)
			{
				legal.push_back(m);
			}
		}
		return legal;
	}

	std::vector<Move> Engine::generate_all_moves()
	{
		uint8_t active_color = board.get_color();
		std::vector<Move> all_moves;
		/* Reuse a single buffer to avoid per-piece heap allocations */
		std::vector<Move> piece_moves;
		piece_moves.reserve(32);

		/* Iterate all squares and collect moves for active color's pieces */
		for (uint8_t rank = 0; rank < BOARD_SIZE; ++rank)
		{
			for (uint8_t file = 0; file < BOARD_SIZE; ++file)
			{
				char piece = board.get_piece(rank, file);
				if (piece == '\0')
				{
					continue;
				}
				bool piece_is_white = std::isupper(static_cast<unsigned char>(piece));
				if ((active_color == WHITE) != piece_is_white)
				{
					continue;
				}

				piece_moves.clear();
				Moves::generate_moves(rank, file, board, piece_moves);

				for (const Move &m : piece_moves)
				{
					UndoState undo = board.apply_move(m);
					bool in_check = board.is_in_check(active_color);
					board.undo_move(undo);
					if (!in_check)
					{
						all_moves.push_back(m);
					}
				}
			}
		}

		return all_moves;
	}

	std::string Engine::get_fen(void)
	{
		std::string current_fen = board.get_fen();
		return current_fen;
	}

	void Engine::make_move(const Move &move)
	{
		// Generate all legal moves in the current position
		std::vector<Move> legal_moves = generate_all_moves();
		
		// Find a matching legal move
		bool found = false;
		Move matched_move = move;
		for (const auto &m : legal_moves)
		{
			if (m.get_from_square() == move.get_from_square() && 
				m.get_to_square() == move.get_to_square() && 
				m.get_promotion_piece() == move.get_promotion_piece())
			{
				matched_move = m;
				found = true;
				break;
			}
		}

		if (!found)
		{
			uint8_t from_rank, from_file;
			utils::parse_algebraic_notation(move.get_from(), from_rank, from_file);
			char piece = board.get_piece(from_rank, from_file);
			if (piece == '\0')
			{
				logger::ERROR("No piece found at " + move.get_from());
				return;
			}
			logger::WARN("Non-legal or out-of-turn move requested: " + move.get_from() + " -> " + move.get_to() + ". Playing anyway in permissive mode.");
		}

		UndoState state = board.apply_move(matched_move);
		undo_stack.push_back(state);

		logger::DEBUG("Made move from " + matched_move.get_from() + " to " + matched_move.get_to());
	}

	void Engine::undo_move()
	{
		if (undo_stack.empty())
		{
			logger::WARN("No moves to undo");
			return;
		}
		const UndoState &state = undo_stack.back();
		board.undo_move(state);
		undo_stack.pop_back();
		logger::DEBUG("Undid last move");
	}

	bool Engine::is_checkmate()
	{
		uint8_t active_color = board.get_color();
		if (!board.is_in_check(active_color))
		{
			return false;
		}
		/* Check if any legal move exists */
		auto all_moves = generate_all_moves();
		return all_moves.empty();
	}

	bool Engine::is_stalemate()
	{
		uint8_t active_color = board.get_color();
		if (board.is_in_check(active_color))
		{
			return false;
		}
		/* Check if any legal move exists */
		auto all_moves = generate_all_moves();
		return all_moves.empty();
	}

	bool Engine::is_draw()
	{
		if (is_stalemate())
		{
			return true;
		}

		if (board.get_half_move_clock() >= 100)
		{
			return true;
		}

		UndoState current_state;
		for (int i = 0; i < 12; ++i)
		{
			current_state.bitboards[i] = board.get_bitboard(i);
		}
		current_state.castling_rights = board.get_castling_rights_mask();
		current_state.en_passant_square = board.get_en_passant_square();
		current_state.color = board.get_color();

		int repetitions = 1;
		int limit = board.get_half_move_clock();
		int stack_size = static_cast<int>(undo_stack.size());
		int lookback = (limit < stack_size) ? limit : stack_size;

		for (int i = 0; i < lookback; ++i)
		{
			const UndoState &past_state = undo_stack[static_cast<size_t>(stack_size - 1 - i)];
			bool match = true;
			for (int j = 0; j < 12; ++j)
			{
				if (past_state.bitboards[j] != current_state.bitboards[j])
				{
					match = false;
					break;
				}
			}
			if (match &&
			    past_state.castling_rights == current_state.castling_rights &&
			    past_state.en_passant_square == current_state.en_passant_square &&
			    past_state.color == current_state.color)
			{
				repetitions++;
				if (repetitions >= 3)
				{
					return true;
				}
			}
		}

		return false;
	}

	void Engine::print_board(void) const
	{
		board.print();
	}

	void Engine::reset()
	{
		board.reset(fen);
		undo_stack.clear();
		logger::INFO("Reset the board to the initial state with FEN: " + fen);
	}
}
