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

#include <cstring>
#include <cstddef>

#include "include/chess/fen.h"
#include "include/core/core.h"

namespace loki
{
    Fen::Fen(char *__fen)
    {
        std::vector<char *> tokens = this->__split__(__fen, " ");
        this->placement = tokens[0];
        this->color = (*tokens[1] == 'w' || *tokens[1] == 'W') ? WHITE : BLACK;
        this->castling = tokens[2];
        this->en_passant = tokens[3];
        this->halfmove_clock = (uint8_t)(*tokens[4] - '0');
        this->fullmoves = (uint8_t)(*tokens[5] - '0');
    }

    Fen::~Fen() {}

    std::vector<char *> Fen::__split__(char *__fen, const char *__delimiters) const
    {
        std::vector<char *> tokens;
        char *token = strtok(__fen, __delimiters);
        while (token != nullptr)
        {
            tokens.emplace_back(token);
            token = strtok(nullptr, __delimiters);
        }
        return tokens;
    }

    char *Fen::get_placement(void) const
    {
        return this->placement;
    }

    char *Fen::get_castling(void) const
    {
        return this->castling;
    }

    char *Fen::get_en_passant(void) const
    {
        return this->en_passant;
    }

    uint8_t Fen::get_color(void) const
    {
        return this->color;
    }

    uint8_t Fen::get_halfmove_clock(void) const
    {
        return this->halfmove_clock;
    }

    uint8_t Fen::get_fullmoves(void) const
    {
        return this->fullmoves;
    }
}