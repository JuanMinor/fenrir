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

#include "include/tuner/tuning_parameters.h"
#include <cstdint>
#include <string>

namespace tuner
{
    const std::string BENCHMARK_KIWIPETE_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

    /* Benchmark cadence. benchmark_search() runs for a wall-clock window, so
     * one untimed warmup window absorbs one-time costs (cuDNN autotune,
     * allocator growth) and the timed windows then measure steady state --
     * previously the first config's timed run silently paid those costs. */
    const int BENCHMARK_WARMUP_MS = 2000;
    const int BENCHMARK_RUN_MS = 5000;
    const int BENCHMARK_TIMED_RUNS = 2;

    /* Games/hour projection inputs. 800 simulations/move matches the
     * --selfplay default; 99 plies is the measured average game length over
     * 153,757 self-play games on the training pod. Equivalent FLOPs framing:
     * one game costs simulations x plies x per-eval cost (~122 TFLOPs for
     * the current 10-block net), so games/hour follows directly from NPS. */
    const int ESTIMATE_SIMULATIONS_PER_MOVE = 800;
    const double ESTIMATE_AVERAGE_GAME_PLIES = 99.0;

    /**
     * @brief One benchmarked configuration's measurements and cap verdict.
     */
    struct BenchmarkOutcome
    {
        double nps = 0.0;
        double cpu_percent = -1.0;
        double gpu_percent = -1.0;
        bool ok = false;
        bool within_caps = false;
        std::string error;
    };

    class AutoTuner
    {
        TuningParameters baseline_tuning_parameters;
        TuningParameters real_tuning_parameters;
        double gpu_usage_cap;
        double cpu_usage_cap;
        bool gpu_is_nvidia;

        void print_detected_hardware();

        /**
         * @brief Benchmarks one configuration: warmup window, then timed
         * windows with utilization measured across them.
         * @returns Measurements, cap compliance, and any failure reason.
         */
        BenchmarkOutcome benchmark_config(uint16_t batch_size, uint8_t search_threads,
                                          uint16_t pipeline_target, uint16_t batch_timeout_ms);

        /**
         * @brief Pipeline target for a batch/thread pair, clamped so large
         * batches cannot overflow uint16_t (32768 * 2 used to wrap to 0).
         * @returns The clamped pipeline target.
         */
        static uint16_t clamped_pipeline_target(uint32_t batch_size, uint8_t search_threads);

    public:
        /**
         * @brief Initialize the auto-tuner with baseline tuning parameters.
         * @param tuning_parameters Reference to tuning parameters to optimize.
         * @param gpu_usage_cap Highest acceptable GPU utilization percent (1-100).
         * @param cpu_usage_cap Highest acceptable process CPU usage percent of
         * the whole machine (1-100).
         */
        AutoTuner(TuningParameters &tuning_parameters, double gpu_usage_cap = 100.0, double cpu_usage_cap = 100.0);

        /**
         * @brief Destructor.
         */
        ~AutoTuner();

        /**
         * @brief Run the auto-tuning process to find optimal parameters.
         * @returns Optimized TuningParameters based on hardware benchmarks. The
         * results describe a single-process, single-GPU worker: one Fenrir
         * process drives exactly one ONNX session on one device. Configs whose
         * measured usage exceeds the requested caps are rejected, so the
         * result is the fastest configuration that stays inside the caps.
         */
        TuningParameters run();
    };
}
