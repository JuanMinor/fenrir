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

#include "include/chess/piece.h"

namespace fenrir
{
	std::unordered_map<char, uint8_t> values = {
		{'P', 1}, {'p', 1}, {'N', 3}, {'n', 3}, {'B', 3}, {'b', 3}, {'R', 5}, {'r', 5}, {'Q', 9}, {'q', 9}, {'K', 25}, {'k', 25}};

	Piece::Piece(const char &alias, const uint8_t &rank, const uint8_t &file)
		: alias(alias),
		  value(values.at(alias)),
		  color(std::isupper(alias, std::locale()) ? WHITE : BLACK),
		  rank(rank),
		  file(file),
		  moved(this->hasPieceMoved())
	{
	}

	Piece::~Piece() {}

	bool Piece::hasPieceMoved(void) const
	{
		return !((this->alias == 'p' && this->rank == 6) || (this->alias == 'P' && this->rank == 1));
	}

	char Piece::getAlias(void) const
	{
		return this->alias;
	}

	uint8_t Piece::getColor(void) const
	{
		return this->color;
	}

	uint8_t Piece::getFile(void) const
	{
		return this->file;
	}

	bool Piece::getMoved(void) const
	{
		return this->moved;
	}

	uint8_t Piece::getRank(void) const
	{
		return this->rank;
	}

	uint8_t Piece::getValue(void) const
	{
		return this->value;
	}

	void Piece::setFile(const uint8_t &file)
	{
		if (file > 7)
		{
			LOG_THROW_ERROR("File must be between 1 and 8", true);
		}
		this->file = file;
	}

	void Piece::setMoved(const bool &moved)
	{
		this->moved = moved;
	}

	void Piece::setRank(const uint8_t &rank)
	{
		if (rank > 7)
		{
			LOG_THROW_ERROR("Rank must be between 1 and 8", true);
		}
		this->rank = rank;
		this->moved = this->hasPieceMoved();
	}
}
