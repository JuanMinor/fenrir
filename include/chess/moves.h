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

#ifndef MOVES_H
#define MOVES_H

#include <ctype.h>
#include <stdint.h>
#include <vector>

#include "include/chess/board.h"
#include "include/chess/fen.h"
#include "include/chess/piece.h"

namespace loki
{
    class Move
    {
        uint8_t rank;
        uint8_t file;

    public:
        Move(const uint8_t &__rank, const uint8_t &__file);
        ~Move();

        // @methods
        uint8_t get_rank(void) const;
        uint8_t get_file(void) const;
    };

    class Moves
    {
        // @methods
        void __pawn_old__(const Piece *__piece, const Board *__board, const Fen *__fen, std::vector<const Move *> &__moves) const;
        void __pawn__(const uint8_t &__rank, const u_int8_t &__file, const Board *__board, const Fen *__fen, std::vector<const Move *> &__moves) const;

    public:
        Moves();
        ~Moves();

        // @methods
        const std::vector<const Move *> generate(const uint8_t &__rank, const u_int8_t &__file, const Board *__board, const Fen *__fen) const;
    };
}

#endif