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

#pragma once

#include <string>
#include <sstream>
#include <vector>
#include "include/logger/logger.h"
#include "include/chess/move.h"
#include "include/utils/utils.h"
#include "include/abstract/board.h"

namespace fenrir
{
	class Moves
	{
		Moves();
		~Moves();
		Moves(const Moves &) = delete;
		Moves &operator=(const Moves &) = delete;

		void generateBishopMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const;
		void addCaptureMove(uint8_t fromRank, uint8_t fromFile, char piece_char, uint8_t toRank, uint8_t toFile, char target_char, std::vector<Move> &moves) const;
		void generateKingMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const;
		void generateKnightMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const;
		void logGeneratedMoves(char piece_char, uint8_t rank, uint8_t file, const std::vector<Move> &moves) const;
		void generatePawnMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const;
		void generateQueenMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const;
		void generateRookMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const;
		void slideInDirections(uint8_t fromRank, uint8_t fromFile, char piece_char, const AbstractBoard &board, std::vector<Move> &moves, const int8_t directionVectors[][2], size_t numDirections, bool singleDepth = false) const;

	public:
		static Moves &getInstance();

		void generateMoves(uint8_t rank, uint8_t file, const AbstractBoard &board, std::vector<Move> &moves) const;
	};
}
