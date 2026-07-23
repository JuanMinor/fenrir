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
#include <atomic>
#include <algorithm>
#ifdef _WIN32
#define NOMINMAX
/* logger.h defines an ERROR() macro; wingdi.h (pulled in transitively by
 * the DirectML header) defines ERROR too, which is a fatal C4005 under
 * /WX. NOGDI excludes the GDI section of wingdi.h — DirectML/D3D12 don't
 * need it — leaving the logger macro intact. */
#define NOGDI
#include <dml_provider_factory.h>
#endif

namespace nn
{
    /**
     * Loads the ONNX model, and starts the batch worker thread. batch_timeout_ms
     * is supplied by the caller (the auto-tuner measures it once via
     * measure_latency_ms() and persists it) rather than self-calibrated here,
     * since this constructor runs on every NN instantiation.
     */
    NN::NN(const std::string &onnx_file_path, int gpu, size_t batch_size, int batch_timeout_ms)
        : onnx_file_path(onnx_file_path), batch_size(batch_size), gpu(gpu), batch_timeout_ms(batch_timeout_ms), stop_worker(false)
    {
        env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "Fenrir_NN");
        last_model_load_time = std::filesystem::file_time_type::min();
        last_reload_check_time = std::chrono::steady_clock::now() - std::chrono::seconds(2);

        try_reload_model();

        worker_thread = std::thread(&NN::batch_worker_loop, this);
    }

    NN::~NN()
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
    std::vector<float> NN::board_to_tensor(const chess::AbstractBoard &board)
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

    std::future<Result> NN::request_evaluation(const chess::AbstractBoard &board)
    {
        Request request;
        request.features = board_to_tensor(board);
        std::future<nn::Result> future = request.promise.get_future();

        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            request_queue.emplace_back(std::move(request));
        }

        queue_cv.notify_one();
        return future;
    }

    void NN::batch_worker_loop()
    {
        while (true)
        {
            /* TODO check with flag if we're training, if not, skip this line. Performance issue, small but adds up. */
            {
                std::lock_guard<std::mutex> session_lock(session_mutex);
                try_reload_model();
            }

            std::vector<Request> requests;
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
                requests.reserve(take_count);
                for (size_t i = 0; i < take_count; ++i)
                {
                    requests.emplace_back(std::move(request_queue.front()));
                    request_queue.pop_front();
                }
            }

            if (!requests.empty())
            {
                std::vector<std::vector<float>> batch_features;
                batch_features.reserve(requests.size());
                std::vector<std::promise<Result>> promises;
                promises.reserve(requests.size());

                for (Request &request : requests)
                {
                    batch_features.emplace_back(std::move(request.features));
                    promises.emplace_back(std::move(request.promise));
                }

                try
                {
                    std::lock_guard<std::mutex> session_lock(session_mutex);
                    evaluate_batch(batch_features, promises);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Unhandled exception in evaluate_batch: " << e.what() << "\n";
                    for (std::promise<nn::Result> &promise : promises)
                    {
                        try
                        {
                            Result result;
                            result.value = 0.5;
                            result.policy.resize(POLICY_SIZE, 0.01);
                            promise.set_value(result);
                        }
                        catch (...)
                        {
                            /* TODO: Maybe we can do something here but leave blank for now. */
                        }
                    }
                }
            }
        }
    }

    void NN::evaluate_batch(const std::vector<std::vector<float>> &batch_features, std::vector<std::promise<Result>> &promises)
    {
        if (!session)
        {
            /* Serving uniform fallbacks looks like a very fast evaluator from
             * the outside; say so once, loudly, so benchmarks and searches
             * can't silently measure fake evaluations. */
            static std::atomic<bool> warned{false};
            if (!warned.exchange(true))
            {
                std::cerr << "WARNING: NN has no model session loaded; serving uniform fallback evaluations (not real inference).\n";
            }
            for (size_t i = 0; i < batch_features.size(); ++i)
            {
                Result result;
                result.value = 0.5;
                result.policy.resize(POLICY_SIZE, 0.01);
                promises[i].set_value(result);
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
            size_t policy_size = POLICY_SIZE;

            /* Exact match required: a policy head wider than POLICY_SIZE
             * would silently misalign every batch item after the first. */
            if (policy_info.GetElementCount() != batch_size_actual * policy_size || value_info.GetElementCount() != batch_size_actual)
            {
                throw std::runtime_error("ONNX model returned unexpected tensor size.");
            }

            const float *policy_data = output_tensors[0].GetTensorData<float>();
            const float *value_data = output_tensors[1].GetTensorData<float>();

            for (size_t i = 0; i < batch_size_actual; ++i)
            {
                Result result;
                /* The model's value head is tanh in [-1, 1] (train.py maps
                 * results to that range); MCTS consumes win probability in
                 * [0, 1] alongside terminal scores of 0.0/0.5/1.0. Clamp
                 * away from the exact extremes: a saturated estimate of
                 * 1.0 would tie with a PROVEN in-search checkmate, leaving
                 * the search indifferent between mating and shuffling in
                 * won positions (observed: queen-up arena games wandering
                 * into the 50-move rule). A proven terminal must always
                 * outbid an estimate — by MORE than PUCT's exploration
                 * noise: a 0.005 margin measurably failed (tactics exam
                 * missed mate-in-1s exactly where the head saturates to
                 * 1.0000; U-term jitter at 800 sims is ~0.05), so the gap
                 * must be on that order. */
                double win_probability = (static_cast<double>(value_data[i]) + 1.0) / 2.0;
                result.value = std::clamp(win_probability, 0.03, 0.97);
                result.policy.assign(policy_data + i * policy_size, policy_data + (i + 1) * policy_size);
                promises[i].set_value(result);
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
                        Result result;
                        result.value = 0.5;
                        result.policy.resize(POLICY_SIZE, 0.01);
                        promises[i].set_value(result);
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
     * so callers can calibrate batch_timeout_ms from a real measurement.
     * Returns 4ms if no model is loaded yet.
     */
    int NN::measure_latency_ms()
    {
        std::lock_guard<std::mutex> session_lock(session_mutex);

        if (!session)
            return 4;

        std::vector<std::vector<float>> dummy_batch(batch_size, std::vector<float>(14 * 8 * 8, 0.0f));
        std::vector<std::promise<Result>> dummy_promises(batch_size);

        auto start = std::chrono::steady_clock::now();
        try
        {
            evaluate_batch(dummy_batch, dummy_promises);
        }
        catch (...)
        {
        }
        auto end = std::chrono::steady_clock::now();

        return static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    }

    void NN::try_reload_model()
    {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_reload_check_time).count() < 1 && session)
        {
            return;
        }
        last_reload_check_time = now;

        if (!std::filesystem::exists(onnx_file_path))
        {
            return;
        }

        std::error_code error_code;
        auto write_time = std::filesystem::last_write_time(onnx_file_path, error_code);
        if (error_code)
        {
            return;
        }

        if (write_time > last_model_load_time || !session)
        {
            try
            {
                std::ifstream file(onnx_file_path, std::ios::binary | std::ios::ate);
                if (!file.is_open())
                {
                    return;
                }

                std::streamsize size = file.tellg();
                if (size <= 0)
                {
                    return;
                }
                file.seekg(0, std::ios::beg);
                std::vector<char> temp_buffer(static_cast<size_t>(size));
                if (!file.read(temp_buffer.data(), size))
                {
                    return;
                }

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
                    std::string error = "Could not enable DirectML. Falling back to CPU.";
                    std::cerr << "Warning: " << error << "\n";
                    /* GUIs (Arena etc.) that launch Fenrir as a child process
                     * typically don't surface stderr, so without this the
                     * fallback was invisible -- logger:: persists to
                     * logs/fenrir.log regardless of who owns the console,
                     * matching the CUDA path below. */
                    logger::CRITICAL(error);
                }
#else
                try
                {
                    OrtCUDAProviderOptions cuda_options;
                    cuda_options.device_id = gpu;
                    session_options.AppendExecutionProvider_CUDA(cuda_options);
                }
                catch (const std::exception &e)
                {
                    std::string error = "Could not enable CUDA for GPU " + std::to_string(gpu) + ". Falling back to CPU.";
                    error += "ONNX Runtime error: " + std::string(e.what());
                    std::cerr << error << "\n";
                    logger::CRITICAL(error);
                }
#endif

                session.reset();
                model_buffer = std::move(temp_buffer);
                last_model_load_time = write_time;

                auto new_session = std::make_unique<Ort::Session>(*env, model_buffer.data(), model_buffer.size(), session_options);
                session = std::move(new_session);
            }
            catch (const std::exception &e)
            {
                std::string error = "Failed to reload ONNX model: " + std::string(e.what());
                std::cerr << error << "\n";
                logger::CRITICAL(error);
            }
        }
    }
}
