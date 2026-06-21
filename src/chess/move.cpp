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

#include "include/chess/move.h"
#include "include/utils/utils.h"

namespace fenrir
{
	Move::Move(const std::string &from, const std::string &to,
			   MoveType type, char promotion)
		: from_square(0), to_square(0), moveType(type), promotionPiece(promotion), invalid_squares(nullptr)
	{
		uint8_t rank1 = 0, file1 = 0;
		uint8_t rank2 = 0, file2 = 0;
		bool from_ok = false;
		bool to_ok = false;

		try
		{
			utils::parseAlgebraicNotation(from, rank1, file1);
			from_ok = true;
		}
		catch (...) {}

		try
		{
			utils::parseAlgebraicNotation(to, rank2, file2);
			to_ok = true;
		}
		catch (...) {}

		if (from_ok && to_ok)
		{
			from_square = static_cast<uint8_t>(rank1 * 8 + file1);
			to_square = static_cast<uint8_t>(rank2 * 8 + file2);
		}
		else
		{
			invalid_squares = std::make_unique<std::pair<std::string, std::string>>(from, to);
		}
	}

	Move::Move(uint8_t fromSquare, uint8_t toSquare,
			   MoveType type, char promotion)
		: from_square(fromSquare), to_square(toSquare), moveType(type), promotionPiece(promotion), invalid_squares(nullptr)
	{
	}

	Move::Move(const Move& other)
		: from_square(other.from_square),
		  to_square(other.to_square),
		  moveType(other.moveType),
		  promotionPiece(other.promotionPiece),
		  invalid_squares(nullptr)
	{
		if (other.invalid_squares)
		{
			invalid_squares = std::make_unique<std::pair<std::string, std::string>>(*other.invalid_squares);
		}
	}

	Move& Move::operator=(const Move& other)
	{
		if (this != &other)
		{
			from_square = other.from_square;
			to_square = other.to_square;
			moveType = other.moveType;
			promotionPiece = other.promotionPiece;
			if (other.invalid_squares)
			{
				invalid_squares = std::make_unique<std::pair<std::string, std::string>>(*other.invalid_squares);
			}
			else
			{
				invalid_squares.reset();
			}
		}
		return *this;
	}

	Move::~Move() {}

	/* Getters */
	std::string Move::getFrom() const
	{
		if (invalid_squares)
		{
			return invalid_squares->first;
		}
		return utils::getAlgebraicNotation(static_cast<uint8_t>(from_square / 8), static_cast<uint8_t>(from_square % 8));
	}

	std::string Move::getTo() const
	{
		if (invalid_squares)
		{
			return invalid_squares->second;
		}
		return utils::getAlgebraicNotation(static_cast<uint8_t>(to_square / 8), static_cast<uint8_t>(to_square % 8));
	}

	uint8_t Move::getFromSquare() const
	{
		return this->from_square;
	}

	uint8_t Move::getToSquare() const
	{
		return this->to_square;
	}

	MoveType Move::getMoveType() const
	{
		return this->moveType;
	}

	char Move::getPromotionPiece() const
	{
		return this->promotionPiece;
	}

	/* Utility methods */
	bool Move::isCapture() const
	{
		return this->moveType == MoveType::CAPTURE || this->moveType == MoveType::EN_PASSANT;
	}

	bool Move::isPromotion() const
	{
		return this->moveType == MoveType::PROMOTION;
	}

	bool Move::isCastling() const
	{
		return this->moveType == MoveType::CASTLE_KINGSIDE || this->moveType == MoveType::CASTLE_QUEENSIDE;
	}

	std::string Move::toAlgebraicNotation() const
	{
		return getFrom() + getTo();
	}

	std::string Move::toString() const
	{
		std::string result = "(";

		switch (this->moveType)
		{
		case MoveType::CAPTURE:
			result += "Capture";
			break;
		case MoveType::EN_PASSANT:
			result += "En passant";
			break;
		case MoveType::CASTLE_KINGSIDE:
			result += "Castling king side";
			break;
		case MoveType::CASTLE_QUEENSIDE:
			result += "Castling queen side";
			break;
		case MoveType::PROMOTION:
			result += "Promotion";
			if (this->promotionPiece != '\0')
			{
				result += " to ";
				result += this->promotionPiece;
			}
			break;
		case MoveType::NORMAL:
			result += "Normal move";
			break;
		default:
			result += "Invalid action";
			break;
		}

		result += ")" + getFrom() + "->" + getTo();

		return result;
	}

	std::string Move::toUCINotation() const
	{
		std::string result = getFrom() + getTo();

		if (this->isPromotion() && this->promotionPiece != '\0')
		{
			result += static_cast<char>(std::tolower(static_cast<unsigned char>(this->promotionPiece)));
		}

		return result;
	}
}
