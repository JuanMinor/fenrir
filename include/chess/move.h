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

        Move(const Move &other);
        Move &operator=(const Move &other);
        Move(Move &&other) noexcept = default;
        Move &operator=(Move &&other) noexcept = default;

        ~Move();

        /**
         * @brief Get source square in algebraic notation (e.g., "e2").
         * @returns Source square as string.
         */
        std::string get_from() const;

        /**
         * @brief Get source square as numeric index (0-63).
         * @returns Source square index.
         */
        uint8_t get_from_square() const;

        /**
         * @brief Get the move type (normal, capture, castling, promotion, etc.).
         * @returns Move type enumeration value.
         */
        MoveType get_move_type() const;

        /**
         * @brief Get promotion piece character ('Q', 'R', 'B', 'N') if applicable.
         * @returns Promotion piece character, or '\0' if not a promotion.
         */
        char get_promotion_piece() const;

        /**
         * @brief Get destination square in algebraic notation (e.g., "e4").
         * @returns Destination square as string.
         */
        std::string get_to() const;

        /**
         * @brief Get destination square as numeric index (0-63).
         * @returns Destination square index.
         */
        uint8_t get_to_square() const;

        /**
         * @brief Check if this move is a castling move (kingside or queenside).
         * @returns True if castling, false otherwise.
         */
        bool is_castling() const;

        /**
         * @brief Check if this move captures a piece.
         * @returns True if capture or en passant, false otherwise.
         */
        bool is_capture() const;

        /**
         * @brief Check if this move is a pawn promotion.
         * @returns True if promotion, false otherwise.
         */
        bool is_promotion() const;

        /**
         * @brief Convert move to algebraic notation (e.g., "e2e4").
         * @returns Algebraic notation string.
         */
        std::string to_algebraic_notation() const;

        /**
         * @brief Convert move to human-readable string with move type description.
         * @returns Descriptive string representation.
         */
        std::string to_string() const;

        /**
         * @brief Convert move to UCI notation (e.g., "e2e4q" for promotion to queen).
         * @returns UCI notation string.
         */
        std::string to_uci_notation() const;
    };

}
