#pragma once
#include "include/engine/engine.h"
#include "include/search/mcts.h"
#include "include/eval/nn_eval.h"
#include <string>
#include <memory>

namespace fenrir
{
    class UCI
    {
    public:
        UCI();
        ~UCI();

        void loop();

    private:
        void parse_position(const std::string& command);
        void parse_go(const std::string& command);

        std::unique_ptr<Engine> engine;
        std::unique_ptr<NNEvaluator> evaluator;
        std::unique_ptr<MCTSSearch> search;
    };
}
