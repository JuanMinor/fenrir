#include "include/eval/nn_eval.h"
#include <chrono>
#include <iostream>
#include <fstream>

namespace fenrir
{
    NNEvaluator::NNEvaluator(const std::string& path, size_t b_size)
        : model_path(path), batch_size(b_size), stop_worker(false)
    {
        env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "Fenrir_NN");
        last_model_load_time = std::filesystem::file_time_type::min();
        last_reload_check_time = std::chrono::steady_clock::now() - std::chrono::seconds(2);
        
        try_reload_model();

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

    std::vector<float> NNEvaluator::board_to_tensor(const AbstractBoard& board)
    {
        // Simple 14-channel encoding: 12 for pieces, 1 for color, 1 for castling
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

        // Channel 12: side to move
        float color_val = (board.get_color() == 0) ? 1.0f : -1.0f;
        size_t color_offset = 12 * 64;
        for (size_t sq = 0; sq < 64; ++sq)
        {
            tensor[color_offset + sq] = color_val;
        }

        // Channel 13: castling rights
        float castling_val = static_cast<float>(board.get_castling_rights()[0]); // Just a mock representation
        size_t castling_offset = 13 * 64;
        for (size_t sq = 0; sq < 64; ++sq)
        {
            tensor[castling_offset + sq] = castling_val;
        }

        return tensor;
    }

    std::future<NNResult> NNEvaluator::request_evaluation(const AbstractBoard& board)
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

    void NNEvaluator::try_reload_model()
    {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_reload_check_time).count() < 1 && session) {
            return;
        }
        last_reload_check_time = now;

        if (!std::filesystem::exists(model_path)) return;

        std::error_code ec;
        auto write_time = std::filesystem::last_write_time(model_path, ec);
        if (ec) return;

        if (write_time > last_model_load_time || !session) {
            try {
                // Read the entire file into memory to avoid memory-mapped file locks (Access Violation on Windows)
                std::ifstream file(model_path, std::ios::binary | std::ios::ate);
                if (!file.is_open()) return; // File is locked by Python, try again later
                
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                std::vector<char> temp_buffer(static_cast<size_t>(size));
                if (!file.read(temp_buffer.data(), size)) return; // Failed to read, try again later

                Ort::SessionOptions session_options;
                session_options.SetIntraOpNumThreads(1);
                session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
                
                // CRITICAL: We must destroy the old session before freeing/overwriting the old buffer!
                session.reset();
                model_buffer = std::move(temp_buffer);

                auto new_session = std::make_unique<Ort::Session>(*env, model_buffer.data(), model_buffer.size(), session_options);

                session = std::move(new_session);
                last_model_load_time = write_time;
                std::cout << "Successfully loaded ONNX model: " << model_path << "\n";
            } catch (const std::exception& e) {
                std::cerr << "Failed to reload ONNX model: " << e.what() << "\n";
            }
        }
    }

    void NNEvaluator::batch_worker_loop()
    {
        while (true)
        {
            try_reload_model();
            
            std::vector<EvalRequest> batch;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                queue_cv.wait_for(lock, std::chrono::milliseconds(2), [this] {
                    return stop_worker || request_queue.size() >= batch_size;
                });

                if (stop_worker && request_queue.empty())
                {
                    break;
                }

                if (request_queue.empty())
                {
                    continue;
                }

                // Take up to batch_size items
                size_t take_count = std::min(request_queue.size(), batch_size);
                batch.reserve(take_count);
                for (size_t i = 0; i < take_count; ++i)
                {
                    batch.push_back(std::move(request_queue[i]));
                }
                request_queue.erase(request_queue.begin(), request_queue.begin() + static_cast<std::ptrdiff_t>(take_count));
            }

            if (!batch.empty())
            {
                std::vector<std::vector<float>> batch_features;
                batch_features.reserve(batch.size());
                std::vector<std::promise<NNResult>> promises;
                promises.reserve(batch.size());

                for (auto& req : batch)
                {
                    batch_features.push_back(std::move(req.features));
                    promises.push_back(std::move(req.promise));
                }

                try {
                    evaluate_batch(batch_features, promises);
                } catch (const std::exception& e) {
                    std::cerr << "Unhandled exception in evaluate_batch: " << e.what() << "\n";
                    for (auto& p : promises)
                    {
                        NNResult res;
                        res.value = 0.5;
                        res.policy.resize(4096, 0.01);
                        p.set_value(res);
                    }
                }
            }
        }
    }

    void NNEvaluator::evaluate_batch(const std::vector<std::vector<float>>& batch_features, std::vector<std::promise<NNResult>>& promises)
    {
        if (!session) {
            // Mock inference fallback when model is not available
            for (size_t i = 0; i < batch_features.size(); ++i)
            {
                NNResult res;
                res.value = 0.5; // Draw evaluation
                res.policy.resize(4096, 0.01); // Flat uniform policy
                promises[i].set_value(res);
            }
            return;
        }
        
        size_t batch_size_actual = batch_features.size();
        size_t feature_size = 14 * 8 * 8;
        
        std::vector<float> input_tensor_values;
        input_tensor_values.reserve(batch_size_actual * feature_size);
        for (const auto& features : batch_features)
        {
            input_tensor_values.insert(input_tensor_values.end(), features.begin(), features.end());
        }
        
        std::vector<int64_t> input_shape = {static_cast<int64_t>(batch_size_actual), 14, 8, 8};
        auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info, input_tensor_values.data(), input_tensor_values.size(), input_shape.data(), input_shape.size());
            
        const char* input_names[] = {"input"};
        const char* output_names[] = {"policy", "value"};
        
        try {
            auto output_tensors = session->Run(Ort::RunOptions{nullptr}, input_names, &input_tensor, 1, output_names, 2);
            
            const float* policy_data = output_tensors[0].GetTensorMutableData<float>();
            const float* value_data = output_tensors[1].GetTensorMutableData<float>();
            
            size_t policy_size = 4096; // Flat policy output size from the model
            
            for (size_t i = 0; i < batch_size_actual; ++i)
            {
                NNResult res;
                res.value = static_cast<double>(value_data[i]);
                res.policy.assign(policy_data + i * policy_size, policy_data + (i + 1) * policy_size);
                promises[i].set_value(res);
            }
        } catch (const std::exception& e) {
            std::cerr << "ONNX inference error: " << e.what() << "\n";
            for (size_t i = 0; i < batch_size_actual; ++i)
            {
                NNResult res;
                res.value = 0.5;
                res.policy.resize(4096, 0.01);
                promises[i].set_value(res);
            }
        }
    }
}
