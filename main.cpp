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
#include <vector>

#include "include/chess/board.h"
#include "include/chess/fen.h"
#include "include/modifier/modifier.h"
#include "include/chess/moves.h"
#include "include/pgn/pgn.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "No arguments found. Supply a valid FEN string" << std::endl;
        return 1;
    }

    const loki::Fen *fen = new loki::Fen(argv[1]);

    loki::Board *board = new loki::Board(fen->get_placement());

    std::cout << color::Modifier(color::Color::FG_RED)
              << color::Modifier(color::Color::BG_WHITE)
              << "Black pieces are represented in cyan!"
              << color::Modifier(color::Color::RESET) << std::endl;

    board->print();

    std::cout << color::Modifier(color::Color::FG_MAGENTA) << "-----------------------"
              << color::Modifier(color::Color::RESET) << std::endl;

    board->move(board->get_board().at(1).at(1), 3, 1);
    board->move(board->get_board().at(7).at(1), 5, 2);
    board->move(board->get_board().at(1).at(3), 2, 3);
    board->move(board->get_board().at(6).at(0), 4, 0);

    board->print();

    std::vector<const loki::Move *> moves = loki::Moves().generate(
        3, 1, board, fen);

    std::cout << color::Modifier(color::Color::FG_GREEN)
              << "Moves we can make"
              << color::Modifier(color::Color::RESET) << std::endl;

    for (const loki::Move *move : moves)
    {
        std::cout << color::Modifier(color::Color::FG_YELLOW)
                  << "Move: (" << int(move->get_rank()) << ", "
                  << int(move->get_file()) << ") 🚀"
                  << color::Modifier(color::Color::RESET) << std::endl;
    }

    moves.clear();

    moves = loki::Moves().generate(
        6, 1, board, fen);

    std::cout << color::Modifier(color::Color::FG_GREEN)
              << "Moves we can make"
              << color::Modifier(color::Color::RESET) << std::endl;

    for (const loki::Move *move : moves)
    {
        std::cout << color::Modifier(color::Color::FG_YELLOW)
                  << "Move: (" << int(move->get_rank()) << ", "
                  << int(move->get_file()) << ") 🚀"
                  << color::Modifier(color::Color::RESET) << std::endl;
    }

    io::PGN_CREATE();

    delete board;

    return 0;
}
