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

#pragma once

#include "include/chess/board.h"
#include "include/core/core.h"
#include "include/logger/logger.h"
#include "include/chess/moves.h"
#include "include/utils/utils.h"

namespace fenrir
{

    class FENRIR_API Engine final
    {
        static constexpr const char *default_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        std::string fen;
        Board board;

    public:
        Engine(const std::string &__fen = default_fen);
        ~Engine();

        std::vector<std::pair<const std::string, const std::string>> generate_moves(const std::string &__algebraic_address) const;
        std::string get_fen(void);
        void make_move(const std::string &__from_algebraic_address, const std::string &__to_algebraic_address);
        void print_board(void) const;
        void reset();

#ifndef NDEBUG
        char get_piece(const std::string &__algebraic_address) const;
#endif
    };
}