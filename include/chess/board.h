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

#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <vector>
#include "include/chess/piece.h"

namespace loki
{
    class Board
    {
    private:
        std::vector<std::vector<Piece *>> board;

        // Helper method to get algebraic notation for a position
        std::string __get_algebraic_notation__(const uint8_t &__rank, const uint8_t &__file) const;

    public:
        Board(const char *__placement);
        ~Board();

        // Accessors and mutators
        std::vector<std::vector<Piece *>> get_board(void) const;
        void move(Piece *&__piece, const uint8_t &__rank, const uint8_t &__file);
        void print(void) const;
    };
}

#endif // BOARD_H