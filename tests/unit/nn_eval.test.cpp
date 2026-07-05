#include <gtest/gtest.h>
#include "include/eval/nn_eval.h"
#include "include/engine/engine.h"

using namespace fenrir;

class NNEvaluatorTest : public ::testing::Test {
};

TEST_F(NNEvaluatorTest, RequestEvaluation) {
    NNEvaluator eval("dummy", 2);
    Engine engine;
    
    auto f1 = eval.request_evaluation(engine.get_board_view());
    auto f2 = eval.request_evaluation(engine.get_board_view());
    auto f3 = eval.request_evaluation(engine.get_board_view());

    NNResult res1 = f1.get();
    NNResult res2 = f2.get();
    NNResult res3 = f3.get();

    EXPECT_EQ(res1.value, 0.5);
    EXPECT_EQ(res1.policy.size(), 4096);
    EXPECT_EQ(res2.value, 0.5);
    EXPECT_EQ(res3.value, 0.5);
}
