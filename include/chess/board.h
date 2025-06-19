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
#include "include/utils/utils.h"

namespace fenrir
{
    class Board
    {
    private:
        std::vector<std::vector<Piece *>> board;
        std::string en_passant;

        // Helper method to get algebraic notation for a position
        void __log_piece_action__(const std::string &__action, const Piece *__piece, const std::string &__position, const std::string &__emoji);

    public:
        Board(const std::string &__placement, const std::string &__en_passant = "");
        ~Board();

        // Accessors and mutators
        std::vector<std::vector<Piece *>> get_board(void) const;
        const std::string get_en_passant(void) const;
        Piece *get_piece(const uint8_t &__rank, const uint8_t &__file) const;
        void move(Piece *&__piece, const uint8_t &__rank, const uint8_t &__file);
        void print(void) const;
    };
}

#endif
