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
#include <cstdint>
#include <optional>
#include "include/hardware/hardware.h"

namespace tuner
{
    const uint8_t DEFAULT_BASE_PIPELINE = 2;
    const uint8_t DEFAULT_CPU_BATCH_SIZE = 48;
    const uint8_t DEFAULT_CPU_CORES = 2;
    const uint8_t DEFAULT_THREAD_MINIMUM_PIPELINE_BATCH = 4;

    class TuningParameters
    {
        uint16_t batch_size;
        uint8_t gpu_count;
        std::optional<hardware::HostInfo> host_info;
        uint16_t pipeline_target;
        uint8_t search_threads;

        /**
         * @brief Computes optimal inference batch size based on available GPU count and VRAM.
         * @returns The calculated batch size.
         */
        uint16_t calculate_batch_size();

        /**
         * @brief Computes optimal pipeline size based on batch size and search threads.
         * @returns The calculated pipeline target.
         */
        uint16_t calculate_pipeline_target();

        /**
         * @brief Computes optimal number of search threads based on logical CPU cores.
         * @returns The calculated number of threads.
         */
        uint8_t calculate_search_threads();

    public:
        /**
         * @brief Constructs TuningParameters and optionally loads search parameters from configuration file.
         * @param load_from_file If true, the constructor attempts to parse fenrir.cfg for custom parameters.
         */
        TuningParameters(bool load_from_file = true);

        /**
         * @brief Destructs the TuningParameters object.
         */
        ~TuningParameters();

        /**
         * @brief Gets the batch size search parameter.
         * @returns The batch size.
         */
        inline uint16_t get_batch_size() const { return batch_size; }

        /**
         * @brief Gets the number of detected neural-network execution GPUs.
         * @returns The number of GPUs.
         */
        inline uint8_t get_gpu_count() const { return gpu_count; }

        /**
         * @brief Translates a logical GPU index to its physical hardware Device ID.
         * @param index The 0-based logical index in the detected list.
         * @returns The underlying physical device identifier.
         */
        uint8_t get_gpu_id(uint8_t index) const;

        /**
         * @brief Gets the cached hardware host information (if detected during initialization).
         * @returns Optional containing HostInfo if query was performed.
         */
        inline std::optional<hardware::HostInfo> get_host_info() const { return host_info; }

        /**
         * @brief Gets the pipeline queue target parameter.
         * @returns The pipeline target.
         */
        inline uint16_t get_pipeline_target() const { return pipeline_target; }

        /**
         * @brief Gets the active search threads count.
         * @returns Number of threads.
         */
        inline uint8_t get_search_threads() const { return search_threads; }

        /**
         * @brief Updates the active evaluation batch size manually.
         * @param size The new batch size value.
         */
        inline void set_batch_size(uint16_t size) { batch_size = size; }

        /**
         * @brief Updates the active execution pipeline limit manually.
         * @param target The new pipeline limit value.
         */
        inline void set_pipeline_target(uint16_t target) { pipeline_target = target; }

        /**
         * @brief Updates the active search thread count manually.
         * @param threads The new thread count value.
         */
        inline void set_search_threads(uint8_t threads) { search_threads = threads; }
    };
}
