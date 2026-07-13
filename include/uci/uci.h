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
#include <memory>

namespace chess
{
    class UCI
    {
        void parse_position(const std::string &command);
        void parse_go(const std::string &command);

        std::unique_ptr<chess::Engine> engine;
        std::unique_ptr<nn::NNEvaluator> evaluator;
        std::unique_ptr<mcts::MCTSSearch> search;

    public:
        UCI();
        ~UCI();

        void loop();
    };
}
