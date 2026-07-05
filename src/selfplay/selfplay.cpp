#include "include/selfplay/selfplay.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>
#include <chrono>

namespace fenrir
{
    SelfPlay::SelfPlay(int simulations_per_move, int num_games, const std::string& start_fen_arg)
        : simulations(simulations_per_move), max_games(num_games), start_fen(start_fen_arg)
    {
    }

    std::string SelfPlay::get_output_dir()
    {
        std::string dir = "data/selfplay/";
        std::filesystem::create_directories(dir);
        return dir;
    }

    void SelfPlay::save_game_json(const std::vector<PositionData>& game_data, double white_result, int game_id)
    {
        std::string filename = get_output_dir() + "game_" + std::to_string(game_id) + ".jsonl";
        std::ofstream out(filename);

        for (const auto& pos : game_data)
        {
            double result = (pos.color_to_move == 0) ? white_result : (1.0 - white_result);
            out << "{\"fen\":\"" << pos.fen << "\",\"result\":" << result << ",\"policy\":{";
            
            bool first = true;
            for (const auto& p : pos.policy)
            {
                if (!first) out << ",";
                out << "\"" << p.first << "\":" << p.second;
                first = false;
            }
            out << "}}\n";
        }
    }

    void SelfPlay::run()
    {
        std::cout << "Starting Self-Play Pipeline...\n";
        std::cout << "Simulations per move: " << simulations << "\n";
        std::cout << "Games: " << max_games << "\n";
        std::cout << "Output Directory: " << get_output_dir() << "\n\n";

        auto evaluator = std::make_unique<NNEvaluator>("onnx/fenrir.onnx");
        auto search = std::make_unique<MCTSSearch>(evaluator.get(), 16);

        std::mt19937 rng(std::random_device{}());

        for (int i = 1; i <= max_games; ++i)
        {
            Engine engine(start_fen);
            std::vector<PositionData> game_data;
            int moves_played = 0;
            
            std::cout << "Game " << i << " started...\n";

            while (!engine.is_checkmate() && !engine.is_stalemate() && !engine.is_draw() && moves_played < 200)
            {
                bool apply_noise = (moves_played < 30); 
                
                auto [best_move, raw_policy] = search->find_best_move_with_policy(engine, simulations, apply_noise);
                
                if (raw_policy.empty()) break;

                PositionData pd;
                pd.fen = engine.get_fen();
                pd.color_to_move = engine.get_board_view().get_color();
                for (const auto& p : raw_policy) {
                    pd.policy.push_back({p.first.to_uci_notation(), p.second});
                }
                game_data.push_back(pd);

                Move chosen_move = best_move;
                if (moves_played < 30) {
                    std::vector<double> weights;
                    for (const auto& p : raw_policy) weights.push_back(p.second);
                    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
                    chosen_move = raw_policy[dist(rng)].first; 
                }

                engine.make_move(chosen_move);
                moves_played++;
            }

            double white_result = 0.5;
            if (engine.is_checkmate())
            {
                if (engine.get_board_view().get_color() == 0)
                {
                    white_result = 0.0;
                }
                else
                {
                    white_result = 1.0;
                }
            }

            std::cout << "Game " << i << " finished in " << moves_played << " moves. Result: " << white_result << "\n";
            auto now = std::chrono::system_clock::now().time_since_epoch();
            int unique_id = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count() % 100000);
            save_game_json(game_data, white_result, unique_id);
        }
    }
}
