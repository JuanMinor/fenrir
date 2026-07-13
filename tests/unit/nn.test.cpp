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
    nn::NNEvaluator eval("dummy_does_not_exist", 2);
    Engine engine;
    
    auto f1 = eval.request_evaluation(engine.get_board_view());
    nn::NNResult res1 = f1.get();

    EXPECT_EQ(res1.value, 0.5);
    EXPECT_EQ(res1.policy.size(), 4096);
}

/**
 * Sleeps briefly after the request to ensure batch_worker_loop hits the
 * early-return path in try_reload_model.
 */
TEST_F(NNEvaluatorTest, RequestEvaluationWithModel) {
    nn::NNEvaluator eval("onnx/fenrir.onnx", 2);
    Engine engine;
    
    auto f1 = eval.request_evaluation(engine.get_board_view());
    nn::NNResult res1 = f1.get();

    EXPECT_EQ(res1.value, 0.0);
    EXPECT_EQ(res1.policy.size(), 4096);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST_F(NNEvaluatorTest, CatchInvalidModelLoad) {
    std::ofstream out("bad_model.onnx");
    out << "garbage data";
    out.close();

    nn::NNEvaluator eval("bad_model.onnx", 1);
    Engine engine;
    auto f1 = eval.request_evaluation(engine.get_board_view());
    nn::NNResult res1 = f1.get();

    EXPECT_EQ(res1.value, 0.5);

    std::filesystem::remove("bad_model.onnx");
}
