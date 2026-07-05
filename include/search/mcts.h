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
    };

    class MCTSSearch
    {
    public:
        MCTSSearch(NNEvaluator* evaluator = nullptr, int threads = 16);
        ~MCTSSearch();

        Move find_best_move(Engine& engine, int simulations);
        std::pair<Move, std::vector<std::pair<Move, double>>> find_best_move_with_policy(Engine& engine, int simulations, bool apply_noise = false);
        
    private:
        void search_worker(std::unique_ptr<Engine> thread_engine, MCTSNode* root, int simulations);
        double simulate(Engine& engine);
        
        NNEvaluator* evaluator;
        int num_threads;
    };
}
