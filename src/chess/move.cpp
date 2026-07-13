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

#include "include/chess/move.h"
#include "include/utils/utils.h"

namespace chess
{
    Move::Move(const std::string &from, const std::string &to,
               MoveType type, char promotion)
        : from_square(0), to_square(0), move_type(type), promotion_piece(promotion), invalid_squares(nullptr)
    {
        uint8_t rank1 = 0, file1 = 0;
        uint8_t rank2 = 0, file2 = 0;
        bool from_ok = false;
        bool to_ok = false;

        try
        {
            utils::parse_algebraic_notation(from, rank1, file1);
            from_ok = true;
        }
        catch (...)
        {
        }

        try
        {
            utils::parse_algebraic_notation(to, rank2, file2);
            to_ok = true;
        }
        catch (...)
        {
        }

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

    Move::Move(uint8_t from, uint8_t to,
               MoveType type, char promotion)
        : from_square(from), to_square(to), move_type(type), promotion_piece(promotion), invalid_squares(nullptr)
    {
    }

    Move::Move(const std::string &from, const std::string &to, char promotion)
        : Move(from, to, (promotion != '\0') ? MoveType::PROMOTION : MoveType::NORMAL, promotion)
    {
    }

    Move::Move(uint8_t from, uint8_t to, char promotion)
        : Move(from, to, (promotion != '\0') ? MoveType::PROMOTION : MoveType::NORMAL, promotion)
    {
    }

    Move::Move(const Move &other)
        : from_square(other.from_square),
          to_square(other.to_square),
          move_type(other.move_type),
          promotion_piece(other.promotion_piece),
          invalid_squares(nullptr)
    {
        if (other.invalid_squares)
        {
            invalid_squares = std::make_unique<std::pair<std::string, std::string>>(*other.invalid_squares);
        }
    }

    Move &Move::operator=(const Move &other)
    {
        if (this != &other)
        {
            from_square = other.from_square;
            to_square = other.to_square;
            move_type = other.move_type;
            promotion_piece = other.promotion_piece;
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

    /**
     * @brief Get source square in algebraic notation (e.g., "e2").
     * @returns Source square as string.
     */
    std::string Move::get_from() const
    {
        if (invalid_squares)
        {
            return invalid_squares->first;
        }
        return utils::get_algebraic_notation(static_cast<uint8_t>(from_square / 8), static_cast<uint8_t>(from_square % 8));
    }

    /**
     * @brief Get destination square in algebraic notation (e.g., "e4").
     * @returns Destination square as string.
     */
    std::string Move::get_to() const
    {
        if (invalid_squares)
        {
            return invalid_squares->second;
        }
        return utils::get_algebraic_notation(static_cast<uint8_t>(to_square / 8), static_cast<uint8_t>(to_square % 8));
    }

    /**
     * @brief Get source square as numeric index (0-63).
     * @returns Source square index.
     */
    uint8_t Move::get_from_square() const
    {
        return this->from_square;
    }

    /**
     * @brief Get the move type (normal, capture, castling, promotion, etc.).
     * @returns Move type enumeration value.
     */
    MoveType Move::get_move_type() const
    {
        return this->move_type;
    }

    /**
     * @brief Get promotion piece character ('Q', 'R', 'B', 'N') if applicable.
     * @returns Promotion piece character, or '\0' if not a promotion.
     */
    char Move::get_promotion_piece() const
    {
        return this->promotion_piece;
    }

    /**
     * @brief Get destination square as numeric index (0-63).
     * @returns Destination square index.
     */
    uint8_t Move::get_to_square() const
    {
        return this->to_square;
    }

    /**
     * @brief Check if this move captures a piece.
     * @returns True if capture or en passant, false otherwise.
     */
    bool Move::is_capture() const
    {
        return this->move_type == MoveType::CAPTURE || this->move_type == MoveType::EN_PASSANT;
    }

    /**
     * @brief Check if this move is a pawn promotion.
     * @returns True if promotion, false otherwise.
     */
    bool Move::is_promotion() const
    {
        return this->move_type == MoveType::PROMOTION;
    }

    /**
     * @brief Check if this move is a castling move (kingside or queenside).
     * @returns True if castling, false otherwise.
     */
    bool Move::is_castling() const
    {
        return this->move_type == MoveType::CASTLE_KINGSIDE || this->move_type == MoveType::CASTLE_QUEENSIDE;
    }

    /**
     * @brief Convert move to algebraic notation (e.g., "e2e4").
     * @returns Algebraic notation string.
     */
    std::string Move::to_algebraic_notation() const
    {
        return get_from() + get_to();
    }

    /**
     * @brief Convert move to human-readable string with move type description.
     * @returns Descriptive string representation.
     */
    std::string Move::to_string() const
    {
        std::string result = "(";

        switch (this->move_type)
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
            if (this->promotion_piece != '\0')
            {
                result += " to ";
                result += this->promotion_piece;
            }
            break;
        case MoveType::NORMAL:
            result += "Normal move";
            break;
        default:
            result += "Invalid action";
            break;
        }

        result += ")" + get_from() + "->" + get_to();

        return result;
    }

    /**
     * @brief Convert move to UCI notation (e.g., "e2e4q" for promotion to queen).
     * @returns UCI notation string.
     */
    std::string Move::to_uci_notation() const
    {
        std::string result = get_from() + get_to();

        if (this->is_promotion() && this->promotion_piece != '\0')
        {
            result += static_cast<char>(std::tolower(static_cast<unsigned char>(this->promotion_piece)));
        }

        return result;
    }
}
