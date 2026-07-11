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
    const uint8_t DEFAULT_CPU_BATCH_SIZE = 48;
    const uint8_t DEFAULT_CPU_CORES = 2;
    const uint8_t DEFAULT_BASE_PIPELINE = 2;
    const uint8_t DEFAULT_THREAD_MINIMUM_PIPELINE_BATCH = 4;

    class TuningParameters
    {
        uint16_t batch_size;
        uint8_t gpu_count;
        std::optional<hardware::HostInfo> host_info;
        uint16_t pipeline_target;
        uint8_t search_threads;

        uint8_t calculate_search_threads();
        uint16_t calculate_batch_size();
        uint16_t calculate_pipeline_target();

    public:
        TuningParameters(bool load_from_file = true);
        ~TuningParameters();

        inline uint16_t get_batch_size() const { return batch_size; }
        inline uint16_t get_pipeline_target() const { return pipeline_target; }
        inline uint8_t get_search_threads() const { return search_threads; }

        inline void set_batch_size(uint16_t size) { batch_size = size; }
        inline void set_pipeline_target(uint16_t target) { pipeline_target = target; }
        inline void set_search_threads(uint8_t threads) { search_threads = threads; }

        uint8_t get_gpu_id(uint8_t index) const;
    };
}
