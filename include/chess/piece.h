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

#include <stdint.h>
#include <unordered_map>
#include <locale>
#include "include/core/core.h"
#include "include/utils/utils.h"

namespace fenrir
{
	class Piece
	{
	private:
		char alias;
		uint8_t value;
		uint8_t color;
		uint8_t rank;
		uint8_t file;
		bool moved;

		// Helper method to check if the piece has moved
		bool hasPieceMoved(void) const;

	public:
		Piece(const char &alias, const uint8_t &rank, const uint8_t &file);
		~Piece();

		// Accessors and mutators
		char getAlias(void) const;
		uint8_t getValue(void) const;
		uint8_t getColor(void) const;
		uint8_t getRank(void) const;
		void setRank(const uint8_t &rank);
		uint8_t getFile(void) const;
		void setFile(const uint8_t &file);
		bool getMoved(void) const;
		void setMoved(const bool &moved);
	};
}
