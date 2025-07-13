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

#include <string>
#include <sstream>
#include <vector>
#include "include/chess/board.h"
#include "include/logger/logger.h"
#include "include/chess/piece.h"
#include "include/utils/utils.h"

namespace fenrir
{
    class Moves
    {
        Moves();
        ~Moves();
        Moves(const Moves &) = delete;
        Moves &operator=(const Moves &) = delete;

        void __capture__(const Piece *__piece, const Piece *__target_piece, std::vector<std::pair<const std::string, const std::string>> &__moves);
        void __log_generated_moves__(const Piece *__piece, const std::vector<std::pair<const std::string, const std::string>> &__moves) const;
        void __pawn__(const Piece *__piece, const Board *__board, std::vector<std::pair<const std::string, const std::string>> &__moves);

    public:
        static Moves &get_instance();

        void generate_moves(const Piece *__piece, const Board *__board, std::vector<std::pair<const std::string, const std::string>> &__moves);
    };
}
