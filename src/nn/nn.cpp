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

#include "include/nn/nn.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <stdexcept>
#ifdef _WIN32
#define NOMINMAX
#include <dml_provider_factory.h>
#endif

namespace nn
{
    /**
     * Loads the ONNX model, detects hardware, and starts the batch worker
     * thread. batch_timeout_ms is set to 25% of the measured inference
     * latency (minimum 2ms). @p b_size overrides the hardware-recommended
     * batch size when nonzero.
     */
    NNEvaluator::NNEvaluator(const std::string &path, int gpu_id, size_t b_size)
        : model_path(path), gpu_id_(gpu_id), stop_worker(false)
    {
        env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "Fenrir_NN");
        last_model_load_time = std::filesystem::file_time_type::min();
        last_reload_check_time = std::chrono::steady_clock::now() - std::chrono::seconds(2);

        try_reload_model();

        detect_hardware();
        int latency = measure_latency_ms();
        hw_profile.inference_latency_ms = latency;
        batch_timeout_ms = std::max(2, latency / 4);

        batch_size = (b_size > 0) ? b_size : hw_profile.batch_size;

        worker_thread = std::thread(&NNEvaluator::batch_worker_loop, this);
    }

    NNEvaluator::~NNEvaluator()
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop_worker = true;
        }
        queue_cv.notify_all();
        if (worker_thread.joinable())
        {
            worker_thread.join();
        }
    }

    /**
     * Encodes @p board into a 14x8x8 float tensor for the NN: channels 0-11
     * are per-piece-type bitboards, channel 12 is the side to move (+1.0 for
     * white, -1.0 for black) broadcast across all squares, and channel 13 is
     * the castling-rights encoding broadcast across all squares, computed to
     * exactly mirror Python's `float(ord(castling_rights[0])) if len > 0 else
     * 0.0`.
     */
    std::vector<float> NNEvaluator::board_to_tensor(const chess::AbstractBoard &board)
    {
        std::vector<float> tensor(14 * 8 * 8, 0.0f);

        for (int i = 0; i < 12; ++i)
        {
            uint64_t bb = board.get_bitboard(i);
            size_t channel_offset = static_cast<size_t>(i) * 64;
            for (size_t sq = 0; sq < 64; ++sq)
            {
                if (bb & (1ULL << sq))
                {
                    tensor[channel_offset + sq] = 1.0f;
                }
            }
        }

        float color_val = (board.get_color() == 0) ? 1.0f : -1.0f;
        size_t color_offset = 12 * 64;
        for (size_t sq = 0; sq < 64; ++sq)
        {
            tensor[color_offset + sq] = color_val;
        }

        const std::string &castling_rights = board.get_castling_rights();
        float castling_val = 0.0f;
        if (!castling_rights.empty())
        {
            castling_val = static_cast<float>(static_cast<unsigned char>(castling_rights[0]));
        }

        size_t castling_offset = 13 * 64;
        for (size_t sq = 0; sq < 64; ++sq)
        {
            tensor[castling_offset + sq] = castling_val;
        }

        return tensor;
    }

    std::future<NNResult> NNEvaluator::request_evaluation(const chess::AbstractBoard &board)
    {
        EvalRequest req;
        req.features = board_to_tensor(board);
        auto future = req.promise.get_future();

        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            request_queue.push_back(std::move(req));
        }

        queue_cv.notify_one();
        return future;
    }

    void NNEvaluator::batch_worker_loop()
    {
        while (true)
        {
            try_reload_model();

            std::vector<EvalRequest> batch;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                queue_cv.wait_for(lock, std::chrono::milliseconds(batch_timeout_ms), [this]
                                  { return stop_worker || request_queue.size() >= batch_size; });

                if (stop_worker && request_queue.empty())
                {
                    break;
                }

                if (request_queue.empty())
                {
                    continue;
                }

                size_t take_count = std::min(request_queue.size(), batch_size);
                batch.reserve(take_count);
                for (size_t i = 0; i < take_count; ++i)
                {
                    batch.push_back(std::move(request_queue.front()));
                    request_queue.pop_front();
                }
            }

            if (!batch.empty())
            {
                std::vector<std::vector<float>> batch_features;
                batch_features.reserve(batch.size());
                std::vector<std::promise<NNResult>> promises;
                promises.reserve(batch.size());

                for (auto &req : batch)
                {
                    batch_features.push_back(std::move(req.features));
                    promises.push_back(std::move(req.promise));
                }

                try
                {
                    evaluate_batch(batch_features, promises);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Unhandled exception in evaluate_batch: " << e.what() << "\n";
                    for (auto &p : promises)
                    {
                        try
                        {
                            NNResult res;
                            res.value = 0.5;
                            res.policy.resize(4096, 0.01);
                            p.set_value(res);
                        }
                        catch (...)
                        {
                        }
                    }
                }
            }
        }
    }

    /**
     * Populates hw_profile from the detected core count: search_threads
     * reserves one core for the OS and one for the NN batch worker,
     * pipeline_target is fixed at 32 (enough in-flight items to saturate the
     * GPU), and batch_size scales with search_threads * pipeline_target.
     */
    void NNEvaluator::detect_hardware()
    {
        int logical_cores = static_cast<int>(std::thread::hardware_concurrency());
        if (logical_cores <= 0) logical_cores = 4;

        hw_profile.logical_cores = logical_cores;

        hw_profile.search_threads = std::max(1, logical_cores - 2);

        hw_profile.pipeline_target = 32;

        hw_profile.batch_size = static_cast<size_t>(hw_profile.search_threads) * hw_profile.pipeline_target;
    }

    void NNEvaluator::evaluate_batch(const std::vector<std::vector<float>> &batch_features, std::vector<std::promise<NNResult>> &promises)
    {
        if (!session)
        {
            for (size_t i = 0; i < batch_features.size(); ++i)
            {
                NNResult res;
                res.value = 0.5;
                res.policy.resize(4096, 0.01);
                promises[i].set_value(res);
            }
            return;
        }

        size_t batch_size_actual = batch_features.size();
        size_t feature_size = 14 * 8 * 8;

        std::vector<float> input_tensor_values;
        input_tensor_values.reserve(batch_size_actual * feature_size);
        for (const auto &features : batch_features)
        {
            input_tensor_values.insert(input_tensor_values.end(), features.begin(), features.end());
        }

        std::vector<int64_t> input_shape = {static_cast<int64_t>(batch_size_actual), 14, 8, 8};
        auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info, input_tensor_values.data(), input_tensor_values.size(), input_shape.data(), input_shape.size());

        const char *const input_names[] = {"input"};
        const char *const output_names[] = {"policy", "value"};

        std::vector<bool> promise_set(batch_size_actual, false);
        try
        {
            auto output_tensors = session->Run(Ort::RunOptions{nullptr}, input_names, &input_tensor, 1, output_names, 2);

            auto policy_info = output_tensors[0].GetTensorTypeAndShapeInfo();
            auto value_info = output_tensors[1].GetTensorTypeAndShapeInfo();
            size_t policy_size = 4096;

            if (policy_info.GetElementCount() < batch_size_actual * policy_size || value_info.GetElementCount() < batch_size_actual)
            {
                throw std::runtime_error("ONNX model returned unexpected tensor size.");
            }

            const float *policy_data = output_tensors[0].GetTensorData<float>();
            const float *value_data = output_tensors[1].GetTensorData<float>();

            for (size_t i = 0; i < batch_size_actual; ++i)
            {
                NNResult res;
                res.value = static_cast<double>(value_data[i]);
                res.policy.assign(policy_data + i * policy_size, policy_data + (i + 1) * policy_size);
                promises[i].set_value(res);
                promise_set[i] = true;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "ONNX inference error: " << e.what() << "\n";
            for (size_t i = 0; i < batch_size_actual; ++i)
            {
                if (!promise_set[i])
                {
                    try
                    {
                        NNResult res;
                        res.value = 0.5;
                        res.policy.resize(4096, 0.01);
                        promises[i].set_value(res);
                    }
                    catch (...)
                    {
                    }
                    promise_set[i] = true;
                }
            }
        }
    }

    /**
     * Runs one dummy full-size batch through evaluate_batch() and times it,
     * to calibrate batch_timeout_ms. Returns 4ms if no model is loaded yet.
     */
    int NNEvaluator::measure_latency_ms()
    {
        if (!session) return 4;

        std::vector<std::vector<float>> dummy_batch;
        for (size_t i = 0; i < hw_profile.batch_size; ++i) {
            dummy_batch.push_back(std::vector<float>(14 * 8 * 8, 0.0f));
        }
        std::vector<std::promise<NNResult>> dummy_promises(hw_profile.batch_size);

        auto start = std::chrono::steady_clock::now();

        try {
            evaluate_batch(dummy_batch, dummy_promises);
        } catch (...) {}

        auto end = std::chrono::steady_clock::now();
        return static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    }

    void NNEvaluator::try_reload_model()
    {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_reload_check_time).count() < 1 && session)
        {
            return;
        }
        last_reload_check_time = now;

        if (!std::filesystem::exists(model_path))
            return;

        std::error_code ec;
        auto write_time = std::filesystem::last_write_time(model_path, ec);
        if (ec)
            return;

        if (write_time > last_model_load_time || !session)
        {
            try
            {
                std::ifstream file(model_path, std::ios::binary | std::ios::ate);
                if (!file.is_open())
                    return;

                std::streamsize size = file.tellg();
                if (size <= 0)
                    return;
                file.seekg(0, std::ios::beg);
                std::vector<char> temp_buffer(static_cast<size_t>(size));
                if (!file.read(temp_buffer.data(), size))
                    return;

                Ort::SessionOptions session_options;
                session_options.SetIntraOpNumThreads(1);
                session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

#ifdef _WIN32
                try
                {
                    session_options.DisableMemPattern();
                    session_options.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
                    Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_DML(session_options, 0));
                }
                catch (...)
                {
                    std::cerr << "Warning: Could not enable DirectML. Falling back to CPU.\n";
                }
#else
                try
                {
                    OrtCUDAProviderOptions cuda_options;
                    cuda_options.device_id = gpu_id_;
                    session_options.AppendExecutionProvider_CUDA(cuda_options);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Warning: Could not enable CUDA for GPU " << gpu_id_ << ". Falling back to CPU.\n";
                    std::cerr << "ONNX Runtime Error: " << e.what() << "\n";
                }
#endif

                session.reset();
                model_buffer = std::move(temp_buffer);
                last_model_load_time = write_time;

                auto new_session = std::make_unique<Ort::Session>(*env, model_buffer.data(), model_buffer.size(), session_options);
                session = std::move(new_session);
                std::cout << "Successfully loaded ONNX model: " << model_path << "\n";
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to reload ONNX model: " << e.what() << "\n";
            }
        }
    }
}
