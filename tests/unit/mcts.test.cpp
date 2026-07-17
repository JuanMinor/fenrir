#include <gtest/gtest.h>
#include "include/mcts/mcts.h"
#include "include/engine/engine.h"

using namespace chess;

class MCTSTest : public ::testing::Test
{
protected:
    Engine engine;
    mcts::Tree search{nullptr, 2, 8};
};

TEST_F(MCTSTest, FindBestMove_InitialPosition)
{
    chess::Move best_move = search.find_best_move(engine, 50, -1);

    auto legal_moves = engine.generate_all_moves();
    bool is_legal = false;
    for (const auto& m : legal_moves)
    {
        if (m.get_from_square() == best_move.get_from_square() &&
            m.get_to_square() == best_move.get_to_square() &&
            m.get_promotion_piece() == best_move.get_promotion_piece())
        {
            is_legal = true;
            break;
        }
    }
    EXPECT_TRUE(is_legal);
}

TEST_F(MCTSTest, NodeExpansionAndPuct)
{
    chess::Move dummy_move(0, 0);
    mcts::Node root(nullptr, dummy_move, WHITE);

    root.expand(engine);
    EXPECT_TRUE(root.is_expanded);
    EXPECT_EQ(root.children.size(), 20U);

    mcts::Node* child = root.select_child();
    EXPECT_NE(child, nullptr);
    EXPECT_EQ(child->visits, 0);

    child->backpropagate(1.0);
    EXPECT_EQ(child->visits, 1);
    EXPECT_EQ(child->win_score, 1.0);
    EXPECT_EQ(root.visits, 1);
    EXPECT_EQ(root.win_score, 0.0);
}

TEST_F(MCTSTest, CheckmateInOne)
{
    Engine mate_engine("7k/5Q2/5K2/8/8/8/8/8 w - - 0 1");

    mcts::Tree search_single(nullptr, 1, 8);
    chess::Move best_move = search_single.find_best_move(mate_engine, -1, 400);

    EXPECT_EQ(best_move.get_from(), "f7");
    EXPECT_EQ(best_move.get_to(), "g7");
}

TEST_F(MCTSTest, DirichletNoiseAndPolicyExpand)
{
    chess::Move dummy_move(0, 0);
    mcts::Node root(nullptr, dummy_move, WHITE);

    std::vector<double> policy(4672, 0.0);
    policy[0] = 1.0;

    root.expand(engine, policy);
    EXPECT_TRUE(root.is_expanded);
    EXPECT_EQ(root.children.size(), 20U);

    root.add_dirichlet_noise(0.25, 0.3);
    EXPECT_TRUE(root.is_expanded);

    auto result = search.find_best_move_with_policy(engine, 50, true);
    chess::Move best_move = result.first;
    auto extracted_policy = result.second;

    EXPECT_GT(extracted_policy.size(), 0U);
    EXPECT_NE(best_move.to_uci_notation(), "0000");
}

TEST_F(MCTSTest, EmptyChildrenReturnEmptyMove)
{
    Engine mated_engine("7k/5Q2/5K2/8/8/8/8/8 b - - 0 1");
    chess::Move m = search.find_best_move(mated_engine, 10, -1);
    EXPECT_EQ(m.to_uci_notation(), "a1a1");

    auto result = search.find_best_move_with_policy(mated_engine, 10, false);
    EXPECT_EQ(result.first.to_uci_notation(), "a1a1");
    EXPECT_EQ(result.second.size(), 0U);
}


/**
 * White's king is trapped with only pawn moves available against Black's 3
 * queens, so the random rollout in mcts::Tree::simulate will reliably
 * checkmate White and exercise the result = 1.0 path.
 */
TEST_F(MCTSTest, RolloutOpponentCheckmate)
{
    Engine custom_engine("K7/2qqq3/8/8/8/8/1P6/k7 w - - 0 1");
    search.find_best_move(custom_engine, 400, -1);
    SUCCEED();
}

/**
 * Passing 0 threads forces the synchronous fallback path.
 */
TEST_F(MCTSTest, FallbackToSynchronous)
{
    mcts::Tree sync_search(nullptr, 0, 8);
    auto result = sync_search.find_best_move_with_policy(engine, 10, false);
    EXPECT_NE(result.first.to_uci_notation(), "0000");
}
