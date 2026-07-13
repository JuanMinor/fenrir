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

#include "include/mcts/mcts.h"
#include <algorithm>
#include <iostream>
#include <random>

namespace mcts
{
    namespace
    {
        uint32_t move_index(const chess::Move &m)
        {
            int from_sq = static_cast<int>(m.get_from_square());
            int to_sq = static_cast<int>(m.get_to_square());

            int from_file = from_sq % 8;
            int from_rank = from_sq / 8;
            int to_file = to_sq % 8;
            int to_rank = to_sq / 8;

            int dx = to_file - from_file;
            int dy = to_rank - from_rank;

            int channel = 0;
            char promo = static_cast<char>(std::tolower(static_cast<unsigned char>(m.get_promotion_piece())));

            if (m.is_promotion() && promo != 'q' && promo != '\0')
            {
                int promo_dir = dx + 1;
                int promo_type = 0;
                if (promo == 'b')
                    promo_type = 1;
                else if (promo == 'r')
                    promo_type = 2;

                channel = 64 + (promo_dir * 3) + promo_type;
            }
            else if (std::abs(dx) * std::abs(dy) == 2)
            {
                std::pair<int, int> knight_lookups[8] = {
                    {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
                for (int i = 0; i < 8; ++i)
                {
                    if (dx == knight_lookups[i].first && dy == knight_lookups[i].second)
                    {
                        channel = 56 + i;
                        break;
                    }
                }
            }
            else
            {
                int step_x = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
                int step_y = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);
                int distance = std::max(std::abs(dx), std::abs(dy));

                int dir_idx = 0;
                if (step_x == 0 && step_y == 1)
                    dir_idx = 0;
                else if (step_x == 1 && step_y == 1)
                    dir_idx = 1;
                else if (step_x == 1 && step_y == 0)
                    dir_idx = 2;
                else if (step_x == 1 && step_y == -1)
                    dir_idx = 3;
                else if (step_x == 0 && step_y == -1)
                    dir_idx = 4;
                else if (step_x == -1 && step_y == -1)
                    dir_idx = 5;
                else if (step_x == -1 && step_y == 0)
                    dir_idx = 6;
                else if (step_x == -1 && step_y == 1)
                    dir_idx = 7;

                channel = (dir_idx * 7) + (distance - 1);
            }

            return static_cast<uint32_t>((from_sq * 73) + channel);
        }

        /**
         * RAII helper that increments a node's virtual loss on construction and
         * decrements it on destruction, guaranteeing balance even if an
         * exception unwinds the stack mid-search.
         */
        struct VirtualLossGuard
        {
            MCTSNode *node;
            VirtualLossGuard(MCTSNode *n) : node(n) { node->virtual_loss++; }
            ~VirtualLossGuard() { node->virtual_loss--; }
        };
    }

    MCTSNode::MCTSNode(MCTSNode *parent_node, const chess::Move &m, uint8_t color)
        : parent(parent_node), move(m), color_to_move(color),
          visits(0), win_score(0.0), virtual_loss(0), prior(0.0), is_expanded(false)
    {
    }

    MCTSNode::~MCTSNode() = default;

    void MCTSNode::add_dirichlet_noise(double epsilon, double alpha)
    {
        if (children.empty())
            return;

        std::mt19937 rng(std::random_device{}());
        std::gamma_distribution<double> gamma(alpha, 1.0);

        double original_prior_sum = 0.0;
        for (const auto &child : children)
        {
            original_prior_sum += child->prior;
        }
        if (original_prior_sum <= 0.0)
            original_prior_sum = 1.0;

        double noise_sum = 0.0;
        std::vector<double> noise;
        noise.reserve(children.size());
        for (size_t i = 0; i < children.size(); ++i)
        {
            double n = gamma(rng);
            noise.push_back(n);
            noise_sum += n;
        }
        if (noise_sum <= 0.0)
            noise_sum = 1.0;

        for (size_t i = 0; i < children.size(); ++i)
        {
            double normalized_original = children[i]->prior / original_prior_sum;
            double normalized_noise = noise[i] / noise_sum;
            children[i]->prior = (1.0 - epsilon) * normalized_original + epsilon * normalized_noise;
        }
    }

    void MCTSNode::backpropagate(double result)
    {
        visits++;
        double current = win_score.load();
        while (!win_score.compare_exchange_weak(current, current + result))
            ;
        if (parent)
            parent->backpropagate(1.0 - result);
    }

    /**
     * Expands this node with one child per legal move. When @p policy is
     * supplied (from the NN), each child's prior is taken directly from the
     * policy vector indexed by move_index(). When no policy is given, priors
     * fall back to an MVV-LVA (Most Valuable Victim, Least Valuable
     * Aggressor) heuristic so the tree explores tactical lines before quiet
     * ones: score = victim_value - aggressor_value / 10 (piece values in
     * centipawns, P=100 N=320 B=330 R=500 Q=900 K=20000), clamped to a floor
     * of 1.0. Heuristic priors are then normalized to sum to 1.0.
     */
    void MCTSNode::expand(chess::Engine &engine, const std::vector<double> &policy)
    {
        std::lock_guard<std::mutex> lock(expand_mutex);
        if (is_expanded.load())
            return;

        std::vector<chess::Move> moves = engine.generate_all_moves();
        children.reserve(moves.size());

        for (size_t i = 0; i < moves.size(); ++i)
        {
            uint8_t next_color = 1 - color_to_move;
            children.push_back(std::make_unique<MCTSNode>(this, moves[i], next_color));

            if (policy.empty())
            {
                auto piece_value = [](char pc) -> double
                {
                    switch (std::tolower(static_cast<unsigned char>(pc)))
                    {
                    case 'p':
                        return 100.0;
                    case 'n':
                        return 320.0;
                    case 'b':
                        return 330.0;
                    case 'r':
                        return 500.0;
                    case 'q':
                        return 900.0;
                    case 'k':
                        return 20000.0;
                    default:
                        return 0.0;
                    }
                };

                double base = 1.0;
                const chess::MoveType mt = moves[i].get_move_type();

                if (mt == chess::MoveType::CAPTURE)
                {
                    uint8_t to_sq = moves[i].get_to_square();
                    uint8_t from_sq = moves[i].get_from_square();
                    char victim = engine.get_board_view().get_piece(to_sq / 8, to_sq % 8);
                    char attacker = engine.get_board_view().get_piece(from_sq / 8, from_sq % 8);
                    base = 1.0 + piece_value(victim) - piece_value(attacker) / 10.0;
                    if (base < 1.0)
                        base = 1.0;
                }
                else if (mt == chess::MoveType::EN_PASSANT)
                {
                    base = 1.0 + 100.0 - 100.0 / 10.0;
                }
                else if (mt == chess::MoveType::PROMOTION)
                {
                    base = 1.0 + piece_value(moves[i].get_promotion_piece()) / 100.0;
                }

                children.back()->prior = base;
            }
            else
            {
                uint32_t idx = move_index(moves[i]);
                children.back()->prior = (idx < policy.size()) ? policy[idx] : 0.0;
            }
        }

        if (policy.empty() && !children.empty())
        {
            double sum = 0.0;
            for (const auto &child : children)
                sum += child->prior;
            if (sum > 0.0)
                for (auto &child : children)
                    child->prior /= sum;
        }

        is_expanded.store(true);
    }

    /**
     * Returns the deepest explored line below this node. A child counts as
     * explored if it has real visits or a nonzero virtual loss, so in-flight
     * (currently evaluating) branches aren't underreported.
     */
    int MCTSNode::get_max_depth() const
    {
        if (!is_expanded.load() || children.empty())
            return 0;
        int max_d = 0;
        for (const auto &child : children)
        {
            if (child->visits.load() > 0 || child->virtual_loss.load() > 0)
            {
                max_d = std::max(max_d, child->get_max_depth());
            }
        }
        return 1 + max_d;
    }

    /**
     * Computes the PUCT (predictor + UCT) selection score for this child, per
     * the AlphaZero formula Q + U with a dynamic CPUCT constant.
     *
     * The visit count feeding the U-term denominator, v, depends on node
     * state: for a node with real visits, v = real_visits + virtual_loss
     * (virtual loss is normally 0 on interior nodes and only nonzero for a
     * leaf currently being evaluated by another walk in the same pipeline);
     * for an in-flight leaf with no backpropagated result yet, v = virtual_loss
     * alone, so U shrinks each time another walk lands here, naturally
     * diverting concurrent walks to sibling nodes; for an untouched leaf,
     * v = 0 and U is maximised, the standard MCTS exploration behaviour.
     *
     * Q is computed only from real backpropagated results, never diluted by
     * virtual loss; since a child's win_score is the child's (opponent's) win
     * rate, the parent's win rate is 1.0 - q.
     */
    double MCTSNode::puct_value(int total_visits) const
    {
        const int real_visits = visits.load();
        const int vl = virtual_loss.load();

        int v = (real_visits > 0) ? (real_visits + vl) : vl;

        double q = 0.0;
        if (real_visits > 0)
        {
            q = 1.0 - (win_score.load() / real_visits);
        }

        double c_init = 1.25;
        double c_base = 19652.0;
        double c = std::log((1.0 + total_visits + c_base) / c_base) + c_init;

        double u = c * prior * std::sqrt(static_cast<double>(total_visits)) / (1 + v);

        return q + u;
    }

    MCTSNode *MCTSNode::select_child()
    {
        MCTSNode *best_child = nullptr;
        double best_value = -1e9;
        int parent_visits = visits.load();

        for (auto &child : children)
        {
            double puct = child->puct_value(parent_visits);
            if (puct > best_value)
            {
                best_value = puct;
                best_child = child.get();
            }
        }
        return best_child;
    }

    MCTSSearch::MCTSSearch(nn::NNEvaluator *eval, int threads, size_t pipeline_t)
        : evaluator(eval), num_threads(threads), pipeline_target(pipeline_t) {}

    MCTSSearch::~MCTSSearch() = default;

    int MCTSSearch::benchmark_search(chess::Engine &engine, int time_limit_ms)
    {
        uint8_t root_color = engine.get_board_view().get_color();
        chess::Move empty_move(0, 0);
        auto root = std::make_unique<MCTSNode>(nullptr, empty_move, root_color);

        if (evaluator)
        {
            try
            {
                auto future = evaluator->request_evaluation(engine.get_board_view());
                nn::NNResult res = future.get();
                root->expand(engine, res.policy);
            }
            catch (...)
            {
                root->expand(engine);
            }
        }
        else
        {
            root->expand(engine);
        }

        std::vector<std::thread> workers;
        auto end_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(std::max(1, time_limit_ms));

        for (int i = 0; i < num_threads; ++i)
        {
            try
            {
                workers.emplace_back(&MCTSSearch::search_worker, this, std::make_unique<chess::Engine>(engine.get_fen()), root.get(), -1, end_time, true);
            }
            catch (...)
            {
                break;
            }
        }

        if (workers.empty())
        {
            search_worker(std::make_unique<chess::Engine>(engine.get_fen()), root.get(), -1, end_time, true);
        }
        else
        {
            for (auto &w : workers)
            {
                if (w.joinable())
                    w.join();
            }
        }

        return root->visits.load();
    }

    /**
     * Runs a multithreaded MCTS search from the given position for either a
     * fixed time budget or a fixed simulation count, and returns the move
     * with the most visits at the root. The reported score is inverted from
     * the best child's win_score, since that value represents the opponent's
     * win rate.
     */
    chess::Move MCTSSearch::find_best_move(chess::Engine &engine, int time_limit_ms, int max_simulations)
    {
        uint8_t root_color = engine.get_board_view().get_color();
        chess::Move empty_move(0, 0);
        auto root = std::make_unique<MCTSNode>(nullptr, empty_move, root_color);

        if (evaluator)
        {
            auto future = evaluator->request_evaluation(engine.get_board_view());
            nn::NNResult res = future.get();
            root->expand(engine, res.policy);
        }
        else
        {
            root->expand(engine);
        }

        if (root->children.empty())
            return empty_move;

        std::vector<std::thread> workers;
        int sims_per_thread = -1;
        bool use_time = (time_limit_ms > 0);

        if (!use_time)
        {
            sims_per_thread = num_threads > 0 ? max_simulations / num_threads : max_simulations;
            if (sims_per_thread <= 0)
                sims_per_thread = 1;
        }

        auto end_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(std::max(1, time_limit_ms));

        for (int i = 0; i < num_threads; ++i)
        {
            workers.emplace_back(&MCTSSearch::search_worker, this, std::make_unique<chess::Engine>(engine.get_fen()), root.get(), sims_per_thread, end_time, use_time);
        }

        for (auto &w : workers)
        {
            w.join();
        }

        chess::Move best_move(0, 0);
        int max_visits = -1;
        double best_score = 0.5;
        for (auto &child : root->children)
        {
            if (child->visits.load() > max_visits)
            {
                max_visits = child->visits.load();
                best_move = child->move;
                best_score = 1.0 - (child->win_score.load() / std::max(1, child->visits.load()));
            }
        }

        int total_nodes = root->visits.load();
        int max_depth = root->get_max_depth();
        int score_cp = static_cast<int>((best_score - 0.5) * 200);
        std::cout << "info depth " << max_depth << " nodes " << total_nodes << " score cp " << score_cp << " pv " << best_move.to_uci_notation() << std::endl;

        return best_move;
    }

    std::pair<chess::Move, std::vector<std::pair<chess::Move, double>>> MCTSSearch::find_best_move_with_policy(chess::Engine &engine, int simulations, bool apply_noise)
    {
        uint8_t root_color = engine.get_board_view().get_color();
        chess::Move empty_move(0, 0);
        auto root = std::make_unique<MCTSNode>(nullptr, empty_move, root_color);

        if (evaluator)
        {
            try
            {
                auto future = evaluator->request_evaluation(engine.get_board_view());
                nn::NNResult res = future.get();
                root->expand(engine, res.policy);
            }
            catch (const std::exception &e)
            {
                (void)e;
                std::cerr << "Root evaluation failed: " << e.what() << "\n";
                root->expand(engine);
            }
        }
        else
        {
            root->expand(engine);
        }

        if (apply_noise)
        {
            root->add_dirichlet_noise(0.25, 0.3);
        }

        if (root->children.empty())
            return {empty_move, {}};

        std::vector<std::thread> workers;
        int sims_per_thread = num_threads > 0 ? simulations / num_threads : simulations;
        if (sims_per_thread == 0)
            sims_per_thread = 1;

        for (int i = 0; i < num_threads; ++i)
        {
            try
            {
                workers.emplace_back(&MCTSSearch::search_worker, this, std::make_unique<chess::Engine>(engine.get_fen()), root.get(), sims_per_thread, std::chrono::steady_clock::now(), false);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Thread creation failed: " << e.what() << "\n";
                break;
            }
        }

        if (workers.empty())
        {
            std::cerr << "Warning: Falling back to synchronous search!\n";
            search_worker(std::make_unique<chess::Engine>(engine.get_fen()), root.get(), simulations, std::chrono::steady_clock::now(), false);
        }

        for (auto &w : workers)
        {
            if (w.joinable())
                w.join();
        }

        chess::Move best_move(0, 0);
        int max_visits = -1;
        std::vector<std::pair<chess::Move, double>> policy;
        double total_visits = root->visits.load();

        for (auto &child : root->children)
        {
            int v = child->visits.load();
            policy.push_back({child->move, static_cast<double>(v) / std::max(1.0, total_visits)});
            if (v > max_visits)
            {
                max_visits = v;
                best_move = child->move;
            }
        }
        return {best_move, policy};
    }

    /**
     * Runs simulations on a dedicated engine/thread until the time budget or
     * simulation count is exhausted, using two-phase pipelined batches of
     * size pipeline_target to keep the GPU evaluator saturated without the
     * tree exploding sideways every cycle.
     *
     * Phase 1 (selection): for each pipeline slot, walk from the root via
     * PUCT to a leaf, recording the move path taken so the leaf position can
     * be cheaply re-reached later without reconstructing an Engine (which
     * would re-parse FEN, rebuild attack tables, and log). Virtual loss is
     * applied only to the leaf, never to interior nodes on the path — marking
     * every node en route would make PUCT treat the whole line as "in use"
     * and force purely horizontal tree growth; leaving interior nodes clean
     * lets subsequent walks keep following the same best line to different
     * leaves. Each leaf's terminal status is resolved with a single
     * get_terminal_state() call (checks the 50-move rule and repetition
     * first, then calls generate_all_moves() at most once), replacing the
     * old is_stalemate() + is_draw() pattern that generated moves twice per
     * position. Non-terminal leaves kick off an async NN evaluation request
     * when an evaluator is configured; otherwise, since thread_engine is
     * still sitting at the leaf at this point, the leaf is expanded and
     * rolled out immediately via simulate().
     *
     * Phase 2 (backpropagation): for each pipeline item, await its NN result
     * (re-walking thread_engine along the recorded path to reach the leaf
     * again, cheaply, via make_move_fast), expand the node with the
     * resulting policy, and backpropagate the value up the tree.
     */
    void MCTSSearch::search_worker(std::unique_ptr<chess::Engine> thread_engine_ptr, MCTSNode *root, int simulations, std::chrono::steady_clock::time_point end_time, bool use_time)
    {
        chess::Engine &thread_engine = *thread_engine_ptr;
        int sim_count = 0;

        const size_t local_pipeline_target = pipeline_target;

        struct PipelineItem
        {
            MCTSNode *node;
            std::vector<VirtualLossGuard> path_guards;
            std::future<nn::NNResult> eval_future;
            bool is_terminal;
            double terminal_result;
            std::vector<chess::Move> path_moves;
        };

        while (true)
        {
            if (use_time)
            {
                if ((sim_count & 15) == 0 && std::chrono::steady_clock::now() >= end_time)
                    break;
            }
            else
            {
                if (sim_count >= simulations)
                    break;
            }

            std::vector<PipelineItem> pipeline;
            pipeline.reserve(local_pipeline_target);

            for (size_t p = 0; p < local_pipeline_target; ++p)
            {
                sim_count++;
                MCTSNode *node = root;
                std::vector<VirtualLossGuard> guards;
                std::vector<chess::Move> path;

                while (node->is_expanded.load() && !node->children.empty())
                {
                    MCTSNode *next_node = node->select_child();

                    if (!next_node)
                        break;

                    node = next_node;
                    path.push_back(node->move);
                    thread_engine.make_move_fast(node->move);
                }
                guards.emplace_back(node);

                PipelineItem item;
                item.node = node;
                item.path_guards = std::move(guards);
                item.is_terminal = false;
                item.terminal_result = 0.0;

                auto ts = thread_engine.get_terminal_state();
                if (ts.is_terminal)
                {
                    item.is_terminal = true;
                    item.terminal_result = ts.score;
                }
                else if (evaluator)
                {
                    item.path_moves = path;
                    item.eval_future = evaluator->request_evaluation(thread_engine.get_board_view());
                }
                else
                {
                    item.is_terminal = true;
                    node->expand(thread_engine);
                    item.terminal_result = simulate(thread_engine);
                }

                pipeline.push_back(std::move(item));

                MCTSNode *curr = node;
                while (curr != root)
                {
                    thread_engine.undo_move();
                    curr = curr->parent;
                }
            }

            for (auto &item : pipeline)
            {
                double final_result = 0.0;

                if (item.is_terminal)
                {
                    final_result = item.terminal_result;
                }
                else
                {
                    try
                    {
                        nn::NNResult res = item.eval_future.get();
                        for (const auto &m : item.path_moves)
                            thread_engine.make_move_fast(m);
                        item.node->expand(thread_engine, res.policy);
                        for (size_t i = 0; i < item.path_moves.size(); ++i)
                            thread_engine.undo_move();
                        final_result = res.value;
                    }
                    catch (...)
                    {
                        for (const auto &m : item.path_moves)
                            thread_engine.make_move_fast(m);
                        item.node->expand(thread_engine);
                        final_result = simulate(thread_engine);
                        for (size_t i = 0; i < item.path_moves.size(); ++i)
                            thread_engine.undo_move();
                    }
                }

                item.path_guards.clear();
                item.node->backpropagate(final_result);
            }
        }
    }

    double MCTSSearch::simulate(chess::Engine &engine)
    {
        int moves_played = 0;
        uint8_t start_color = engine.get_board_view().get_color();

        std::mt19937 local_rng(std::random_device{}());

        while (!engine.is_checkmate() && !engine.is_stalemate() && !engine.is_draw() && moves_played < 100)
        {
            std::vector<chess::Move> legal_moves = engine.generate_all_moves();
            if (legal_moves.empty())
                break;

            std::uniform_int_distribution<size_t> dist(0, legal_moves.size() - 1);
            engine.make_move_fast(legal_moves[dist(local_rng)]);
            moves_played++;
        }

        double result = 0.5;
        if (engine.is_checkmate())
        {
            result = (engine.get_board_view().get_color() == start_color) ? 0.0 : 1.0;
        }

        for (int i = 0; i < moves_played; ++i)
        {
            engine.undo_move();
        }

        return result;
    }
}
