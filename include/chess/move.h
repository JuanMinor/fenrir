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
		MoveType moveType;
		char promotionPiece;
		std::unique_ptr<std::pair<std::string, std::string>> invalid_squares;

	public:
		Move(const std::string &from, const std::string &to,
			 MoveType type = MoveType::NORMAL, char promotion = '\0');
		Move(uint8_t fromSquare, uint8_t toSquare,
			 MoveType type = MoveType::NORMAL, char promotion = '\0');

		Move(const Move& other);
		Move& operator=(const Move& other);
		Move(Move&& other) noexcept = default;
		Move& operator=(Move&& other) noexcept = default;

		~Move();

		/* Getters */
		std::string getFrom() const;
		std::string getTo() const;
		uint8_t getFromSquare() const;
		uint8_t getToSquare() const;
		MoveType getMoveType() const;
		char getPromotionPiece() const;

		/* Utility methods */
		bool isCapture() const;
		bool isPromotion() const;
		bool isCastling() const;
		std::string toAlgebraicNotation() const;
		std::string toString() const;
		std::string toUCINotation() const;
	};

}
