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
#include <string>
#include <vector>
#include "include/engine/engine.h"

int main(int argc, char *argv[])
{
    try
    {
        fenrir::Engine engine = fenrir::Engine();
        engine.print_board();
        engine.make_move("b2", "b4");
        engine.make_move("b8", "c6");
        engine.make_move("d2", "d3");
        engine.make_move("a7", "a5");
        engine.print_board();

        engine.reset();
        engine.print_board();

        // test rook moves
        engine.make_move("a2", "b3");

        // move generation
        const std::vector<std::pair<const std::string, const std::string>> moves = engine.generate_moves("a1");

        std::cout << "Generated moves for piece" << std::endl;
        for (const auto &move : moves)
        {
            std::cout << move.first << " -> " << move.second << std::endl;
        }
        std::cout << "Total moves: " << moves.size() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
