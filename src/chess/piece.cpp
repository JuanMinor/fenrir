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

#include <unordered_map>

#include "include/chess/piece.h"
#include "include/core/core.h"

namespace loki
{
    std::unordered_map<char, uint8_t>
        values = {
            {'P', 1},
            {'p', 1},
            {'N', 3},
            {'n', 3},
            {'B', 3},
            {'b', 3},
            {'R', 5},
            {'r', 5},
            {'Q', 9},
            {'q', 9},
            {'K', 25},
            {'k', 25}};

    Piece::Piece(const char &__alias, const uint8_t &__rank, const uint8_t &__file)
    {
        this->alias = __alias;
        this->value = values.at(__alias);
        this->color = isupper(__alias) ? WHITE : BLACK;
        this->rank = __rank;
        this->file = __file;
        this->moved = this->__has_piece_moved__();
    }

    Piece::~Piece()
    {
    }

    // @methods
    bool Piece::__has_piece_moved__(void) const
    {
        if ((this->alias == 'p' && this->rank == 6) || (this->alias == 'P' && this->rank == 1))
        {
            return false;
        }
        return true;
    }

    char Piece::get_alias(void) const
    {
        return this->alias;
    }

    uint8_t Piece::get_value(void) const
    {
        return this->value;
    }

    uint8_t Piece::get_color(void) const
    {
        return this->color;
    }

    uint8_t Piece::get_rank(void) const
    {
        return this->rank;
    }

    void Piece::set_rank(const uint8_t &__rank)
    {
        this->rank = __rank;
        this->moved = this->__has_piece_moved__();
    }

    uint8_t Piece::get_file(void) const
    {
        return this->file;
    }

    void Piece::set_file(const uint8_t &__file)
    {
        this->file = __file;
    }

    bool Piece::get_moved(void) const
    {
        return this->moved;
    }

    void Piece::set_moved(const bool &__moved)
    {
        this->moved = __moved;
    }
}