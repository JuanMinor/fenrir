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
    class AbstractBoard
    {
    public:
        virtual ~AbstractBoard() = default;

        virtual char get_piece(uint8_t rank, uint8_t file) const = 0;
        virtual const std::string &get_en_passant(void) const = 0;

        /* Extended interface for move generation and check detection. */
        virtual uint8_t get_color(void) const = 0;
        virtual const std::string &get_castling_rights(void) const = 0;
        virtual uint64_t get_occupancy(uint8_t color) const = 0;
        virtual uint64_t get_combined_occupancy(void) const = 0;
        virtual uint64_t get_en_passant_bb(void) const = 0;
        virtual uint64_t get_bitboard(int index) const = 0;
        virtual uint8_t get_half_move_clock(void) const = 0;
    };

}
