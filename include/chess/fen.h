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
		void __split__(const std::string &__fen, const std::string &__delimiters, std::vector<std::string> &__tokens) const;
		void __validate_chess_rules__(const std::string &__placement) const;
		void __validate_placement__(const std::string &__placement) const;
		void __validate_pawn_placement__(const std::vector<std::string> &__ranks) const;
		void __validate_king_safety__(const std::vector<std::string> &__ranks) const;

	public:
		Fen(const std::string &__fen, GameMode __game_mode = GameMode::PERMISSIVE);
		~Fen();

		// Getters for FEN components
		std::string get_placement(void) const;
		std::string get_castling(void) const;
		std::string get_en_passant(void) const;
		uint8_t get_color(void) const;
		uint32_t get_halfmove_clock(void) const;
		uint32_t get_fullmoves(void) const;

		// Setters for FEN components
		void set_placement(const std::string &__placement);
		void set_castling(const std::string &__castling);
		void set_en_passant(const std::string &__en_passant);
		void set_color(const uint8_t &__color);
		void set_halfmove_clock(const uint32_t &__halfmove_clock);
		void set_fullmoves(const uint32_t &__fullmoves);

		std::string generate_fen(void) const;
	};
}
