#pragma once

#include "include/abstract/board.h"
#include <vector>
#include <future>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

#ifdef FENRIR_USE_ONNX
#include <onnxruntime_cxx_api.h>
#endif

namespace fenrir
{
    struct NNResult {
        double value; 
        std::vector<double> policy; // Flat policy array, sized for maximum possible moves (e.g. 4096 or mapped to legal moves)
    };

    class NNEvaluator {
    public:
        NNEvaluator(const std::string& model_path, size_t batch_size = 256);
        ~NNEvaluator();

        // Threads call this to queue a board state for evaluation.
        std::future<NNResult> request_evaluation(const AbstractBoard& board);

        // Convert board to input tensor features (14 channels * 8 * 8 = 896 floats)
        static std::vector<float> board_to_tensor(const AbstractBoard& board);

    private:
        void batch_worker_loop();
        void evaluate_batch(const std::vector<std::vector<float>>& batch_features, std::vector<std::promise<NNResult>>& promises);

        std::string model_path;
        size_t batch_size;

        struct EvalRequest {
            std::vector<float> features;
            std::promise<NNResult> promise;
        };

        std::vector<EvalRequest> request_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_cv;
        bool stop_worker;
        std::thread worker_thread;

#ifdef FENRIR_USE_ONNX
        std::unique_ptr<Ort::Env> env;
        std::unique_ptr<Ort::Session> session;
#endif
    };
}
