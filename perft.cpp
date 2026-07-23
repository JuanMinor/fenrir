/*
 *   Copyright (c) 2026 Juan Minor
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * Standalone perft (performance test) binary: recursively counts leaf
 * nodes at a fixed depth from a given FEN, to verify move-generation
 * correctness against known reference node counts and measure raw
 * move-generation throughput. Built separately from the main `fenrir`
 * UCI/search binary since it exercises only the rules core
 * (include/engine, include/chess) -- no search, NN, or UCI involved.
 * tools/perft.py drives this binary and compares its output against
 * published reference counts for standard test positions.
 *
 * Usage: perft <depth> <fen>
 */

#include "include/engine/engine.h"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    uint64_t perft(chess::Engine &engine, int depth)
    {
        if (depth == 0)
        {
            return 1;
        }

        uint64_t nodes = 0;
        for (const chess::Move &move : engine.generate_all_moves())
        {
            engine.make_move_fast(move);
            nodes += perft(engine, depth - 1);
            engine.undo_move();
        }
        return nodes;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <depth> <fen>\n";
        return EXIT_FAILURE;
    }

    try
    {
        int depth = std::stoi(argv[1]);
        std::string fen = argv[2];

        chess::Engine engine(fen);

        auto start = std::chrono::steady_clock::now();
        uint64_t nodes = perft(engine, depth);
        auto end = std::chrono::steady_clock::now();

        double seconds = std::chrono::duration<double>(end - start).count();
        uint64_t nps = seconds > 0.0 ? static_cast<uint64_t>(static_cast<double>(nodes) / seconds) : 0;

        std::cout << "Nodes generated: " << nodes << "\n";
        std::cout << "Time taken: " << seconds << " seconds\n";
        std::cout << "NPS: " << nps << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
