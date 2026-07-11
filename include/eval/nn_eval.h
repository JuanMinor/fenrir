#pragma once

#include "include/abstract/board.h"
#include <vector>
#include <deque>
#include <future>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <filesystem>
#include <chrono>
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#include <onnxruntime_cxx_api.h>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace fenrir
{
    struct NNResult
    {
        double value;
        std::vector<double> policy; // Flat policy array, sized for maximum possible moves (e.g. 4096 or mapped to legal moves)
    };

    struct HardwareProfile
    {
        int logical_cores;
        int search_threads;
        size_t batch_size;
        size_t pipeline_target;
        int batch_timeout_ms;
        int inference_latency_ms;
    };

    class NNEvaluator
    {
    public:
        NNEvaluator(const std::string &model_path, int gpu_id = 0, size_t batch_size = 512);
        ~NNEvaluator();

        // Threads call this to queue a board state for evaluation.
        std::future<NNResult> request_evaluation(const AbstractBoard &board);

        // Convert board to input tensor features (14 channels * 8 * 8 = 896 floats)
        static std::vector<float> board_to_tensor(const AbstractBoard &board);

        HardwareProfile get_hardware_profile() const { return hw_profile; }

    private:
        void batch_worker_loop();
        void evaluate_batch(const std::vector<std::vector<float>> &batch_features, std::vector<std::promise<NNResult>> &promises);
        void try_reload_model();
        void detect_hardware();
        int measure_latency_ms();

        std::string model_path;
        size_t batch_size;
        int gpu_id_;
        HardwareProfile hw_profile;
        int batch_timeout_ms;

        struct EvalRequest
        {
            std::vector<float> features;
            std::promise<NNResult> promise;
        };

        std::deque<EvalRequest> request_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_cv;
        bool stop_worker;
        std::thread worker_thread;

        std::unique_ptr<Ort::Env> env;
        std::vector<char> model_buffer;
        std::unique_ptr<Ort::Session> session;
        std::filesystem::file_time_type last_model_load_time;
        std::chrono::steady_clock::time_point last_reload_check_time;
    };
}
