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

#include <iostream>

#include "include/modifier/modifier.h"
#include "include/chess/moves.h"

namespace loki
{
    Move::Move(const uint8_t &__rank, const uint8_t &__file)
    {
        this->rank = __rank;
        this->file = __file;
    }

    Move::~Move() {}

    uint8_t Move::get_rank(void) const
    {
        return this->rank;
    }

    uint8_t Move::get_file(void) const
    {
        return this->file;
    }

    Moves::Moves() {}

    Moves::~Moves() {}

    void __add_move__(const uint8_t &__rank, const u_int8_t &__file, std::vector<const Move *> &__moves)
    {
        __moves.emplace_back(new Move(__rank, __file));
    }

    void __pawn_capture_and_add_move__(const uint8_t &__rank, const u_int8_t &__file, const Piece *__piece, const Board *__board, std::vector<const Move *> &__moves)
    {
        if (__file >= BOARD_MAX_LEFT && __file <= BOARD_MAX_RIGHT)
        {
            const Piece *piece = __board->get_board().at(__rank).at(__file);
            if (piece && piece->get_color() != __piece->get_color())
            {
                __add_move__(__rank, __file, __moves);
            }
        }
        return;
    }

    void Moves::__pawn__(const uint8_t &__rank, const u_int8_t &__file, const Board *__board, const Fen *__fen, std::vector<const Move *> &__moves) const
    {
        const Piece *piece = __board->get_board().at(__rank).at(__file);
        if (!piece)
        {
            return;
        }
        uint8_t rank = -1;
        uint8_t file = -1;
        // @pawn has not moved yet. Entitled to move two squares forward
        if (!piece->get_moved())
        {
            rank = piece->get_color() == BLACK ? __rank - 2 : __rank + 2;
            if (!__board->get_board().at(rank).at(__file))
            {
                __add_move__(rank, __file, __moves);
            }
        }
        rank = piece->get_color() == BLACK ? __rank - 1 : __rank + 1;
        if (!__board->get_board().at(rank).at(__file))
        {
            __add_move__(rank, __file, __moves);
        }
        // @look at right position
        if (__file < BOARD_MAX_RIGHT)
        {
            __pawn_capture_and_add_move__(rank, __file + 1, piece, __board, __moves);
        }
        // @look at left position
        if (__file > BOARD_MAX_LEFT)
        {
            __pawn_capture_and_add_move__(rank, __file - 1, piece, __board, __moves);
        }
        return;
    }

    const std::vector<const Move *> Moves::generate(const uint8_t &__rank, const u_int8_t &__file, const Board *__board, const Fen *__fen) const
    {
        std::vector<const Move *> moves = {};
        const Piece *piece = __board->get_board().at(__rank).at(__file);
        if (!piece || !__board)
        {
            return moves;
        }
        switch (piece->get_alias())
        {
        case 'p':
        case 'P':
            this->__pawn__(__rank, __file, __board, __fen, moves);
            break;
        default:
            break;
        }
        return moves;
    }

}