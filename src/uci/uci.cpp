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

#include "include/uci/uci.h"
#include "include/tuner/tuning_parameters.h"
#include <iostream>
#include <sstream>

namespace chess
{
    /**
     * Handles the UCI "go" command and derives a time/simulation budget for
     * the search. Priority: explicit "movetime" wins outright; otherwise
     * "wtime"/"btime" allocate 1/30th of the side-to-move's remaining clock
     * plus 3/4 of its increment (floored at 100ms); otherwise an explicit
     * "nodes" count is used as a simulation limit with no time bound. If the
     * GUI omits the side-to-move's own clock, the 1000ms default applies.
     * When only a time budget is known, it's converted to an estimated
     * simulation count (at ~25000 nodes/sec, floored at 1200) for
     * reporting/consistency purposes.
     */
    void UCI::parse_go(const std::string &command)
    {
        int simulations = -1;
        int move_time = -1;
        int wtime = -1, btime = -1;
        int winc = 0, binc = 0;
        std::istringstream is(command);
        std::string token;
        is >> token;

        while (is >> token)
        {
            if (token == "nodes")
                is >> simulations;
            else if (token == "movetime")
                is >> move_time;
            else if (token == "wtime")
                is >> wtime;
            else if (token == "btime")
                is >> btime;
            else if (token == "winc")
                is >> winc;
            else if (token == "binc")
                is >> binc;
        }

        int allocated_time_ms = 1000;
        bool white_to_move = engine->get_board_view().get_color() == 0;
        int time_left = white_to_move ? wtime : btime;
        int increment = white_to_move ? winc : binc;

        if (move_time != -1)
        {
            allocated_time_ms = move_time;
        }
        else if (time_left > 0)
        {
            allocated_time_ms = time_left / 30 + (increment * 3) / 4;

            /* Never budget into the flag: after the search deadline the
             * engine still pays for thread joins and in-flight NN batches
             * (~100-300ms), so keep a reserve off the remaining clock and
             * let the floor drop in scrambles instead of overspending. */
            const int reserve_ms = 200;
            if (allocated_time_ms > time_left - reserve_ms)
                allocated_time_ms = time_left - reserve_ms;
            if (allocated_time_ms < 50)
                allocated_time_ms = 50;
        }
        else if (simulations != -1)
        {
            allocated_time_ms = -1;
        }

        if (simulations == -1 && allocated_time_ms != -1)
        {
            int estimated_nps = 25000;
            simulations = static_cast<int>((allocated_time_ms / 1000.0) * estimated_nps);
            if (simulations < 1200)
                simulations = 1200;
        }

        chess::Move best_move = this->search->find_best_move(*engine, allocated_time_ms, simulations);
        std::cout << "bestmove " << best_move.to_uci_notation() << std::endl;
    }

    /**
     * Handles the UCI "position" command: sets up the board from either
     * "startpos" or a "fen ..." spec, then applies any trailing "moves ..."
     * list in UCI notation.
     */
    void UCI::parse_position(const std::string &command)
    {
        std::istringstream is(command);
        std::string token;
        is >> token;

        is >> token;
        if (token == "startpos")
        {
            /* Engine::reset() returns to the FEN the engine was constructed
             * with, which may be a custom position from an earlier
             * "position fen" command — so rebuild from the standard start. */
            engine = std::make_unique<Engine>();
            is >> token;
        }
        else if (token == "fen")
        {
            std::string fen;
            while (is >> token && token != "moves")
            {
                fen += token + " ";
            }
            if (!fen.empty())
                fen.pop_back();
            engine = std::make_unique<Engine>(fen);
        }

        while (is >> token)
        {
            if (token == "moves")
                continue;

            bool applied = false;
            std::vector<Move> legal_moves = engine->generate_all_moves();
            for (const auto &move : legal_moves)
            {
                if (move.to_uci_notation() == token)
                {
                    engine->make_move(move);
                    applied = true;
                    break;
                }
            }

            /* Stop instead of skipping: applying later moves to a board that
             * is missing this one would silently desync from the GUI. */
            if (!applied)
            {
                std::cout << "info string illegal move '" << token << "' in position command; ignoring rest of move list" << std::endl;
                break;
            }
        }
    }

    UCI::UCI()
    {
        engine = std::make_unique<Engine>();

        tuner::TuningParameters tuning(true);
        evaluator = std::make_unique<nn::NN>("onnx/fenrir.onnx", tuning.get_gpu_id(0), tuning.get_batch_size(), tuning.get_batch_timeout_ms());
        search = std::make_unique<mcts::Tree>(evaluator.get(), tuning.get_search_threads(), tuning.get_pipeline_target());
    }

    UCI::~UCI() = default;

    void UCI::loop()
    {
        std::string line;
        while (std::getline(std::cin, line))
        {
            std::istringstream is(line);
            std::string token;
            is >> std::skipws >> token;

            if (token == "uci")
            {
                std::cout << "id name Fenrir " << Engine::version() << std::endl;
                std::cout << "id author Juan Minor" << std::endl;
                std::cout << "uciok" << std::endl;
            }
            else if (token == "isready")
            {
                std::cout << "readyok" << std::endl;
            }
            else if (token == "ucinewgame")
            {
                /* Fresh engine, not reset(): reset() returns to the FEN this
                 * engine was constructed with, not the standard start. */
                engine = std::make_unique<Engine>();
            }
            else if (token == "position")
            {
                parse_position(line);
            }
            else if (token == "go")
            {
                parse_go(line);
            }
            else if (token == "quit")
            {
                break;
            }
        }
    }
}
