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

#include "include/constants/tuner.h"
#include <algorithm>

namespace constants
{
    namespace tuner
    {
        TuningParameters::TuningParameters()
            : host_info(hardware::detect_host_info())
        {
            gpu_count = static_cast<uint8_t>(host_info.get_gpus().size());
            search_threads = calculate_search_threads();
            batch_size = calculate_batch_size();
            pipeline_target = calculate_pipeline_target();
        }

        TuningParameters::~TuningParameters() = default;

        uint8_t TuningParameters::calculate_search_threads()
        {
            uint8_t cores = DEFAULT_CPU_CORES;
            if (!host_info.get_cpus().empty())
            {
                cores = host_info.get_cpus().at(0).get_logical_cores();
            }

            if (cores > 4)
            {
                return cores - 2;
            }
            else if (cores > 1)
            {
                return cores - 1;
            }
            return 1;
        }

        uint16_t TuningParameters::calculate_batch_size()
        {
            if (host_info.get_gpus().empty())
            {
                return DEFAULT_CPU_BATCH_SIZE;
            }

            uint64_t total_vram = 0;
            for (const auto &gpu : host_info.get_gpus())
            {
                total_vram += gpu.get_vram_size_in_bytes();
            }
            uint64_t avg_vram = total_vram / gpu_count;
            uint32_t avg_vram_gb = hardware::convert_bytes_to_gb(avg_vram);

            uint32_t batch_per_gpu = 256;
            if (avg_vram_gb >= 16)
            {
                batch_per_gpu = 1024;
            }
            else if (avg_vram_gb >= 8)
            {
                batch_per_gpu = 512;
            }

            uint32_t gpu_batch = static_cast<uint32_t>(batch_per_gpu * gpu_count);

            uint32_t search_thread_batch = search_threads * DEFAULT_CPU_BATCH_SIZE;

            return static_cast<uint16_t>(std::min(gpu_batch, std::max(static_cast<uint32_t>(DEFAULT_CPU_BATCH_SIZE), search_thread_batch)));
        }

        uint16_t TuningParameters::calculate_pipeline_target()
        {
            uint32_t base_pipeline = batch_size * DEFAULT_BASE_PIPELINE;

            uint32_t thread_minimum = search_threads * DEFAULT_THREAD_MINIMUM_PIPELINE_BATCH;

            return static_cast<uint16_t>(std::max(base_pipeline, thread_minimum));
        }

        uint8_t TuningParameters::get_gpu_id(uint8_t index) const
        {
            if (index < host_info.get_gpus().size())
            {
                return static_cast<uint8_t>(host_info.get_gpus().at(index).get_device_id());
            }
            return 0;
        }
    }
}
