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
#include "include/tuner/utilization.h"
#include "include/mcts/mcts.h"
#include "include/modifier/modifier.h"
#include "include/nn/nn.h"
#include "include/engine/engine.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

namespace tuner
{
    namespace
    {
        /**
         * @brief Formats a utilization percentage for display.
         * @returns "n/a" when the value is unknown (negative).
         */
        std::string format_percent(double value)
        {
            if (value < 0.0)
            {
                return "n/a";
            }
            char buffer[16];
            std::snprintf(buffer, sizeof(buffer), "%.1f%%", value);
            return buffer;
        }
    }

    /**
     * @brief Initialize the auto-tuner with baseline tuning parameters.
     * @param tuning_parameters Reference to tuning parameters to optimize.
     * @param gpu_usage_cap Highest acceptable GPU utilization percent (1-100).
     * @param cpu_usage_cap Highest acceptable process CPU usage percent of
     * the whole machine (1-100).
     */
    AutoTuner::AutoTuner(TuningParameters &tuning_parameters, double gpu_usage_cap_arg, double cpu_usage_cap_arg)
        : baseline_tuning_parameters(tuning_parameters), real_tuning_parameters(tuning_parameters),
          gpu_usage_cap(std::clamp(gpu_usage_cap_arg, 1.0, 100.0)),
          cpu_usage_cap(std::clamp(cpu_usage_cap_arg, 1.0, 100.0)),
          gpu_is_nvidia(false)
    {
        if (auto host_info = tuning_parameters.get_host_info())
        {
            for (const hardware::Gpu &gpu : host_info->get_gpus())
            {
                if (gpu.get_is_nvidia_gpu())
                {
                    gpu_is_nvidia = true;
                    break;
                }
            }
        }
    }

    /**
     * @brief Destructor.
     */
    AutoTuner::~AutoTuner() = default;

    void AutoTuner::print_detected_hardware()
    {
        modifier::Modifier cyan_modifier = modifier::Modifier(modifier::Color::FG_CYAN);
        modifier::Modifier magenta_modifier = modifier::Modifier(modifier::Color::FG_MAGENTA);
        modifier::Modifier reset_modifier = modifier::Modifier(modifier::Color::RESET);
        modifier::Modifier yellow_modifier = modifier::Modifier(modifier::Color::FG_YELLOW);

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

        uint32_t effective_cpu_limit = hardware::get_effective_cpu_limit();
        if (effective_cpu_limit > 0)
        {
            std::cout << magenta_modifier << "Effective CPU limit (cgroup/affinity): " << yellow_modifier << effective_cpu_limit << "\n";
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

    uint16_t AutoTuner::clamped_pipeline_target(uint32_t batch_size, uint8_t search_threads)
    {
        uint32_t base_pipeline_size = batch_size * DEFAULT_BASE_PIPELINE;
        uint32_t thread_minimum_pipeline_size = static_cast<uint32_t>(search_threads) * DEFAULT_THREAD_MINIMUM_PIPELINE_BATCH;
        uint32_t pipeline_target = std::max(base_pipeline_size, thread_minimum_pipeline_size);
        return static_cast<uint16_t>(std::min(pipeline_target, 65535u));
    }

    BenchmarkOutcome AutoTuner::benchmark_config(uint16_t batch_size, uint8_t search_threads,
                                                 uint16_t pipeline_target, uint16_t batch_timeout_ms)
    {
        BenchmarkOutcome outcome;
        try
        {
            auto evaluator = std::make_unique<nn::NN>("onnx/fenrir.onnx", baseline_tuning_parameters.get_gpu_id(0), batch_size, batch_timeout_ms);
            mcts::Tree search(evaluator.get(), search_threads, pipeline_target);
            chess::Engine engine(BENCHMARK_KIWIPETE_FEN);

            /* Untimed warmup so the timed windows measure steady state. */
            search.benchmark_search(engine, BENCHMARK_WARMUP_MS);

            UtilizationMonitor monitor(baseline_tuning_parameters.get_gpu_id(0), gpu_is_nvidia);
            double total_nodes = 0.0;
            double total_ms = 0.0;

            monitor.begin();
            for (int run = 0; run < BENCHMARK_TIMED_RUNS; ++run)
            {
                auto start = std::chrono::steady_clock::now();
                int nodes_evaluated = search.benchmark_search(engine, BENCHMARK_RUN_MS);
                auto end = std::chrono::steady_clock::now();

                total_nodes += static_cast<double>(nodes_evaluated);
                total_ms += std::chrono::duration<double, std::milli>(end - start).count();
            }
            monitor.end();

            if (total_ms > 0.0)
            {
                outcome.nps = (total_nodes / total_ms) * 1000.0;
            }
            outcome.cpu_percent = monitor.cpu_percent();
            outcome.gpu_percent = monitor.gpu_percent();
            outcome.ok = true;

            /* An unmeasurable dimension (-1) cannot be enforced; it counts as
             * compliant and the caveat is announced once up front in run(). */
            outcome.within_caps = (outcome.cpu_percent < 0.0 || outcome.cpu_percent <= cpu_usage_cap) &&
                                  (outcome.gpu_percent < 0.0 || outcome.gpu_percent <= gpu_usage_cap);
        }
        catch (const std::exception &e)
        {
            outcome.error = e.what();
        }
        catch (...)
        {
            outcome.error = "unknown failure";
        }
        return outcome;
    }

    TuningParameters AutoTuner::run()
    {
        print_detected_hardware();

        std::cout << "[Auto-Tuner] Starting FENRIR Hardware Tuning Phase...\n";
        std::cout << "[Auto-Tuner] Usage caps: GPU " << static_cast<int>(gpu_usage_cap)
                  << "%, CPU " << static_cast<int>(cpu_usage_cap) << "%\n";

        const uint8_t baseline_search_threads = baseline_tuning_parameters.get_search_threads();
        const uint16_t baseline_batch_size = baseline_tuning_parameters.get_batch_size();
        const uint16_t baseline_pipeline_target = baseline_tuning_parameters.get_pipeline_target();
        const uint16_t baseline_batch_timeout_ms = baseline_tuning_parameters.get_batch_timeout_ms();

        std::cout << "info string [Auto-Tuner] Baseline configured by hardware: Threads = " << static_cast<int>(baseline_search_threads)
                  << ", Batch Size = " << baseline_batch_size << "\n";

        {
            UtilizationMonitor probe(baseline_tuning_parameters.get_gpu_id(0), gpu_is_nvidia);
            if (gpu_usage_cap < 100.0 && !probe.gpu_available())
            {
                std::cout << "[Auto-Tuner] WARNING: --gpu-usage requested but GPU utilization cannot be "
                             "measured here (non-NVIDIA GPU or NVML unavailable). The GPU cap will NOT "
                             "be enforced; the CPU cap still will be.\n";
            }
        }

        double max_nodes_per_second = 0.0;
        uint16_t best_batch_size = baseline_batch_size;
        uint16_t best_pipeline_target = baseline_pipeline_target;
        uint8_t best_search_threads = baseline_search_threads;
        BenchmarkOutcome best_outcome;
        bool have_compliant = false;

        auto print_benchmark_line = [](const BenchmarkOutcome &outcome) {
            std::cout << "info string [Auto-Tuner]   NPS " << static_cast<int>(outcome.nps)
                      << " | CPU " << format_percent(outcome.cpu_percent)
                      << " | GPU " << format_percent(outcome.gpu_percent)
                      << (outcome.within_caps ? "" : "  [exceeds caps]") << "\n";
        };

        /* Phase 1: GPU batch scaling at the baseline thread count. Larger
         * batches raise GPU utilization, so the first cap breach or
         * plateaued NPS ends the scaling. */
        for (uint32_t current_batch_size = 64; current_batch_size <= 32768; current_batch_size *= 2)
        {
            uint16_t current_pipeline_target = clamped_pipeline_target(current_batch_size, baseline_search_threads);

            std::cout << "info string [Auto-Tuner] Profiling GPU Batch " << current_batch_size << "...\n";
            BenchmarkOutcome outcome = benchmark_config(static_cast<uint16_t>(current_batch_size), baseline_search_threads,
                                                        current_pipeline_target, baseline_batch_timeout_ms);
            if (!outcome.ok)
            {
                std::cout << "info string [Auto-Tuner]   Batch " << current_batch_size
                          << " FAILED (" << outcome.error << ") -- stopping batch scaling\n";
                break;
            }

            print_benchmark_line(outcome);

            if (!outcome.within_caps)
            {
                std::cout << "info string [Auto-Tuner]   exceeds usage caps -- stopping batch scaling\n";
                break;
            }

            if (outcome.nps > max_nodes_per_second * 1.01)
            {
                max_nodes_per_second = outcome.nps;
                best_batch_size = static_cast<uint16_t>(current_batch_size);
                best_pipeline_target = current_pipeline_target;
                best_outcome = outcome;
                have_compliant = true;
                continue;
            }
            break;
        }

        if (!have_compliant)
        {
            /* Even the smallest batch breached the caps at baseline threads;
             * the thread sweep below may still find a compliant config. */
            best_batch_size = 64;
        }

        /* Phase 2: thread sweep at the winning batch size. Thread count is
         * the main CPU-usage lever, so sweep coarse powers of two up to the
         * machine's effective core count (the old +/-2 sweep could never
         * reach a capped target), then refine around the winner. */
        uint32_t max_threads = std::max(1u, std::thread::hardware_concurrency());
        if (auto host_info = baseline_tuning_parameters.get_host_info())
        {
            if (!host_info->get_cpus().empty())
            {
                max_threads = std::max(1u, host_info->get_cpus().at(0).get_logical_cores());
            }
        }
        uint32_t effective_limit = hardware::get_effective_cpu_limit();
        if (effective_limit > 0 && effective_limit < max_threads)
        {
            max_threads = effective_limit;
        }
        max_threads = std::min(max_threads, 255u);

        std::vector<uint8_t> thread_counts_to_test;
        for (uint32_t threads = 1; threads <= max_threads; threads *= 2)
        {
            thread_counts_to_test.push_back(static_cast<uint8_t>(threads));
        }
        thread_counts_to_test.push_back(static_cast<uint8_t>(max_threads));
        if (!have_compliant && baseline_search_threads <= max_threads)
        {
            thread_counts_to_test.push_back(baseline_search_threads);
        }
        std::sort(thread_counts_to_test.begin(), thread_counts_to_test.end());
        thread_counts_to_test.erase(std::unique(thread_counts_to_test.begin(), thread_counts_to_test.end()),
                                    thread_counts_to_test.end());

        std::vector<uint8_t> tested_thread_counts;
        if (have_compliant)
        {
            /* Phase 1's winner already measured this thread count at the
             * winning batch size; don't spend a window re-measuring it. */
            tested_thread_counts.push_back(baseline_search_threads);
        }

        auto benchmark_threads = [&](uint8_t threads) -> BenchmarkOutcome {
            uint16_t current_pipeline_target = clamped_pipeline_target(best_batch_size, threads);
            std::cout << "info string [Auto-Tuner] Profiling " << static_cast<int>(threads) << " search threads...\n";
            BenchmarkOutcome outcome = benchmark_config(best_batch_size, threads, current_pipeline_target, baseline_batch_timeout_ms);
            if (!outcome.ok)
            {
                std::cout << "info string [Auto-Tuner]   " << static_cast<int>(threads)
                          << " threads FAILED (" << outcome.error << ")\n";
                return outcome;
            }
            print_benchmark_line(outcome);
            if (outcome.within_caps && outcome.nps > max_nodes_per_second)
            {
                max_nodes_per_second = outcome.nps;
                best_search_threads = threads;
                best_pipeline_target = current_pipeline_target;
                best_outcome = outcome;
                have_compliant = true;
            }
            return outcome;
        };

        for (uint8_t threads : thread_counts_to_test)
        {
            if (std::find(tested_thread_counts.begin(), tested_thread_counts.end(), threads) != tested_thread_counts.end())
            {
                continue;
            }
            BenchmarkOutcome outcome = benchmark_threads(threads);
            tested_thread_counts.push_back(threads);
            if (outcome.ok && !outcome.within_caps)
            {
                /* More threads only raises usage; nothing above fits. */
                break;
            }
        }

        for (int delta : {-1, +1})
        {
            int refined = static_cast<int>(best_search_threads) + delta;
            if (refined >= 1 && refined <= static_cast<int>(max_threads) &&
                std::find(tested_thread_counts.begin(), tested_thread_counts.end(),
                          static_cast<uint8_t>(refined)) == tested_thread_counts.end())
            {
                benchmark_threads(static_cast<uint8_t>(refined));
                tested_thread_counts.push_back(static_cast<uint8_t>(refined));
            }
        }

        if (!have_compliant)
        {
            best_batch_size = 64;
            best_search_threads = 1;
            best_pipeline_target = clamped_pipeline_target(best_batch_size, best_search_threads);
            std::cout << "[Auto-Tuner] WARNING: no configuration stayed within the requested usage caps "
                         "(GPU " << static_cast<int>(gpu_usage_cap) << "%, CPU " << static_cast<int>(cpu_usage_cap)
                      << "%). Writing the minimal configuration instead -- consider raising the caps.\n";
        }

        /* Phase 3: batch timeout calibration, measured once at the winning batch size */
        uint16_t best_batch_timeout_ms = baseline_batch_timeout_ms;
        try
        {
            auto evaluator = std::make_unique<nn::NN>("onnx/fenrir.onnx", baseline_tuning_parameters.get_gpu_id(0), best_batch_size, baseline_batch_timeout_ms);
            int latency_ms = evaluator->measure_latency_ms();
            best_batch_timeout_ms = static_cast<uint16_t>(std::max(2, latency_ms / 4));
        }
        catch (const std::exception &e)
        {
            std::cout << "info string [Auto-Tuner] Timeout calibration failed (" << e.what()
                      << ") -- keeping baseline " << baseline_batch_timeout_ms << "ms\n";
        }

        /* Games/hour projection: every simulation is at most one NN eval, so
         * NPS / (simulations x plies) bounds complete games per second. Same
         * arithmetic as the FLOPs framing (~122 TFLOPs/game / achieved
         * FLOP/s), just expressed in evals instead of FLOPs. */
        double estimated_games_per_hour = 0.0;
        if (max_nodes_per_second > 0.0)
        {
            estimated_games_per_hour = max_nodes_per_second * 3600.0 /
                                       (static_cast<double>(ESTIMATE_SIMULATIONS_PER_MOVE) * ESTIMATE_AVERAGE_GAME_PLIES);
        }

        std::cout << "[Auto-Tuner] ==================================================\n";
        std::cout << "[Auto-Tuner]            FINAL TUNING RESULTS COMPARISON        \n";
        std::cout << "[Auto-Tuner] ==================================================\n";
        std::cout << "[Auto-Tuner] Parameter     | Baseline Heuristic | Auto-Tuned Peak\n";
        std::cout << "[Auto-Tuner] --------------|--------------------|----------------\n";
        std::cout << "[Auto-Tuner] Threads       | " << std::left << std::setw(18) << static_cast<int>(baseline_search_threads) << " | " << static_cast<int>(best_search_threads) << "\n";
        std::cout << "[Auto-Tuner] Batch Size    | " << std::left << std::setw(18) << baseline_batch_size << " | " << best_batch_size << "\n";
        std::cout << "[Auto-Tuner] Pipeline      | " << std::left << std::setw(18) << static_cast<int>(baseline_pipeline_target) << " | " << static_cast<int>(best_pipeline_target) << "\n";
        std::cout << "[Auto-Tuner] Batch Timeout | " << std::left << std::setw(18) << baseline_batch_timeout_ms << " | " << best_batch_timeout_ms << "\n";
        std::cout << "[Auto-Tuner] Peak NPS      | " << std::left << std::setw(18) << "N/A" << " | " << static_cast<int>(max_nodes_per_second) << "\n";
        std::cout << "[Auto-Tuner] CPU usage     | " << std::left << std::setw(18) << ("cap " + format_percent(cpu_usage_cap)) << " | " << format_percent(best_outcome.cpu_percent) << "\n";
        std::cout << "[Auto-Tuner] GPU usage     | " << std::left << std::setw(18) << ("cap " + format_percent(gpu_usage_cap)) << " | " << format_percent(best_outcome.gpu_percent) << "\n";
        std::cout << "[Auto-Tuner] Est. games/hr | " << std::left << std::setw(18) << "N/A" << " | " << static_cast<int>(estimated_games_per_hour)
                  << "  (at " << ESTIMATE_SIMULATIONS_PER_MOVE << " sims/move)\n";
        std::cout << "[Auto-Tuner] ==================================================\n";

        real_tuning_parameters.set_batch_size(best_batch_size);
        real_tuning_parameters.set_search_threads(best_search_threads);
        real_tuning_parameters.set_pipeline_target(best_pipeline_target);
        real_tuning_parameters.set_batch_timeout_ms(best_batch_timeout_ms);

        std::ofstream cfg("fenrir.cfg");
        if (cfg.is_open())
        {
            cfg << "[Tuning]\n";
            if (!have_compliant)
            {
                cfg << "; WARNING: no configuration met the requested usage caps; minimal fallback written\n";
            }
            cfg << "SearchThreads=" << static_cast<int>(best_search_threads) << "\n";
            cfg << "BatchSize=" << best_batch_size << "\n";
            cfg << "PipelineTarget=" << static_cast<int>(best_pipeline_target) << "\n";
            cfg << "BatchTimeoutMs=" << best_batch_timeout_ms << "\n";
            cfg << "PeakNPS=" << static_cast<int>(max_nodes_per_second) << "\n";
            cfg << "GpuUsageLimit=" << static_cast<int>(gpu_usage_cap) << "\n";
            cfg << "CpuUsageLimit=" << static_cast<int>(cpu_usage_cap) << "\n";
            cfg << "EstimatedGamesPerHour=" << static_cast<int>(estimated_games_per_hour) << "\n";
            cfg.close();
            std::cout << "[Auto-Tuner] Successfully saved configuration to fenrir.cfg\n";
            return real_tuning_parameters;
        }

        std::cout << "Could not write data to cfg file. Please try running the auto-tuner again.";
        return real_tuning_parameters;
    }
}
