#include <gtest/gtest.h>
#include "include/nn/nn.h"
#include "include/engine/engine.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace chess;

class NNEvaluatorTest : public ::testing::Test {
};

TEST_F(NNEvaluatorTest, RequestEvaluationFallback) {
    nn::NN eval("dummy_does_not_exist", 0, 2, 2);
    Engine engine;

    auto f1 = eval.request_evaluation(engine.get_board_view());
    nn::Result res1 = f1.get();

    EXPECT_EQ(res1.value, 0.5);
    EXPECT_EQ(res1.policy.size(), nn::POLICY_SIZE);
}

/**
 * Sleeps briefly after the request to ensure batch_worker_loop hits the
 * early-return path in try_reload_model.
 */
TEST_F(NNEvaluatorTest, RequestEvaluationWithModel) {
    nn::NN eval("onnx/fenrir.onnx", 0, 2, 2);
    Engine engine;

    auto f1 = eval.request_evaluation(engine.get_board_view());
    nn::Result res1 = f1.get();

    /* Real model output: the tanh value head is converted to a win
     * probability in [0, 1]; exact values depend on the trained weights. */
    EXPECT_GE(res1.value, 0.0);
    EXPECT_LE(res1.value, 1.0);
    EXPECT_EQ(res1.policy.size(), nn::POLICY_SIZE);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST_F(NNEvaluatorTest, CatchInvalidModelLoad) {
    std::ofstream out("bad_model.onnx");
    out << "garbage data";
    out.close();

    nn::NN eval("bad_model.onnx", 0, 1, 2);
    Engine engine;
    auto f1 = eval.request_evaluation(engine.get_board_view());
    nn::Result res1 = f1.get();

    EXPECT_EQ(res1.value, 0.5);

    std::filesystem::remove("bad_model.onnx");
}
