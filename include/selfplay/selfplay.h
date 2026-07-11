#pragma once
#include "include/engine/engine.h"
#include "include/search/mcts.h"
#include "include/eval/nn_eval.h"
#include <string>
#include <vector>
#include <utility>

namespace fenrir
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

        std::string get_output_dir();
        void apply_dirichlet_noise(MCTSNode* root);

        int gpu_id_;
        int simulations;
        int max_games;
        std::string start_fen;
    };
}
