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
#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

namespace tuner
{
    struct NvmlUtilizationSample;

    /**
     * @brief Measures this process's CPU usage and (NVIDIA-only) GPU
     * utilization across a benchmark window, so the auto-tuner can enforce
     * --cpu-usage / --gpu-usage caps against measurements instead of guesses.
     *
     * CPU: process CPU-time delta over wall time, reported as a percentage of
     * total machine capacity (all logical cores) -- i.e. what Fenrir itself
     * adds to the machine, independent of other load on the box.
     *
     * GPU: mean of NVML utilization samples polled every 100ms by a
     * background thread. NVML ships with the NVIDIA driver and is loaded
     * dynamically at runtime, so machines without it (AMD, CPU-only) report
     * -1 (unknown) instead of failing to start.
     */
    class UtilizationMonitor
    {
        uint32_t gpu_index;

        void *nvml_library = nullptr;
        void *nvml_device = nullptr;
        int (*nvml_shutdown)() = nullptr;
        int (*nvml_get_utilization)(void *, NvmlUtilizationSample *) = nullptr;

        std::thread sampler;
        std::atomic<bool> sampling{false};
        double gpu_utilization_sum = 0.0;
        uint64_t gpu_sample_count = 0;

        double cpu_seconds_at_begin = -1.0;
        std::chrono::steady_clock::time_point wall_at_begin;

        double cpu_result = -1.0;
        double gpu_result = -1.0;

        void load_nvml();
        void unload_nvml();
        void sample_loop();
        static double process_cpu_seconds();

    public:
        /**
         * @brief Prepares a monitor for one GPU; loads NVML only when the GPU
         * is NVIDIA (there is no NVML on AMD / CPU-only machines).
         * @param gpu_index Device index to watch (matches NVML's ordering).
         * @param gpu_is_nvidia Whether the target GPU is an NVIDIA device.
         */
        UtilizationMonitor(uint32_t gpu_index, bool gpu_is_nvidia);

        /**
         * @brief Stops any active sampling and unloads NVML.
         */
        ~UtilizationMonitor();

        UtilizationMonitor(const UtilizationMonitor &) = delete;
        UtilizationMonitor &operator=(const UtilizationMonitor &) = delete;

        /**
         * @brief Starts a measurement window: snapshots process CPU time and
         * begins GPU sampling (when available).
         */
        void begin();

        /**
         * @brief Ends the measurement window and computes the results
         * readable via cpu_percent() / gpu_percent().
         */
        void end();

        /**
         * @brief Whether GPU utilization can be measured on this machine.
         * @returns True when NVML loaded and resolved the device.
         */
        bool gpu_available() const { return nvml_device != nullptr; }

        /**
         * @brief Process CPU usage over the last begin()/end() window.
         * @returns Percent of total machine CPU capacity, or -1 if unknown.
         */
        double cpu_percent() const { return cpu_result; }

        /**
         * @brief Mean GPU utilization over the last begin()/end() window.
         * @returns Percent, or -1 when GPU measurement is unavailable.
         */
        double gpu_percent() const { return gpu_result; }
    };
}
