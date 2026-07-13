#include <gtest/gtest.h>
#include "include/mcts/mcts.h"
#include "include/engine/engine.h"

using namespace chess;

/**
 * An absurdly high thread count exceeds OS thread limits, forcing the
 * synchronous fallback path.
 */
TEST(MCTSTestFallback, FallbackToSynchronous)
{
    Engine engine;
    MCTSSearch search(nullptr, 100000);
    search.find_best_move_with_policy(engine, 10, false);
}
