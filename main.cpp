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
#include "include/chess/move.h"
#include "include/engine/engine.h"

int main(int argc, char *argv[])
{
	try
	{
		fenrir::Engine engine = fenrir::Engine();
		engine.print_board();
		engine.make_move(fenrir::Move("b2", "b4"));
		engine.make_move(fenrir::Move("b8", "c6"));
		engine.make_move(fenrir::Move("d2", "d3"));
		engine.make_move(fenrir::Move("a7", "a5"));
		engine.print_board();

		engine.reset();
		engine.print_board();

		/* Test rook moves */
		/* engine.make_move("a2", "b3"); */
		engine.make_move(fenrir::Move("a2", "b3"));
		engine.print_board();

		/* Generate moves */
		const std::vector<fenrir::Move> moves = engine.generate_moves("a1");

		std::cout << "Generated moves for piece" << std::endl;
		for (const auto &move : moves)
		{
			std::cout << move.get_from() << " -> " << move.get_to() << std::endl;
		}
		std::cout << "Total moves: " << moves.size() << std::endl;
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
