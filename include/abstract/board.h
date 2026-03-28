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

#pragma once

#include <string>
#include <cstdint>

namespace fenrir
{
	class Piece;

	class AbstractBoard
	{
	public:
		virtual ~AbstractBoard() = default;

		virtual Piece *getPiece(const uint8_t &rank, const uint8_t &file) const = 0;
		virtual const std::string &getEnPassant(void) const = 0;
	};
}
