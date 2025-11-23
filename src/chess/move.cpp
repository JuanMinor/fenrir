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

namespace fenrir
{
	Move::Move(const std::string &from, const std::string &to,
			   MoveType moveType, char promotionPiece)
		: from(from), to(to), moveType(moveType), promotionPiece(promotionPiece)
	{
	}

	Move::~Move() {}

	/* Getters */
	const std::string &Move::getFrom() const
	{
		return this->from;
	}

	const std::string &Move::getTo() const
	{
		return this->to;
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
	};

	bool Move::isPromotion() const
	{
		return this->moveType == MoveType::PROMOTION;
	};

	bool Move::isCastling() const
	{
		return this->moveType == MoveType::CASTLE_KINGSIDE || this->moveType == MoveType::CASTLE_QUEENSIDE;
	};

	std::string Move::toAlgebraicNotation() const
	{
		return from + to;
	};

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

		result += ")" + from + "->" + to;

		return result;
	}

	std::string Move::toUCINotation() const
	{
		std::string result = from + to;

		if (this->isPromotion() && this->promotionPiece != '\0')
		{
			result += static_cast<char>(std::tolower(static_cast<unsigned char>(this->promotionPiece)));
		}

		return result;
	}
}
