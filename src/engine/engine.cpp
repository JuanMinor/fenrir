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
#include <include/engine/engine.h>

namespace loki
{

    Engine::Engine(const char *__fen) : fen(__fen), board(fen.get_placement())
    {
    }

    Engine::~Engine() {}

    void Engine::make_move(const uint8_t &__from_rank, const uint8_t &__from_file,
                           const uint8_t &__to_rank, const uint8_t &__to_file)
    {
        Piece *piece = board.get_board().at(__from_rank).at(__from_file);
        if (piece != nullptr)
        {
            board.move(piece, __to_rank, __to_file);
        }
    }

    void Engine::print_board(void) const
    {
        board.print();
    }

    void Engine::reset()
    {
        board.~Board();
        new (&board) Board(fen.get_placement());
    }
}