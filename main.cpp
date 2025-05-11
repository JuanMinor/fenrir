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
#include "include/engine/engine.h"

int main(int argc, char *argv[])
{
    try
    {
        loki::Engine engine = loki::Engine();
        engine.print_board();
        engine.make_move(1, 1, 3, 1);
        engine.make_move(7, 1, 5, 2);
        engine.make_move(1, 3, 2, 3);
        engine.make_move(6, 0, 4, 0);
        engine.print_board();

        engine.reset();
        engine.print_board();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
