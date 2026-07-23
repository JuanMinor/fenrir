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

#include "include/tuner/utilization.h"

#include <algorithm>

#ifdef _WIN32
/* Without NOMINMAX, windows.h #defines max/min as macros, which mangles
 * every std::max/std::min call in this file into a syntax error (MSVC
 * C2589/C2059) since the preprocessor doesn't understand std:: scoping --
 * it just sees the bare identifier "max" followed by "(" and expands it.
 * nn.cpp hit the same class of windows.h macro-pollution problem with
 * wingdi.h's ERROR macro (see its NOGDI comment). */
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#include <sys/resource.h>
#endif

namespace tuner
{
    /* Mirror of nvmlUtilization_t: percent of time over the past sample
     * period during which the GPU was busy / memory was read or written. */
    struct NvmlUtilizationSample
    {
        unsigned int gpu;
        unsigned int memory;
    };

    namespace
    {
        using NvmlInitFn = int (*)();
        using NvmlShutdownFn = int (*)();
        using NvmlGetHandleFn = int (*)(unsigned int, void **);
        using NvmlGetUtilizationFn = int (*)(void *, NvmlUtilizationSample *);

        void *load_library()
        {
#ifdef _WIN32
            return reinterpret_cast<void *>(LoadLibraryA("nvml.dll"));
#else
            void *handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
            if (handle == nullptr)
            {
                handle = dlopen("libnvidia-ml.so", RTLD_LAZY);
            }
            return handle;
#endif
        }

        void *load_symbol(void *library, const char *name)
        {
#ifdef _WIN32
            return reinterpret_cast<void *>(GetProcAddress(reinterpret_cast<HMODULE>(library), name));
#else
            return dlsym(library, name);
#endif
        }

        void close_library(void *library)
        {
#ifdef _WIN32
            FreeLibrary(reinterpret_cast<HMODULE>(library));
#else
            dlclose(library);
#endif
        }
    }

    UtilizationMonitor::UtilizationMonitor(uint32_t gpu_index_arg, bool gpu_is_nvidia)
        : gpu_index(gpu_index_arg)
    {
        if (gpu_is_nvidia)
        {
            load_nvml();
        }
    }

    UtilizationMonitor::~UtilizationMonitor()
    {
        if (sampling.load())
        {
            end();
        }
        unload_nvml();
    }

    /**
     * @brief Dynamically loads NVML and resolves the target device. Any
     * failure (no driver, symbol missing, bad index) leaves the monitor in
     * the "GPU unknown" state rather than raising an error.
     */
    void UtilizationMonitor::load_nvml()
    {
        nvml_library = load_library();
        if (nvml_library == nullptr)
        {
            return;
        }

        auto init = reinterpret_cast<NvmlInitFn>(load_symbol(nvml_library, "nvmlInit_v2"));
        auto get_handle = reinterpret_cast<NvmlGetHandleFn>(load_symbol(nvml_library, "nvmlDeviceGetHandleByIndex_v2"));
        auto get_utilization = reinterpret_cast<NvmlGetUtilizationFn>(load_symbol(nvml_library, "nvmlDeviceGetUtilizationRates"));
        auto shutdown_fn = reinterpret_cast<NvmlShutdownFn>(load_symbol(nvml_library, "nvmlShutdown"));

        bool ready = init != nullptr && get_handle != nullptr &&
                     get_utilization != nullptr && shutdown_fn != nullptr && init() == 0;

        if (ready && (get_handle(gpu_index, &nvml_device) != 0 || nvml_device == nullptr))
        {
            shutdown_fn();
            ready = false;
        }

        if (!ready)
        {
            close_library(nvml_library);
            nvml_library = nullptr;
            nvml_device = nullptr;
            return;
        }

        nvml_shutdown = shutdown_fn;
        nvml_get_utilization = get_utilization;
    }

    void UtilizationMonitor::unload_nvml()
    {
        if (nvml_shutdown != nullptr)
        {
            nvml_shutdown();
            nvml_shutdown = nullptr;
        }
        if (nvml_library != nullptr)
        {
            close_library(nvml_library);
            nvml_library = nullptr;
        }
        nvml_device = nullptr;
        nvml_get_utilization = nullptr;
    }

    /**
     * @brief Background loop: polls NVML every 100ms while the window is
     * open. Only this thread touches the sum/count fields between begin()
     * and the join in end(), so no further synchronization is needed.
     */
    void UtilizationMonitor::sample_loop()
    {
        while (sampling.load())
        {
            NvmlUtilizationSample sample{};
            if (nvml_get_utilization(nvml_device, &sample) == 0)
            {
                gpu_utilization_sum += static_cast<double>(sample.gpu);
                gpu_sample_count++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    /**
     * @brief Total CPU time (user + system) this process has consumed.
     * @returns Seconds, or -1 when the platform query fails.
     */
    double UtilizationMonitor::process_cpu_seconds()
    {
#ifdef _WIN32
        FILETIME creation_time;
        FILETIME exit_time;
        FILETIME kernel_time;
        FILETIME user_time;
        if (GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time) == 0)
        {
            return -1.0;
        }
        auto to_seconds = [](const FILETIME &filetime) {
            ULARGE_INTEGER value;
            value.LowPart = filetime.dwLowDateTime;
            value.HighPart = filetime.dwHighDateTime;
            return static_cast<double>(value.QuadPart) * 1e-7;
        };
        return to_seconds(kernel_time) + to_seconds(user_time);
#else
        rusage usage{};
        if (getrusage(RUSAGE_SELF, &usage) != 0)
        {
            return -1.0;
        }
        auto to_seconds = [](const timeval &tv) {
            return static_cast<double>(tv.tv_sec) + static_cast<double>(tv.tv_usec) * 1e-6;
        };
        return to_seconds(usage.ru_utime) + to_seconds(usage.ru_stime);
#endif
    }

    void UtilizationMonitor::begin()
    {
        cpu_result = -1.0;
        gpu_result = -1.0;
        gpu_utilization_sum = 0.0;
        gpu_sample_count = 0;

        cpu_seconds_at_begin = process_cpu_seconds();
        wall_at_begin = std::chrono::steady_clock::now();

        if (nvml_device != nullptr)
        {
            sampling.store(true);
            sampler = std::thread(&UtilizationMonitor::sample_loop, this);
        }
    }

    void UtilizationMonitor::end()
    {
        double wall_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - wall_at_begin).count();

        if (sampling.load())
        {
            sampling.store(false);
            if (sampler.joinable())
            {
                sampler.join();
            }
        }

        double cpu_seconds_now = process_cpu_seconds();
        if (cpu_seconds_at_begin >= 0.0 && cpu_seconds_now >= 0.0 && wall_seconds > 0.0)
        {
            double capacity = static_cast<double>(std::max(1u, std::thread::hardware_concurrency()));
            cpu_result = 100.0 * (cpu_seconds_now - cpu_seconds_at_begin) / (wall_seconds * capacity);
        }

        if (gpu_sample_count > 0)
        {
            gpu_result = gpu_utilization_sum / static_cast<double>(gpu_sample_count);
        }
    }
}
