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
		this->halfMoveClock = fen.getHalfMoveClock();
		this->fullMoves = fen.getFullMoves();

		this->buildBoard(fen.getPlacement());

		logger::INFO("Board initialized with FEN: " + fen.getPlacement());
	}

	Board::~Board()
	{
		for (int rank = BOARD_SIZE - 1; rank >= 0; --rank)
		{
			for (uint8_t file = 0; file < BOARD_SIZE; ++file)
			{
				Piece *piece = this->board.at(rank).at(file);
				if (piece != nullptr)
				{
					this->logPieceAction("Destroyed", piece, utils::getAlgebraicNotation(rank, file), "❌");
					delete piece;
				}
			}
		}
		logger::INFO("Board destroyed and all pieces cleaned up.");
	}

	/* Private */
	void Board::buildBoard(const std::string &placement)
	{
		size_t size = placement.size();
		uint8_t rank = BOARD_SIZE - 1, file = 0;

		this->board.resize(BOARD_SIZE, std::vector<Piece *>(BOARD_SIZE, nullptr));

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
				uint8_t empties = c - '0';
				for (uint8_t j = 0; j < empties; ++j)
				{
					this->board[rank][file++] = nullptr;
				}
				continue;
			}
			Piece *piece = new Piece(c, rank, file);
			this->board[rank][file++] = piece;
			this->logPieceAction("Created", piece, utils::getAlgebraicNotation(rank, file - 1), "✅");
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
				const Piece *piece = this->board[rank][file];
				if (!piece)
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
					placement += piece->getAlias();
				}
			}
			if (empty_squares)
			{
				placement += static_cast<char>('0' + empty_squares);
			}
			if (rank)
			{
				placement += '/';
			}
		}
		return placement;
	}

	void Board::logPieceAction(const std::string &action, const Piece *piece, const std::string &position, const std::string &emoji)
	{
		const std::string color = piece->getColor() == WHITE ? "white" : "black";
		std::stringstream oss;
		oss << action << " " << color << " " << PIECE_NAMES.at(std::tolower(piece->getAlias(), std::locale()))
			<< " in position " << position << " " << emoji;

		// Use stringstream as fallback for std::format compatibility
		std::stringstream detailed_oss;
		detailed_oss << oss.str() << " (" << piece->getValue() << " value) at <"
					 << unsigned(piece->getRank()) << ", " << unsigned(piece->getFile()) << ">";

		logger::DEBUG(detailed_oss.str());
	}

	std::vector<std::vector<Piece *>> Board::getBoard(void) const
	{
		return this->board;
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

	Piece *Board::getPiece(const uint8_t &rank, const uint8_t &file) const
	{
		if (rank >= BOARD_SIZE || file >= BOARD_SIZE)
		{
			LOG_THROW_ERROR(
				("Address <" + std::to_string(unsigned(rank)) + ", " + std::to_string(unsigned(file)) + "> is invalid").c_str(),
				false);
			return nullptr;
		}
		return this->board.at(rank).at(file);
	}

	void Board::move(Piece *&piece, const uint8_t &rank, const uint8_t &file)
	{
		if (rank >= BOARD_SIZE || file >= BOARD_SIZE)
		{
			LOG_THROW_ERROR(
				("Address <" + std::to_string(unsigned(rank)) + ", " + std::to_string(unsigned(file)) + "> is invalid").c_str(),
				false);
			return;
		}
		uint8_t oldRank = piece->getRank(), oldFile = piece->getFile();
		if (this->board[oldRank][oldFile] == nullptr)
		{
			LOG_THROW_ERROR(
				("Piece at board address <" + std::to_string(unsigned(oldRank)) + ", " + std::to_string(unsigned(oldFile)) + "> is invalid").c_str(),
				false);
			return;
		}

		if (this->board[rank][file] != nullptr &&
			this->board[rank][file]->getColor() == piece->getColor())
		{
			LOG_THROW_ERROR(
				("Cannot move to address <" + std::to_string(unsigned(rank)) + ", " + std::to_string(unsigned(file)) + ">").c_str(),
				false);
			return;
		}

		/* En passant logic */
		this->enPassant = "";
		if (std::tolower(piece->getAlias()) == 'p' && std::abs(rank - piece->getRank()) == 2)
		{
			this->enPassant = std::string(utils::getAlgebraicNotation(
				(rank + piece->getRank()) / 2,
				piece->getFile()));
		}

		piece->setRank(rank);
		piece->setFile(file);
		this->board[rank][file] = piece;
		this->board[oldRank][oldFile] = nullptr;

		this->logPieceAction("Moved", piece, utils::getAlgebraicNotation(rank, file), "🚀");
		io::PGN_RECORD(std::string(utils::getAlgebraicNotation(oldRank, oldFile)) + " " + utils::getAlgebraicNotation(rank, file));
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
			std::cout << color::Modifier(color::Color::FG_YELLOW) << unsigned(i + 1) << " "
					  << color::Modifier(color::Color::RESET);
			for (uint8_t j = 0; j < BOARD_SIZE; ++j)
			{
				if (this->board.at(i).at(j))
				{
					std::cout << color::Modifier(this->board.at(i).at(j)->getColor() == BLACK
													 ? color::Color::FG_CYAN
													 : color::Color::RESET)
							  << this->board.at(i).at(j)->getAlias()
							  << color::Modifier(color::Color::RESET) << " ";
					continue;
				}
				std::cout << ". ";
			}
			std::cout << color::Modifier(color::Color::FG_YELLOW) << unsigned(i + 1)
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
