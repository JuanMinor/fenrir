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

#ifndef PIECE_H
#define PIECE_H

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
        bool __has_piece_moved__(void) const;

    public:
        Piece(const char &__alias, const uint8_t &__rank, const uint8_t &__file);
        ~Piece();

        // Accessors and mutators
        char get_alias(void) const;
        uint8_t get_value(void) const;
        uint8_t get_color(void) const;
        uint8_t get_rank(void) const;
        void set_rank(const uint8_t &__rank);
        uint8_t get_file(void) const;
        void set_file(const uint8_t &__file);
        bool get_moved(void) const;
        void set_moved(const bool &__moved);
    };
}

#endif // PIECE_H