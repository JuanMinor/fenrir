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

#ifndef ENGINE_H
#define ENGINE_H

#include "include/chess/board.h"
#include "include/core/core.h"
#include "include/chess/fen.h"

namespace loki
{

    class LOKI_API Engine
    {
        static constexpr const char *default_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        Fen fen;
        Board board;

    public:
        Engine(const char *__fen = default_fen);
        ~Engine();

        void make_move(const uint8_t &__from_rank, const uint8_t &__from_file,
                       const uint8_t &__to_rank, const uint8_t &__to_file);
        void print_board(void) const;
        void reset();
    };
}

#endif