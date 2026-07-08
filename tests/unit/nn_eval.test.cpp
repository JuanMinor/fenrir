#include <gtest/gtest.h>
#include "include/eval/nn_eval.h"
#include "include/engine/engine.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace fenrir;

class NNEvaluatorTest : public ::testing::Test {
};

TEST_F(NNEvaluatorTest, RequestEvaluationFallback) {
    NNEvaluator eval("dummy_does_not_exist", 2);
    Engine engine;
    
    auto f1 = eval.request_evaluation(engine.get_board_view());
    NNResult res1 = f1.get();

    EXPECT_EQ(res1.value, 0.5);
    EXPECT_EQ(res1.policy.size(), 4096);
}

TEST_F(NNEvaluatorTest, RequestEvaluationWithModel) {
    NNEvaluator eval("onnx/fenrir.onnx", 2);
    Engine engine;
    
    auto f1 = eval.request_evaluation(engine.get_board_view());
    NNResult res1 = f1.get();

    EXPECT_EQ(res1.value, 0.0);
    EXPECT_EQ(res1.policy.size(), 4096);
    
    // Wait a little to ensure batch_worker_loop hits the early return in try_reload_model
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST_F(NNEvaluatorTest, CatchInvalidModelLoad) {
    std::ofstream out("bad_model.onnx");
    out << "garbage data";
    out.close();

    NNEvaluator eval("bad_model.onnx", 1);
    Engine engine;
    auto f1 = eval.request_evaluation(engine.get_board_view());
    NNResult res1 = f1.get();

    EXPECT_EQ(res1.value, 0.5);

    std::filesystem::remove("bad_model.onnx");
}
