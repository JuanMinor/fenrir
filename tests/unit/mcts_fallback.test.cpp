#include <gtest/gtest.h>
#include "include/search/mcts.h"
#include "include/engine/engine.h"

using namespace chess;

TEST(MCTSTestFallback, FallbackToSynchronous)
{
    Engine engine;
    MCTSSearch search(nullptr, 100000); // Exceed OS thread limits
    search.find_best_move_with_policy(engine, 10, false);
}
