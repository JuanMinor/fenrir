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
#include <memory>
#include <include/core/core.h>

namespace fenrir
{
	class Move
	{
		uint8_t from_square;
		uint8_t to_square;
		MoveType move_type;
		char promotion_piece;
		std::unique_ptr<std::pair<std::string, std::string>> invalid_squares;

	public:
		Move(const std::string &from, const std::string &to, char promotion = '\0');
		Move(uint8_t from, uint8_t to, char promotion = '\0');
		Move(const std::string &from, const std::string &to, MoveType type, char promotion = '\0');
		Move(uint8_t from, uint8_t to, MoveType type, char promotion = '\0');

		Move(const Move& other);
		Move& operator=(const Move& other);
		Move(Move&& other) noexcept = default;
		Move& operator=(Move&& other) noexcept = default;

		~Move();

		/* Getters */
		std::string get_from() const;
		std::string get_to() const;
		uint8_t get_from_square() const;
		uint8_t get_to_square() const;
		MoveType get_move_type() const;
		char get_promotion_piece() const;

		/* Utility methods */
		bool is_capture() const;
		bool is_promotion() const;
		bool is_castling() const;
		std::string to_algebraic_notation() const;
		std::string to_string() const;
		std::string to_uci_notation() const;
	};

}
