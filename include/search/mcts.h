#pragma once

#include "include/engine/engine.h"
#include "include/eval/nn_eval.h"
#include <vector>
#include <memory>
#include <cmath>
#include <random>
#include <mutex>
#include <atomic>
#include <thread>

namespace fenrir
{
    class MCTSNode
    {
    public:
        MCTSNode(MCTSNode* parent_node, const Move& m, uint8_t color);
        ~MCTSNode();

        MCTSNode* parent;
        std::vector<std::unique_ptr<MCTSNode>> children;
        Move move;
        uint8_t color_to_move;

        std::atomic<int> visits;
        std::atomic<double> win_score;
        std::atomic<int> virtual_loss;
        
        double prior;
        std::atomic<bool> is_expanded;
        std::mutex expand_mutex;

        void add_dirichlet_noise(double epsilon, double alpha);

        void expand(Engine& engine, const std::vector<double>& policy = {});
        MCTSNode* select_child();
        void backpropagate(double result); 

        double puct_value(int total_visits) const;
        int get_max_depth() const;
    };

    class MCTSSearch
    {
    public:
        MCTSSearch(NNEvaluator* evaluator = nullptr, int threads = 16, size_t pipeline_target = 32);
        ~MCTSSearch();

        Move find_best_move(Engine& engine, int time_limit_ms, int max_simulations = -1);
        std::pair<Move, std::vector<std::pair<Move, double>>> find_best_move_with_policy(Engine& engine, int simulations, bool apply_noise = false);
        
    private:
        void search_worker(std::unique_ptr<Engine> thread_engine, MCTSNode* root, int simulations, std::chrono::steady_clock::time_point end_time, bool use_time);
        double simulate(Engine& engine);
        
        NNEvaluator* evaluator;
        int num_threads;
        size_t pipeline_target;
    };
}
