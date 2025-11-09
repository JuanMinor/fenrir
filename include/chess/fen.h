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

#pragma once

#include <regex>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "include/core/core.h"
#include "include/logger/logger.h"
#include "include/utils/utils.h"

namespace fenrir
{
	class Fen
	{
	private:
		GameMode game_mode;
		std::string placement;
		std::string castling;
		std::string en_passant;
		uint8_t color;
		uint32_t halfmove_clock;
		uint32_t fullmoves;

		// Helper method to split a FEN string into components
		void splitString(const std::string &fenString, const std::string &delimiters, std::vector<std::string> &tokens) const;
		void validateChessRules(const std::string &placement) const;
		void validatePlacement(const std::string &placement) const;
		void validatePawnPlacement(const std::vector<std::string> &ranks) const;
		void validateKingSafety(const std::vector<std::string> &ranks) const;

	public:
		Fen(const std::string &fenString, GameMode gameMode = GameMode::PERMISSIVE);
		~Fen();

		// Getters for FEN components
		std::string getPlacement(void) const;
		std::string getCastling(void) const;
		std::string getEnPassant(void) const;
		uint8_t getColor(void) const;
		uint32_t getHalfmoveClock(void) const;
		uint32_t getFullmoves(void) const;

		// Setters for FEN components
		void setPlacement(const std::string &placement);
		void setCastling(const std::string &castling);
		void setEnPassant(const std::string &enPassant);
		void setColor(const uint8_t &color);
		void setHalfmoveClock(const uint32_t &halfmoveClock);
		void setFullmoves(const uint32_t &fullmoves);

		std::string generateFen(void) const;
	};
}
