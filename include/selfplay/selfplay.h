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

#pragma once
#include "include/engine/engine.h"
#include "include/mcts/mcts.h"
#include "include/nn/nn.h"
#include <string>
#include <vector>
#include <utility>

namespace chess
{
    class SelfPlay
    {
    public:
        SelfPlay(int gpu_id, int simulations_per_move, int num_games, const std::string& start_fen_arg = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        void run();

    private:
        struct PositionData {
            std::string fen;
            std::vector<std::pair<std::string, double>> policy;
            uint8_t color_to_move;
        };

        void apply_dirichlet_noise(mcts::MCTSNode* root);
        std::string get_output_dir();

        int gpu_id_;
        int simulations;
        int max_games;
        std::string start_fen;
    };
}
