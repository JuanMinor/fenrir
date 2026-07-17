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
#include "include/nn/nn.h"
#include <vector>
#include <memory>
#include <cmath>
#include <random>
#include <mutex>
#include <atomic>
#include <thread>
#include <future>

namespace mcts
{
    const double PUCT_C_INIT = 1.25;
    const double PUCT_C_BASE = 19652.0;

    const double DIRICHLET_EPSILON = 0.25;
    const double DIRICHLET_ALPHA = 0.3;

    const int MAX_ROLLOUT_MOVES = 100;

    const double PIECE_VALUE_PAWN = 100.0;
    const double PIECE_VALUE_KNIGHT = 320.0;
    const double PIECE_VALUE_BISHOP = 330.0;
    const double PIECE_VALUE_ROOK = 500.0;
    const double PIECE_VALUE_QUEEN = 900.0;
    const double PIECE_VALUE_KING = 20000.0;

    class Node
    {
    public:
        Node(Node *parent_node, const chess::Move &m, uint8_t color);
        ~Node();

        Node *parent;
        std::vector<std::unique_ptr<Node>> children;
        chess::Move move;
        uint8_t color_to_move;

        std::atomic<int> visits;
        std::atomic<double> win_score;
        std::atomic<int> virtual_loss;

        double prior;
        std::atomic<bool> is_expanded;
        std::mutex expand_mutex;

        void add_dirichlet_noise(double epsilon, double alpha);
        void backpropagate(double result);
        void expand(chess::Engine &engine, const std::vector<double> &policy = {});
        int get_max_depth() const;
        double puct_value(int total_visits) const;
        Node *select_child();
    };

    /**
     * RAII helper that increments a node's virtual loss on construction and
     * decrements it on destruction, guaranteeing balance even if an
     * exception unwinds the stack mid-search.
     */
    struct VirtualLossGuard
    {
        Node *node;
        VirtualLossGuard(Node *n) : node(n) { node->virtual_loss++; }
        ~VirtualLossGuard() { node->virtual_loss--; }
    };

    /**
     * One in-flight simulation slot in a pipelined search batch: either a
     * resolved terminal result, or a pending NN evaluation together with the
     * move path needed to cheaply re-reach its leaf for expansion.
     */
    struct PipelineItem
    {
        Node *node;
        std::vector<VirtualLossGuard> path_guards;
        std::future<nn::Result> eval_future;
        bool is_terminal;
        double terminal_result;
        std::vector<chess::Move> path_moves;
    };

    class Tree
    {
    public:
        Tree(nn::NN *nn, int threads, size_t pipeline_target);
        ~Tree();

        int benchmark_search(chess::Engine &engine, int time_limit_ms);
        chess::Move find_best_move(chess::Engine &engine, int time_limit_ms, int max_simulations);
        std::pair<chess::Move, std::vector<std::pair<chess::Move, double>>> find_best_move_with_policy(chess::Engine &engine, int simulations, bool apply_noise = false);

    private:
        void search_worker(std::unique_ptr<chess::Engine> thread_engine, Node *root, int simulations, std::chrono::steady_clock::time_point end_time, bool use_time);
        double simulate(chess::Engine &engine);

        nn::NN *evaluator;
        int num_threads;
        size_t pipeline_target;
    };
}
