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
#include <include/core/core.h>

namespace fenrir
{
	class Move
	{
		std::string from;
		std::string to;
		MoveType moveType;
		char promotionPiece;

	public:
		Move(const std::string &from, const std::string &to,
			 MoveType moveType = MoveType::NORMAL, char promotionPiece = '\0');
		~Move();

		/* Getters */
		const std::string &getFrom() const;
		const std::string &getTo() const;
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
