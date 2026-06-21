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

namespace fenrir
{
	Board::Board(const std::string &fenString)
		: fen(fenString)
	{
		this->castling = fen.getCastling();
		this->enPassant = fen.getEnPassant() == "-" ? "" : fen.getEnPassant();
		this->color = fen.getColor();
		this->halfMoveClock = static_cast<uint8_t>(fen.getHalfMoveClock());
		this->fullMoves = static_cast<uint8_t>(fen.getFullMoves());

		this->buildBoard(fen.getPlacement());

		logger::INFO("Board initialized with FEN: " + fen.getPlacement());
	}

	/* Private */
	void Board::buildBoard(const std::string &placement)
	{
		size_t size = placement.size();
		uint8_t rank = BOARD_SIZE - 1, file = 0;

		// Initialize bitboards and occupancy
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
			int bb_idx = getBitboardIndex(c);
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

			this->logPieceAction("Created", c, rank, file, "✅");
			++file;
		}
	}

	std::string Board::generatePlacementFromBoard(void) const
	{
		std::string placement;
		placement.reserve(80);

		for (int rank = BOARD_SIZE - 1; rank >= 0; --rank)
		{
			uint8_t empty_squares = 0;
			for (uint8_t file = 0; file < BOARD_SIZE; ++file)
			{
				char c = this->getPiece(static_cast<uint8_t>(rank), file);
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

	void Board::logPieceAction(const std::string &action, char piece_char, uint8_t rank, uint8_t file, const std::string &emoji) const
	{
		bool is_white = std::isupper(static_cast<unsigned char>(piece_char));
		const std::string colorStr = is_white ? "white" : "black";
		char lower_alias = static_cast<char>(std::tolower(static_cast<unsigned char>(piece_char)));

		uint8_t piece_value = 0;
		switch (lower_alias)
		{
			case 'p': piece_value = 1; break;
			case 'n': piece_value = 3; break;
			case 'b': piece_value = 3; break;
			case 'r': piece_value = 5; break;
			case 'q': piece_value = 9; break;
			case 'k': piece_value = 0; break;
		}

		std::stringstream oss;
		oss << action << " " << colorStr << " " << PIECE_NAMES.at(lower_alias)
			<< " in position " << utils::getAlgebraicNotation(rank, file) << " " << emoji;

		std::stringstream detailed_oss;
		detailed_oss << oss.str() << " (" << unsigned(piece_value) << " value) at <"
					 << unsigned(rank) << ", " << unsigned(file) << ">";

		logger::DEBUG(detailed_oss.str());
	}

	std::string Board::getFen(void)
	{
		fen.setPlacement(this->generatePlacementFromBoard());
		fen.setCastling(this->castling.empty() ? "-" : this->castling);
		fen.setEnPassant(this->enPassant.empty() ? "-" : this->enPassant);
		fen.setColor(this->color);
		fen.setHalfMoveClock(this->halfMoveClock);
		fen.setFullMoves(this->fullMoves);

		return fen.generateFen();
	}

	const std::string &Board::getEnPassant(void) const
	{
		return this->enPassant;
	}

	char Board::getPiece(uint8_t rank, uint8_t file) const
	{
		if (rank >= BOARD_SIZE || file >= BOARD_SIZE)
		{
			LOG_THROW_ERROR(
				"Address <" + std::to_string(unsigned(rank)) + ", " + std::to_string(unsigned(file)) + "> is invalid",
				false);
			return '\0';
		}
		uint8_t square_idx = static_cast<uint8_t>(rank * 8 + file);

		for (int i = 0; i < 12; ++i)
		{
			if (test_bit(this->bitboards[i], square_idx))
			{
				static const char aliases[12] = {
					'P', 'N', 'B', 'R', 'Q', 'K',
					'p', 'n', 'b', 'r', 'q', 'k'
				};
				return aliases[i];
			}
		}
		return '\0';
	}

	void Board::move(uint8_t fromRank, uint8_t fromFile, uint8_t toRank, uint8_t toFile)
	{
		if (fromRank >= BOARD_SIZE || fromFile >= BOARD_SIZE || toRank >= BOARD_SIZE || toFile >= BOARD_SIZE)
		{
			LOG_THROW_ERROR(
				"Move addresses are invalid",
				false);
			return;
		}

		uint8_t old_sq = static_cast<uint8_t>(fromRank * 8 + fromFile);
		uint8_t new_sq = static_cast<uint8_t>(toRank * 8 + toFile);

		char active_c = this->getPiece(fromRank, fromFile);
		if (active_c == '\0')
		{
			LOG_THROW_ERROR(
				"Piece at board address <" + std::to_string(unsigned(fromRank)) + ", " + std::to_string(unsigned(fromFile)) + "> is invalid",
				false);
			return;
		}

		char target_c = this->getPiece(toRank, toFile);
		bool active_is_white = std::isupper(static_cast<unsigned char>(active_c));

		if (target_c != '\0')
		{
			bool target_is_white = std::isupper(static_cast<unsigned char>(target_c));
			if (active_is_white == target_is_white)
			{
				LOG_THROW_ERROR(
					"Cannot move to address <" + std::to_string(unsigned(toRank)) + ", " + std::to_string(unsigned(toFile)) + ">",
					false);
				return;
			}
			else
			{
				int captured_bb_idx = getBitboardIndex(target_c);
				if (captured_bb_idx >= 0)
				{
					clear_bit(this->bitboards[captured_bb_idx], new_sq);
					if (target_is_white)
					{
						clear_bit(this->white_occupancy, new_sq);
					}
					else
					{
						clear_bit(this->black_occupancy, new_sq);
					}
				}
			}
		}

		/* En passant logic */
		this->enPassant = "";
		if (std::tolower(static_cast<unsigned char>(active_c)) == 'p' && std::abs(static_cast<int>(toRank) - static_cast<int>(fromRank)) == 2)
		{
			this->enPassant = std::string(utils::getAlgebraicNotation(
				static_cast<uint8_t>((static_cast<int>(toRank) + static_cast<int>(fromRank)) / 2),
				fromFile));
		}

		int active_bb_idx = getBitboardIndex(active_c);
		if (active_bb_idx >= 0)
		{
			clear_bit(this->bitboards[active_bb_idx], old_sq);
			set_bit(this->bitboards[active_bb_idx], new_sq);
			if (active_is_white)
			{
				clear_bit(this->white_occupancy, old_sq);
				set_bit(this->white_occupancy, new_sq);
			}
			else
			{
				clear_bit(this->black_occupancy, old_sq);
				set_bit(this->black_occupancy, new_sq);
			}
		}

		this->combined_occupancy = this->white_occupancy | this->black_occupancy;

		this->logPieceAction("Moved", active_c, toRank, toFile, "🚀");
		io::PGN_RECORD(std::string(utils::getAlgebraicNotation(fromRank, fromFile)) + " " + utils::getAlgebraicNotation(toRank, toFile));
	}

	void Board::reset(const std::string &fenString)
	{
		this->fen = Fen(fenString);
		this->castling = fen.getCastling();
		this->enPassant = fen.getEnPassant() == "-" ? "" : fen.getEnPassant();
		this->color = fen.getColor();
		this->halfMoveClock = static_cast<uint8_t>(fen.getHalfMoveClock());
		this->fullMoves = static_cast<uint8_t>(fen.getFullMoves());
		this->buildBoard(fen.getPlacement());
		logger::INFO("Board reset to FEN: " + fenString);
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
				char c = this->getPiece(static_cast<uint8_t>(i), j);
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
}
