/*
 *   Copyright (c) 2025 Juan Minor

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

#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <format>
#include "include/core/core.h"
#include "include/chess/fen.h"
#include "include/logger/logger.h"
#include "include/modifier/modifier.h"
#include "include/pgn/pgn.h"
#include "include/chess/piece.h"
#include "include/utils/utils.h"

namespace fenrir
{
	class Board
	{
	private:
		std::vector<std::vector<Piece *>> board;
		std::string castling;
		std::string enPassant;
		uint8_t color;
		uint8_t halfMoveClock;
		uint8_t fullMoves;
		Fen fen;

		void buildBoard(const std::string &placement);
		std::string generatePlacementFromBoard(void) const;
		void logPieceAction(const std::string &action, const Piece *piece, const std::string &position, const std::string &emoji);

	public:
		Board(const std::string &fenString);
		~Board();

		// Accessors and mutators
		std::vector<std::vector<Piece *>> getBoard(void) const;
		std::string getFen(void);
		const std::string getEnPassant(void) const;
		Piece *getPiece(const uint8_t &rank, const uint8_t &file) const;
		void move(Piece *&piece, const uint8_t &rank, const uint8_t &file);
		void print(void) const;
	};
}
