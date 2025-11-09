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
#include <sstream>
#include <vector>
#include "include/chess/board.h"
#include "include/logger/logger.h"
#include "include/chess/piece.h"
#include "include/utils/utils.h"

namespace fenrir
{
	class Moves
	{
		Moves();
		~Moves();
		Moves(const Moves &) = delete;
		Moves &operator=(const Moves &) = delete;

		void generateBishopMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves);
		void addCaptureMove(const Piece *piece, const Piece *targetPiece, std::vector<std::pair<const std::string, const std::string>> &moves);
		void generateKingMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves);
		void generateKnightMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves);
		void logGeneratedMoves(const Piece *piece, const std::vector<std::pair<const std::string, const std::string>> &moves) const;
		void generatePawnMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves);
		void generateQueenMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves);
		void generateRookMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves);
		void slideInDirections(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves, const int8_t directionVectors[][2], size_t numDirections, bool singleDepth = false);

	public:
		static Moves &getInstance();

		void generateMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves);
	};
}
