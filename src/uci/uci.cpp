#include "include/uci/uci.h"
#include <iostream>
#include <sstream>

namespace fenrir
{
    UCI::UCI()
    {
        engine = std::make_unique<Engine>();
        evaluator = std::make_unique<NNEvaluator>("onnx/fenrir.onnx");
        search = std::make_unique<MCTSSearch>(evaluator.get(), 16);
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
                engine->reset();
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

    void UCI::parse_position(const std::string& command)
    {
        std::istringstream is(command);
        std::string token;
        is >> token; // consume "position"

        is >> token;
        if (token == "startpos")
        {
            engine->reset();
            is >> token; // consume "moves" if it exists
        }
        else if (token == "fen")
        {
            std::string fen;
            while (is >> token && token != "moves")
            {
                fen += token + " ";
            }
            if (!fen.empty()) fen.pop_back(); // strip trailing space
            engine = std::make_unique<Engine>(fen);
        }

        // Parse moves
        while (is >> token)
        {
            if (token == "moves") continue;

            std::vector<Move> legal_moves = engine->generate_all_moves();
            for (const auto& move : legal_moves)
            {
                if (move.to_uci_notation() == token)
                {
                    engine->make_move(move);
                    break;
                }
            }
        }
    }

    void UCI::parse_go(const std::string& command)
    {
        int simulations = -1;
        int move_time = -1;
        int wtime = -1, btime = -1;
        std::istringstream is(command);
        std::string token;
        is >> token; // consume "go"

        while (is >> token)
        {
            if (token == "nodes") is >> simulations;
            else if (token == "movetime") is >> move_time;
            else if (token == "wtime") is >> wtime;
            else if (token == "btime") is >> btime;
        }

        // Time management logic
        int allocated_time_ms = 1000; // default 1 second if nothing provided
        if (move_time != -1) {
            allocated_time_ms = move_time;
        } else if (wtime != -1 || btime != -1) {
            int time_left = (engine->get_board_view().get_color() == 0) ? wtime : btime;
            allocated_time_ms = time_left / 30; // standard rule of thumb: use 1/30th of remaining time
            if (allocated_time_ms < 100) allocated_time_ms = 100;
        } else if (simulations != -1) {
            allocated_time_ms = -1; // flag to use node limit instead of time
        }

		if (simulations == -1 && allocated_time_ms != -1)
		{
			int estimated_nps = 25000;
			simulations = static_cast<int>((allocated_time_ms / 1000.0) * estimated_nps);
			if (simulations < 1200)
				simulations = 1200;
		}

        Move best_move = search->find_best_move(*engine, allocated_time_ms, simulations);
        std::cout << "bestmove " << best_move.to_uci_notation() << std::endl;
    }
}
