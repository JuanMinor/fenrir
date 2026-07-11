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

#include "include/tuner/auto_tuner.h"
#include "include/search/mcts.h"
#include "include/eval/nn_eval.h"
#include "include/engine/engine.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <fstream>

namespace tuner
{
    AutoTuner::AutoTuner(TuningParameters &tuning_parameters)
        : baseline_tuning_parameters(tuning_parameters), real_tuning_parameters(tuning_parameters)
    {
    }

    AutoTuner::~AutoTuner() = default;

    TuningParameters AutoTuner::run()
    {
        std::cout << "info string [Auto-Tuner] Starting FENRIR Hardware Tuning Phase...\n";
        
        // KiwiPete FEN: Highly branched position perfect for stressing MCTS/GPU
        const std::string KIWIPETE = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

        uint8_t threads = baseline_tuning_parameters.get_search_threads();
        uint16_t baseline_batch = baseline_tuning_parameters.get_batch_size();
        
        std::cout << "info string [Auto-Tuner] Baseline configured by hardware: Threads = " << static_cast<int>(threads) 
                  << ", Batch Size = " << baseline_batch << "\n";

        // Phase 1: Test scaling batch size while locking threads to find peak GPU/PCIe throughput
        std::vector<uint16_t> batch_sizes_to_test = {64, 128, 256, 512, 1024, 2048};
        
        double best_nps = 0.0;
        uint16_t best_batch = baseline_batch;
        uint16_t best_pipeline = baseline_tuning_parameters.get_pipeline_target();

        for (uint16_t test_batch : batch_sizes_to_test)
        {
            uint32_t base_pipe = test_batch * DEFAULT_BASE_PIPELINE;
            uint32_t thread_min = threads * DEFAULT_THREAD_MINIMUM_PIPELINE_BATCH;
            uint16_t test_pipeline = static_cast<uint16_t>(std::max(base_pipe, thread_min));

            std::cout << "info string [Auto-Tuner] Profiling GPU Batch " << test_batch << "...\n";

            try {
                // Initialize evaluator and search tree
                auto evaluator = std::make_unique<fenrir::NNEvaluator>("onnx/fenrir.onnx", baseline_tuning_parameters.get_gpu_id(0), test_batch);
                fenrir::MCTSSearch search(evaluator.get(), threads, test_pipeline);
                fenrir::Engine engine(KIWIPETE);

                // Run a 10s continuous benchmark to measure sustained NPS
                auto start = std::chrono::steady_clock::now();
                int nodes_evaluated = search.benchmark_search(engine, 10000);
                auto end = std::chrono::steady_clock::now();
                
                double ms = std::chrono::duration<double, std::milli>(end - start).count();
                double nps = (nodes_evaluated / ms) * 1000.0;

                std::cout << "info string [Auto-Tuner] NPS = " << static_cast<int>(nps) << " (took " << ms << "ms)\n";

                if (nps > best_nps)
                {
                    best_nps = nps;
                    best_batch = test_batch;
                    best_pipeline = test_pipeline;
                }
            } catch (const std::exception& e) {
                std::cout << "info string [Auto-Tuner] Profiling failed for batch " << test_batch << ": " << e.what() << "\n";
            }
        }

        std::cout << "info string [Auto-Tuner] Phase 1 Complete. Optimal GPU Batch Size = " << best_batch 
                  << " (Peak NPS = " << static_cast<int>(best_nps) << ")\n";
        
        // Phase 2: Lock the optimal GPU batch size, and test scaling CPU threads
        std::cout << "info string [Auto-Tuner] Starting Phase 2: CPU Thread Scaling...\n";
        std::vector<uint8_t> threads_to_test;
        if (threads > 2) threads_to_test.push_back(threads - 2);
        threads_to_test.push_back(threads);
        threads_to_test.push_back(threads + 2);
        
        uint8_t best_threads = threads;

        for (uint8_t test_threads : threads_to_test)
        {
            uint32_t base_pipe = best_batch * DEFAULT_BASE_PIPELINE;
            uint32_t thread_min = test_threads * DEFAULT_THREAD_MINIMUM_PIPELINE_BATCH;
            uint16_t test_pipeline = static_cast<uint16_t>(std::max(base_pipe, thread_min));

            std::cout << "info string [Auto-Tuner] Profiling CPU Threads " << static_cast<int>(test_threads) << "...\n";

            try {
                auto evaluator = std::make_unique<fenrir::NNEvaluator>("onnx/fenrir.onnx", baseline_tuning_parameters.get_gpu_id(0), best_batch);
                fenrir::MCTSSearch search(evaluator.get(), test_threads, test_pipeline);
                fenrir::Engine engine(KIWIPETE);

                auto start = std::chrono::steady_clock::now();
                int nodes_evaluated = search.benchmark_search(engine, 10000);
                auto end = std::chrono::steady_clock::now();
                
                double ms = std::chrono::duration<double, std::milli>(end - start).count();
                double nps = (nodes_evaluated / ms) * 1000.0;

                std::cout << "info string [Auto-Tuner] NPS = " << static_cast<int>(nps) << " (took " << ms << "ms)\n";

                if (nps > best_nps)
                {
                    best_nps = nps;
                    best_threads = test_threads;
                    best_pipeline = test_pipeline;
                }
            } catch (const std::exception& e) {
                std::cout << "info string [Auto-Tuner] Profiling failed for threads " << static_cast<int>(test_threads) << ": " << e.what() << "\n";
            }
        }

        std::cout << "info string [Auto-Tuner] ==================================================\n";
        std::cout << "info string [Auto-Tuner]            FINAL TUNING RESULTS COMPARISON        \n";
        std::cout << "info string [Auto-Tuner] ==================================================\n";
        std::cout << "info string [Auto-Tuner] Parameter     | Baseline Heuristic | Auto-Tuned Peak\n";
        std::cout << "info string [Auto-Tuner] --------------|--------------------|----------------\n";
        std::cout << "info string [Auto-Tuner] Threads       | " << std::left << std::setw(18) << static_cast<int>(threads) << " | " << static_cast<int>(best_threads) << "\n";
        std::cout << "info string [Auto-Tuner] Batch Size    | " << std::left << std::setw(18) << baseline_batch << " | " << best_batch << "\n";
        std::cout << "info string [Auto-Tuner] Pipeline      | " << std::left << std::setw(18) << static_cast<int>(baseline_tuning_parameters.get_pipeline_target()) << " | " << static_cast<int>(best_pipeline) << "\n";
        std::cout << "info string [Auto-Tuner] Peak NPS      | " << std::left << std::setw(18) << "N/A" << " | " << static_cast<int>(best_nps) << "\n";
        std::cout << "info string [Auto-Tuner] ==================================================\n";

        if (best_batch != baseline_batch || best_threads != threads) {
            std::cout << "info string [Auto-Tuner] Hardware heuristic overridden. Using auto-tuned peak.\n";
        } else {
            std::cout << "info string [Auto-Tuner] Hardware heuristic was perfectly accurate.\n";
        }

        // Lock in the new optimized parameters
        real_tuning_parameters.set_batch_size(best_batch);
        real_tuning_parameters.set_search_threads(best_threads);
        real_tuning_parameters.set_pipeline_target(best_pipeline);

        // Write configuration to fenrir.cfg
        std::ofstream cfg("fenrir.cfg");
        if (cfg.is_open())
        {
            cfg << "[Tuning]\n";
            cfg << "SearchThreads=" << static_cast<int>(best_threads) << "\n";
            cfg << "BatchSize=" << best_batch << "\n";
            cfg << "PipelineTarget=" << static_cast<int>(best_pipeline) << "\n";
            cfg << "PeakNPS=" << static_cast<int>(best_nps) << "\n";
            cfg.close();
            std::cout << "info string [Auto-Tuner] Successfully saved configuration to fenrir.cfg\n";
        }
        else
        {
            std::cerr << "info string [Auto-Tuner] Warning: Failed to write fenrir.cfg\n";
        }

        return real_tuning_parameters;
    }
}
