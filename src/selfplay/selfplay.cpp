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

#include "include/selfplay/selfplay.h"
#include "include/tuner/tuning_parameters.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>
#include <cstdlib>
#include <bit>
#include <chrono>

#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

namespace chess
{
    SelfPlay::SelfPlay(int gpu_id, int simulations_per_move, int num_games, const std::string &start_fen_arg)
        : gpu_id_(gpu_id), simulations(simulations_per_move), max_games(num_games), start_fen(start_fen_arg)
    {
    }

    /**
     * Runs the self-play pipeline: for each game, repeatedly searches the
     * current position, filters the raw MCTS policy down to strictly legal
     * moves (falling back to a uniform distribution over legal moves if
     * filtering empties it due to a noise mismatch) and renormalizes it,
     * records the position/policy pair, then samples the next move from the
     * legal policy (falling back to sampling if best_move comes back
     * uninitialized). Output batches are written to a temp JSONL file named
     * with the GPU id, this process's PID, and a timestamp, so multiple
     * concurrent instances never collide.
     */
    void SelfPlay::run()
    {
        std::cout << "Starting Self-Play Pipeline on GPU " << gpu_id_ << "...\n";
        std::cout << "Simulations per move: " << simulations << "\n";
        std::cout << "Games: " << max_games << "\n";
        std::cout << "Output Directory: " << get_output_dir() << "\n\n";

        tuner::TuningParameters tuning(true);
        auto evaluator = std::make_unique<nn::NN>("onnx/fenrir.onnx", gpu_id_, tuning.get_batch_size(), tuning.get_batch_timeout_ms());
        auto search = std::make_unique<mcts::Tree>(evaluator.get(), tuning.get_search_threads(), tuning.get_pipeline_target());

        std::mt19937 rng(std::random_device{}());

        /* How many plies to keep temperature sampling and Dirichlet noise
         * active. AlphaZero's classic 30 assumes a competent net; during a
         * cold start, switching to deterministic play with a value head
         * that rates everything ~0.5 walks straight into repetition draws
         * (observed: 0 decisive games in 800+ once training began), which
         * starves value learning. Default to sampling the whole game;
         * lower FENRIR_TEMPERATURE_MOVES as the net matures. */
        int temperature_moves = 100000;
        if (const char *env_val = std::getenv("FENRIR_TEMPERATURE_MOVES"))
        {
            int parsed = std::atoi(env_val);
            if (parsed >= 0)
                temperature_moves = parsed;
        }
        std::cout << "Temperature sampling plies: " << temperature_moves << "\n";

        /* Cold-start curriculum: games that end without checkmate (200-ply
         * cap, repetition, 50-move) but with a lopsided material count are
         * scored as wins for the side that is ahead, instead of 0.5. A
         * from-scratch net produces ~85% marathon "draws" that are really
         * unconverted wins; labeling them 0.5 starves the value head and
         * the whole system settles into a uniform fixed point (observed:
         * policy loss pinned at the target-entropy floor, hour-1 vs hour-4
         * checkpoints drawing 23/23 arena games). Material adjudication
         * multiplies the value signal and teaches material worth first.
         * Set FENRIR_ADJUDICATE_MATERIAL=0 to disable once the net converts
         * wins on its own; default threshold is 5 (a rook or more). */
        int adjudicate_material = 5;
        if (const char *env_val = std::getenv("FENRIR_ADJUDICATE_MATERIAL"))
        {
            int parsed = std::atoi(env_val);
            if (parsed >= 0)
                adjudicate_material = parsed;
        }
        std::cout << "Material adjudication threshold: " << adjudicate_material << (adjudicate_material == 0 ? " (off)" : "") << "\n";

        int games_in_batch = 0;
        std::string temp_filename;
        std::ofstream batch_out;

        for (int i = 1; i <= max_games; ++i)
        {
            if (games_in_batch == 0)
            {
                auto now = std::chrono::system_clock::now().time_since_epoch();
                int timestamp = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count() % 100000000);

                int pid = static_cast<int>(getpid());

                temp_filename = get_output_dir() + "temp_gpu" + std::to_string(gpu_id_) + "_pid" + std::to_string(pid) + "_" + std::to_string(timestamp) + ".jsonl";
                batch_out.open(temp_filename);
            }

            Engine engine(start_fen);
            std::vector<PositionData> game_data;
            int moves_played = 0;

            std::cout << "Game " << i << " started...\n";

            while (moves_played < 200)
            {
                /* One combined terminal query instead of is_checkmate() +
                 * is_stalemate() + is_draw(), which each regenerate all
                 * legal moves for the position. */
                if (engine.get_terminal_state().is_terminal)
                    break;

                bool apply_noise = (moves_played < temperature_moves);

                auto [best_move, raw_policy] = search->find_best_move_with_policy(engine, simulations, apply_noise);

                if (raw_policy.empty())
                    break;

                std::vector<Move> legal_moves = engine.generate_all_moves();
                if (legal_moves.empty())
                    break;

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

                if (legal_policy.empty())
                {
                    for (const auto &lm : legal_moves)
                    {
                        legal_policy.push_back({lm, 1.0 / static_cast<double>(legal_moves.size())});
                    }
                    legal_sum = 1.0;
                }

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

                Move chosen_move = best_move;
                if (chosen_move.get_from_square() == chosen_move.get_to_square() || moves_played < temperature_moves)
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
            else if (adjudicate_material > 0)
            {
                /* Bitboards 0-4 are white P,N,B,R,Q; 6-10 black. Kings excluded. */
                static constexpr int piece_values[5] = {1, 3, 3, 5, 9};
                const AbstractBoard &board = engine.get_board_view();
                int material_balance = 0;
                for (int piece = 0; piece < 5; ++piece)
                {
                    material_balance += piece_values[piece] * std::popcount(board.get_bitboard(piece));
                    material_balance -= piece_values[piece] * std::popcount(board.get_bitboard(piece + 6));
                }

                if (material_balance >= adjudicate_material)
                {
                    white_result = 1.0;
                }
                else if (material_balance <= -adjudicate_material)
                {
                    white_result = 0.0;
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

    std::string SelfPlay::get_output_dir()
    {
        std::string dir = "data/selfplay/";
        std::filesystem::create_directories(dir);
        return dir;
    }
}
