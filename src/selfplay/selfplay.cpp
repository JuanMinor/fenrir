#include "include/selfplay/selfplay.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>
#include <chrono>

#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

namespace fenrir
{
    SelfPlay::SelfPlay(int gpu_id, int simulations_per_move, int num_games, const std::string &start_fen_arg)
        : gpu_id_(gpu_id), simulations(simulations_per_move), max_games(num_games), start_fen(start_fen_arg)
    {
    }

    std::string SelfPlay::get_output_dir()
    {
        std::string dir = "data/selfplay/";
        std::filesystem::create_directories(dir);
        return dir;
    }

    void SelfPlay::run()
    {
        std::cout << "Starting Self-Play Pipeline on GPU " << gpu_id_ << "...\n";
        std::cout << "Simulations per move: " << simulations << "\n";
        std::cout << "Games: " << max_games << "\n";
        std::cout << "Output Directory: " << get_output_dir() << "\n\n";

        auto evaluator = std::make_unique<NNEvaluator>("onnx/fenrir.onnx", gpu_id_);
        auto search = std::make_unique<MCTSSearch>(evaluator.get(), 16);

        std::mt19937 rng(std::random_device{}());

        int games_in_batch = 0;
        std::string temp_filename;
        std::ofstream batch_out;

        for (int i = 1; i <= max_games; ++i)
        {
            if (games_in_batch == 0)
            {
                auto now = std::chrono::system_clock::now().time_since_epoch();
                int timestamp = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count() % 100000000);

                // Get the unique process ID of this specific running engine instance
                int pid = static_cast<int>(getpid());

                // FIXED: Appended the unique PID into the filename string
                temp_filename = get_output_dir() + "temp_gpu" + std::to_string(gpu_id_) + "_pid" + std::to_string(pid) + "_" + std::to_string(timestamp) + ".jsonl";
                batch_out.open(temp_filename);
            }

            Engine engine(start_fen);
            std::vector<PositionData> game_data;
            int moves_played = 0;

            std::cout << "Game " << i << " started...\n";

            while (!engine.is_checkmate() && !engine.is_stalemate() && !engine.is_draw() && moves_played < 200)
            {
                bool apply_noise = (moves_played < 30);

                auto [best_move, raw_policy] = search->find_best_move_with_policy(engine, simulations, apply_noise);

                if (raw_policy.empty())
                    break;

                // Create a fast lookup set of absolute legal moves for the current board state
                std::vector<Move> legal_moves = engine.generate_all_moves();
                if (legal_moves.empty())
                    break;

                // Filter raw_policy to ensure only strictly legal choices can be recorded or played
                std::vector<std::pair<Move, double>> legal_policy;
                double legal_sum = 0.0;

                for (const auto &p : raw_policy)
                {
                    bool is_legal = false;
                    for (const auto &lm : legal_moves)
                    {
                        if (p.first.get_from_square() == lm.get_from_square() &&
                            p.first.get_to_square() == lm.get_to_square())
                        {
                            is_legal = true;
                            break;
                        }
                    }
                    if (is_legal)
                    {
                        legal_policy.push_back(p);
                        legal_sum += p.second;
                    }
                }

                // If filtering wiped the vector due to noise mismatch, fallback safely to strict legal generation
                if (legal_policy.empty())
                {
                    for (const auto &lm : legal_moves)
                    {
                        legal_policy.push_back({lm, 1.0 / static_cast<double>(legal_moves.size())});
                    }
                    legal_sum = 1.0;
                }

                // Renormalize the legal policy distribution
                for (auto &p : legal_policy)
                {
                    p.second /= (legal_sum > 0.0 ? legal_sum : 1.0);
                }

                PositionData pd;
                pd.fen = engine.get_fen();
                pd.color_to_move = engine.get_board_view().get_color();
                for (const auto &p : legal_policy)
                {
                    pd.policy.emplace_back(p.first.to_uci_notation(), p.second);
                }
                game_data.push_back(pd);

                // Fallback safely to a known legal choice if best_move returns uninitialized or blank data
                Move chosen_move = best_move;
                if (chosen_move.get_from_square() == chosen_move.get_to_square() || moves_played < 30)
                {
                    std::vector<double> weights;
                    for (const auto &p : legal_policy)
                    {
                        weights.push_back(p.second);
                    }
                    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
                    chosen_move = legal_policy[dist(rng)].first;
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

            std::cout << "GPU " << gpu_id_ << " Game " << i << " finished in " << moves_played << " moves. Result: " << white_result << "\n";

            for (const auto &pos : game_data)
            {
                double res = (pos.color_to_move == 0) ? white_result : (1.0 - white_result);
                batch_out << "{\"fen\":\"" << pos.fen << "\",\"result\":" << res << ",\"policy\":{";
                bool first = true;
                for (const auto &p : pos.policy)
                {
                    if (!first)
                        batch_out << ",";
                    batch_out << "\"" << p.first << "\":" << p.second;
                    first = false;
                }
                batch_out << "}}\n";
            }

            games_in_batch++;
            if (games_in_batch >= 5 || i == max_games)
            {
                batch_out.close();
                std::string ready_filename = temp_filename;
                size_t pos = ready_filename.find("temp_");
                if (pos != std::string::npos)
                    ready_filename.replace(pos, 5, "READY_");
                std::filesystem::rename(temp_filename, ready_filename);
                games_in_batch = 0;
            }
        }
    }
}
