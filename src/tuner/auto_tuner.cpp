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
#include "include/modifier/modifier.h"
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

    void AutoTuner::print_detected_hardware()
    {
        color::Modifier cyan_modifier = color::Modifier(color::Color::FG_CYAN);
        color::Modifier magenta_modifier = color::Modifier(color::Color::FG_MAGENTA);
        color::Modifier reset_modifier = color::Modifier(color::Color::RESET);
        color::Modifier yellow_modifier = color::Modifier(color::Color::FG_YELLOW);

        hardware::HostInfo host_info;

        try
        {
            host_info = baseline_tuning_parameters.get_host_info().value();
        }
        catch (const std::bad_optional_access &)
        {
            return;
        }

        std::vector<hardware::Cpu> cpus = host_info.get_cpus();
        std::vector<hardware::Gpu> gpus = host_info.get_gpus();
        hardware::Ram ram = host_info.get_ram();
        hardware::OperatingSystem operating_system = host_info.get_os();

        std::cout << cyan_modifier << "Fenrir found this hardware and operating sytem on your machine:\n";

        /* Cpu */
        for (hardware::Cpu cpu : cpus)
        {
            std::cout << magenta_modifier << "CPU model: " << yellow_modifier << cpu.get_model_name() << "\n";
            std::cout << magenta_modifier << "CPU physical cores: " << yellow_modifier << cpu.get_physical_cores() << "\n";
            std::cout << magenta_modifier << "CPU logical cores: " << yellow_modifier << cpu.get_logical_cores() << "\n";
        }

        /* Gpu */
        for (hardware::Gpu gpu : gpus)
        {
            std::cout << magenta_modifier << "GPU model: " << yellow_modifier << gpu.get_model_name() << "\n";
            std::cout << magenta_modifier << "GPU VRAM: " << yellow_modifier << hardware::convert_bytes_to_gb(gpu.get_vram_size_in_bytes()) << " GB\n";
        }

        /* Ram */
        std::cout << magenta_modifier << "System RAM: " << yellow_modifier << hardware::convert_bytes_to_gb(ram.get_total_size_in_bytes()) << " GB\n";

        /* OS */
        std::cout << magenta_modifier << "Operating System: " << yellow_modifier << operating_system.get_name() << "\n";
        std::cout << reset_modifier;
    }

    TuningParameters AutoTuner::run()
    {
        print_detected_hardware();

        std::cout << "[Auto-Tuner] Starting FENRIR Hardware Tuning Phase...\n";

        const uint8_t baseline_search_threads = baseline_tuning_parameters.get_search_threads();
        const uint16_t baseline_batch_size = baseline_tuning_parameters.get_batch_size();
        const uint16_t baseline_pipeline_target = baseline_tuning_parameters.get_pipeline_target();

        std::cout << "info string [Auto-Tuner] Baseline configured by hardware: Threads = " << static_cast<int>(baseline_search_threads)
                  << ", Batch Size = " << baseline_batch_size << "\n";

        double max_nodes_per_second = 0.0;
        uint16_t best_batch_size = baseline_batch_size;
        uint16_t best_pipeline_target = baseline_pipeline_target;

        uint32_t current_batch_size = 64;
        bool keep_scaling = true;

        /* Phase 1: GPU benchmark */
        while (keep_scaling)
        {
            uint32_t base_pipeline_size = current_batch_size * DEFAULT_BASE_PIPELINE;
            uint32_t thread_minimum_pipeline_size = baseline_search_threads * DEFAULT_THREAD_MINIMUM_PIPELINE_BATCH;
            uint16_t current_pipeline_target = static_cast<uint16_t>(std::max(base_pipeline_size, thread_minimum_pipeline_size));

            std::cout << "info string [Auto-Tuner] Profiling GPU Batch " << current_batch_size << "...\n";

            try
            {
                auto evaluator = std::make_unique<fenrir::NNEvaluator>("onnx/fenrir.onnx", baseline_tuning_parameters.get_gpu_id(0), current_batch_size);
                fenrir::MCTSSearch search(evaluator.get(), baseline_search_threads, current_pipeline_target);
                fenrir::Engine engine(BENCHMARK_KIWIPETE_FEN);

                /* Run test */
                auto start = std::chrono::steady_clock::now();
                int nodes_evaluated = search.benchmark_search(engine, 10000);
                auto end = std::chrono::steady_clock::now();

                double duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
                double current_nps = (nodes_evaluated / duration_ms) * 1000.0;

                /* Measure performance improvement > 1% */
                if (current_nps > (max_nodes_per_second * 1.01))
                {
                    max_nodes_per_second = current_nps;
                    best_batch_size = static_cast<uint16_t>(current_batch_size);
                    best_pipeline_target = current_pipeline_target;

                    current_batch_size *= 2;

                    if (current_batch_size > 32768)
                    {
                        keep_scaling = false;
                    }
                    continue;
                }
                keep_scaling = false;
            }
            catch (const std::exception &)
            {
                keep_scaling = false;
            }
        }

        /* Phase 2: CPU benchmark */
        std::vector<uint8_t> thread_counts_to_test;
        if (baseline_search_threads > 2)
        {
            thread_counts_to_test.push_back(baseline_search_threads - 2);
        }
        thread_counts_to_test.push_back(baseline_search_threads);
        thread_counts_to_test.push_back(baseline_search_threads + 2);

        uint8_t best_search_threads = baseline_search_threads;

        for (uint8_t current_search_threads : thread_counts_to_test)
        {
            uint32_t base_pipeline_size = best_batch_size * DEFAULT_BASE_PIPELINE;
            uint32_t thread_minimum_pipeline_size = current_search_threads * DEFAULT_THREAD_MINIMUM_PIPELINE_BATCH;
            uint16_t current_pipeline_target = static_cast<uint16_t>(std::max(base_pipeline_size, thread_minimum_pipeline_size));

            try
            {
                auto evaluator = std::make_unique<fenrir::NNEvaluator>("onnx/fenrir.onnx", baseline_tuning_parameters.get_gpu_id(0), best_batch_size);
                fenrir::MCTSSearch search(evaluator.get(), current_search_threads, current_pipeline_target);
                fenrir::Engine engine(BENCHMARK_KIWIPETE_FEN);

                auto start = std::chrono::steady_clock::now();
                int nodes_evaluated = search.benchmark_search(engine, 10000);
                auto end = std::chrono::steady_clock::now();

                double duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
                double current_nps = (nodes_evaluated / duration_ms) * 1000.0;

                if (current_nps > max_nodes_per_second)
                {
                    max_nodes_per_second = current_nps;
                    best_search_threads = current_search_threads;
                    best_pipeline_target = current_pipeline_target;
                }
            }
            catch (const std::exception &)
            {
                continue;
            }
        }

        std::cout << "[Auto-Tuner] ==================================================\n";
        std::cout << "[Auto-Tuner]            FINAL TUNING RESULTS COMPARISON        \n";
        std::cout << "[Auto-Tuner] ==================================================\n";
        std::cout << "[Auto-Tuner] Parameter     | Baseline Heuristic | Auto-Tuned Peak\n";
        std::cout << "[Auto-Tuner] --------------|--------------------|----------------\n";
        std::cout << "[Auto-Tuner] Threads       | " << std::left << std::setw(18) << static_cast<int>(baseline_search_threads) << " | " << static_cast<int>(best_search_threads) << "\n";
        std::cout << "[Auto-Tuner] Batch Size    | " << std::left << std::setw(18) << baseline_batch_size << " | " << best_batch_size << "\n";
        std::cout << "[Auto-Tuner] Pipeline      | " << std::left << std::setw(18) << static_cast<int>(baseline_pipeline_target) << " | " << static_cast<int>(best_pipeline_target) << "\n";
        std::cout << "[Auto-Tuner] Peak NPS      | " << std::left << std::setw(18) << "N/A" << " | " << static_cast<int>(max_nodes_per_second) << "\n";
        std::cout << "[Auto-Tuner] ==================================================\n";

        // Scale total CPU threads to saturate all available GPUs
        uint8_t system_gpus = baseline_tuning_parameters.get_gpu_count();
        if (system_gpus == 0)
        {
            system_gpus = 1;
        }

        uint8_t final_search_threads = best_search_threads * system_gpus;

        if (final_search_threads < best_search_threads)
        {
            final_search_threads = 255;
        }

        real_tuning_parameters.set_batch_size(best_batch_size);
        real_tuning_parameters.set_search_threads(final_search_threads);
        real_tuning_parameters.set_pipeline_target(best_pipeline_target);

        std::ofstream cfg("fenrir.cfg");
        if (cfg.is_open())
        {
            cfg << "[Tuning]\n";
            cfg << "SearchThreads=" << static_cast<int>(final_search_threads) << "\n";
            cfg << "BatchSize=" << best_batch_size << "\n";
            cfg << "PipelineTarget=" << static_cast<int>(best_pipeline_target) << "\n";
            cfg << "PeakNPS=" << static_cast<int>(max_nodes_per_second) << "\n";
            cfg.close();
            std::cout << "[Auto-Tuner] Successfully saved configuration to fenrir.cfg\n";
            return real_tuning_parameters;
        }

        std::cout << "Could not write data to cfg file. Please try running the auto-tuner again.";
        return real_tuning_parameters;
    }
}
