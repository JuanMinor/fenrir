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
#include "include/logger/logger.h"
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
    /*
     * Width of the model's policy head: AlphaZero 73-plane move encoding,
     * indexed as (from_square * 73) + channel. Must match policy_fc in
     * training/model.py and move_index() in src/mcts/mcts.cpp.
     */
    constexpr size_t POLICY_SIZE = 4672;

    struct Result
    {
        double value;
        std::vector<double> policy;
    };

    struct Request
    {
        std::vector<float> features;
        std::promise<Result> promise;
    };

    class NN
    {
    public:
        NN(const std::string &onnx_file_path, int gpu, size_t batch_size, int batch_timeout_ms);
        ~NN();

        static std::vector<float> board_to_tensor(const chess::AbstractBoard &board);

        /**
         * @brief Runs one dummy full-size batch through evaluate_batch() and times it.
         * @returns The measured latency in milliseconds, so callers (the auto-tuner)
         * can calibrate batch_timeout_ms from a real measurement.
         */
        int measure_latency_ms();

        std::future<Result> request_evaluation(const chess::AbstractBoard &board);

    private:
        void batch_worker_loop();
        void evaluate_batch(const std::vector<std::vector<float>> &batch_features, std::vector<std::promise<Result>> &promises);
        void try_reload_model();

        std::string onnx_file_path;
        size_t batch_size;
        int gpu;
        int batch_timeout_ms;

        std::deque<Request> request_queue;
        /* Serializes access to `session` between the batch worker thread
         * (evaluate_batch/try_reload_model can swap the session) and public
         * callers such as measure_latency_ms(). */
        std::mutex session_mutex;
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
