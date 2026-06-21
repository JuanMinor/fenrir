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

	Piece::Piece(char pieceAlias, uint8_t pieceRank, uint8_t pieceFile)
		: alias(pieceAlias),
		  value(values.at(pieceAlias)),
		  color(std::isupper(pieceAlias, std::locale()) ? WHITE : BLACK),
		  rank(pieceRank),
		  file(pieceFile),
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

	void Piece::setFile(uint8_t pieceFile)
	{
		if (pieceFile > 7)
		{
			LOG_THROW_ERROR("File must be between 1 and 8", true);
		}
		this->file = pieceFile;
	}

	void Piece::setMoved(bool hasMoved)
	{
		this->moved = hasMoved;
	}

	void Piece::setRank(uint8_t pieceRank)
	{
		if (pieceRank > 7)
		{
			LOG_THROW_ERROR("Rank must be between 1 and 8", true);
		}
		this->rank = pieceRank;
		this->moved = this->hasPieceMoved();
	}


}
