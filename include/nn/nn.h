/*
 *   Copyright (c) 2026 Juan Minor
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

namespace nn
{
    struct NNResult
    {
        double value;
        std::vector<double> policy;
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

        static std::vector<float> board_to_tensor(const chess::AbstractBoard &board);

        HardwareProfile get_hardware_profile() const { return hw_profile; }

        std::future<NNResult> request_evaluation(const chess::AbstractBoard &board);

    private:
        void batch_worker_loop();
        void detect_hardware();
        void evaluate_batch(const std::vector<std::vector<float>> &batch_features, std::vector<std::promise<NNResult>> &promises);
        int measure_latency_ms();
        void try_reload_model();

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
