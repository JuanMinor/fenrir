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

namespace chess
{
    class AbstractBoard
    {
    public:
        virtual ~AbstractBoard() = default;

        /**
         * @brief Get the bitboard for a specific piece type.
         * @param index Bitboard index (0-5: White P/N/B/R/Q/K, 6-11: Black p/n/b/r/q/k).
         * @returns 64-bit bitboard representing piece positions.
         */
        virtual uint64_t get_bitboard(int index) const = 0;

        /**
         * @brief Get castling rights string representation.
         * @returns Castling rights as UCI string (e.g., "KQkq", "-").
         */
        virtual const std::string &get_castling_rights(void) const = 0;

        /**
         * @brief Get the side to move.
         * @returns Color value: 0 for white, 1 for black.
         */
        virtual uint8_t get_color(void) const = 0;

        /**
         * @brief Get combined occupancy bitboard (all pieces, both colors).
         * @returns 64-bit bitboard with all occupied squares.
         */
        virtual uint64_t get_combined_occupancy(void) const = 0;

        /**
         * @brief Get en passant target square as string.
         * @returns En passant square as UCI notation string (e.g., "e3", "-").
         */
        virtual const std::string &get_en_passant(void) const = 0;

        /**
         * @brief Get en passant target square as bitboard.
         * @returns 64-bit bitboard with en passant square set, or 0 if none available.
         */
        virtual uint64_t get_en_passant_bb(void) const = 0;

        /**
         * @brief Get half-move clock (moves since last pawn move or capture).
         * @returns Half-move clock value (0-50 for draw rule).
         */
        virtual uint8_t get_half_move_clock(void) const = 0;

        /**
         * @brief Get piece character at a board position.
         * @param rank Rank index (0-7, 0 is rank 1 in chess notation).
         * @param file File index (0-7, 0 is file a in chess notation).
         * @returns Character representation of piece ('P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k', or ' ' for empty).
         */
        virtual char get_piece(uint8_t rank, uint8_t file) const = 0;

        /**
         * @brief Get occupancy bitboard for a specific color.
         * @param color Color to query: 0 for white, 1 for black.
         * @returns 64-bit bitboard with all pieces of the given color.
         */
        virtual uint64_t get_occupancy(uint8_t color) const = 0;
    };

}
