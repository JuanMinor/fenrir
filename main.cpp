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
#include "include/pgn/pgn.h"

int main(int argc, char *argv[])
{
    // Validate input arguments
    if (argc < 2)
    {
        std::cerr << "Error: No arguments found. Supply a valid FEN string." << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        // Initialize FEN and Board
        const loki::Fen fen(argv[1]);
        std::cout << color::Modifier(color::Color::FG_BLUE)
                  << "FEN: " << fen.get_placement()
                  << color::Modifier(color::Color::RESET) << std::endl;

        loki::Board board(fen.get_placement());

        board.print();

        std::cout << color::Modifier(color::Color::FG_MAGENTA) << "-----------------------"
                  << color::Modifier(color::Color::RESET) << std::endl;

        // Perform some moves
        board.move(board.get_board().at(1).at(1), 3, 1);
        board.move(board.get_board().at(7).at(1), 5, 2);
        board.move(board.get_board().at(1).at(3), 2, 3);
        board.move(board.get_board().at(6).at(0), 4, 0);

        board.print();

        // Create PGN file
        io::PGN_CREATE();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
