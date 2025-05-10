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

#include "include/core/core.h"
#include "include/chess/piece.h"
#include "include/logger/logger.h"
#include <unordered_map>
#include <cctype>
#include <locale>

namespace loki
{
    // Mapping of piece aliases to their values
    std::unordered_map<char, uint8_t> values = {
        {'P', 1}, {'p', 1}, {'N', 3}, {'n', 3}, {'B', 3}, {'b', 3}, {'R', 5}, {'r', 5}, {'Q', 9}, {'q', 9}, {'K', 25}, {'k', 25}};

    Piece::Piece(const char &__alias, const uint8_t &__rank, const uint8_t &__file)
        : alias(__alias),
          value(values.at(__alias)),
          color(std::isupper(__alias, std::locale()) ? WHITE : BLACK),
          rank(__rank),
          file(__file),
          moved(this->__has_piece_moved__()) {}

    Piece::~Piece() {}

    bool Piece::__has_piece_moved__(void) const
    {
        return !((this->alias == 'p' && this->rank == 6) || (this->alias == 'P' && this->rank == 1));
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
        if (__rank < 0 || __rank > 7)
        {
            const char *error = "Rank must be between 1 and 8";
            throw std::out_of_range(error);
            logger::LOG_ERROR(error);
        }
        this->rank = __rank;
        this->moved = this->__has_piece_moved__();
    }

    uint8_t Piece::get_file(void) const
    {
        return this->file;
    }

    void Piece::set_file(const uint8_t &__file)
    {
        if (__file < 0 || __file > 7)
        {
            const char *error = "File must be between 1 and 8";
            throw std::out_of_range(error);
            logger::LOG_ERROR(error);
        }
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