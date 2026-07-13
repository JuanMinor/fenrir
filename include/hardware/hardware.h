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
#include <string>
#include <vector>

namespace hardware
{

    constexpr uint64_t BYTES_PER_GB = 1024ULL * 1024ULL * 1024ULL;

    enum class CpuType
    {
        AMD,
        INTEL
    };

    enum class GpuType
    {
        AMD,
        NVIDIA
    };

    enum class OperatingSystemType
    {
        LINUX,
        MACOS,
        WINDOWS
    };

    /**
     * @brief Represents a Central Processing Unit (CPU) in the system.
     */
    class Cpu
    {
        CpuType cpu_type;
        uint32_t logical_cores;
        std::string model_name;
        uint32_t physical_cores;

        /**
         * @brief Private helper to classify the CPU model vendor type (AMD or INTEL).
         * @param model_name The processor model name.
         */
        void set_cpu_type(const std::string &model_name);

    public:
        /**
         * @brief Constructs a CPU representation.
         * @param logical_cores Number of logical processor cores.
         * @param model_name The brand or model name of the CPU.
         * @param physical_cores Number of physical processor cores.
         */
        Cpu(uint32_t logical_cores, const std::string &model_name, uint32_t physical_cores);

        /**
         * @brief Destructor for Cpu.
         */
        ~Cpu();

        /**
         * @brief Retrieves the processor vendor type (AMD/Intel).
         * @returns The CpuType.
         */
        CpuType get_cpu_type() const;

        /**
         * @brief Gets the number of logical CPU cores.
         * @returns Logical core count.
         */
        uint32_t get_logical_cores() const;

        /**
         * @brief Gets the CPU model or brand name string.
         * @returns The CPU model string.
         */
        std::string get_model_name() const;

        /**
         * @brief Gets the number of physical CPU cores.
         * @returns Physical core count.
         */
        uint32_t get_physical_cores() const;
    };

    /**
     * @brief Represents a graphics processing unit (GPU).
     */
    class Gpu
    {
        int device_id;
        GpuType gpu_type;
        bool is_amd_gpu;
        bool is_nvidia_gpu;
        std::string model_name;
        uint64_t total_memory_in_bytes;
        uint64_t vram_size_in_bytes;

        /**
         * @brief Classifies the GPU vendor type based on model descriptor name.
         * @param model_name The model descriptor string.
         */
        void set_gpu_type(const std::string &model_name);

    public:
        /**
         * @brief Constructs a GPU representation.
         * @param device_id System index of the GPU device.
         * @param model_name Brand or model name of the GPU.
         * @param total_memory_in_bytes Total physical system memory associated with GPU context in bytes.
         * @param vram_size_in_bytes Dedicated video RAM size in bytes.
         */
        Gpu(int device_id, const std::string &model_name, uint64_t total_memory_in_bytes, uint64_t vram_size_in_bytes);

        /**
         * @brief Destructor for Gpu.
         */
        ~Gpu();

        /**
         * @brief Gets the GPU device ID.
         * @returns Integer device identifier.
         */
        int get_device_id() const;

        /**
         * @brief Gets the GPU vendor type classification.
         * @returns GpuType enum (AMD or NVIDIA).
         */
        GpuType get_gpu_type() const;

        /**
         * @brief Returns whether the GPU vendor is AMD.
         * @returns True if AMD, false otherwise.
         */
        bool get_is_amd_gpu() const;

        /**
         * @brief Returns whether the GPU vendor is NVIDIA.
         * @returns True if NVIDIA, false otherwise.
         */
        bool get_is_nvidia_gpu() const;

        /**
         * @brief Gets the GPU model or brand name string.
         * @returns Model descriptor string.
         */
        std::string get_model_name() const;

        /**
         * @brief Gets the total physical memory capacity of the GPU in bytes.
         * @returns Total memory bytes.
         */
        uint64_t get_total_memory_in_bytes() const;

        /**
         * @brief Gets the dedicated VRAM size of the GPU in bytes.
         * @returns VRAM size in bytes.
         */
        uint64_t get_vram_size_in_bytes() const;
    };

    /**
     * @brief Represents system Random Access Memory (RAM).
     */
    class Ram
    {
        uint64_t total_size_in_bytes;

    public:
        /**
         * @brief Constructs a RAM representation with specified total physical size.
         * @param total_size_in_bytes Total RAM size in bytes.
         */
        Ram(uint64_t total_size_in_bytes);

        /**
         * @brief Destructor for Ram.
         */
        ~Ram();

        /**
         * @brief Gets the total size of RAM in bytes.
         * @returns Memory capacity in bytes.
         */
        uint64_t get_total_size_in_bytes() const;
    };

    /**
     * @brief Represents the Operating System environment.
     */
    class OperatingSystem
    {
        bool is_64_bit;
        bool is_windows;
        std::string name;
        OperatingSystemType os_type;

    public:
        /**
         * @brief Constructs an OperatingSystem instance.
         * @param name The OS name string.
         */
        OperatingSystem(const std::string &name);

        /**
         * @brief Destructor for OperatingSystem.
         */
        ~OperatingSystem();

        /**
         * @brief Checks if the OS environment is 64-bit.
         * @returns True if 64-bit system, false otherwise.
         */
        bool get_is_64_bit() const;

        /**
         * @brief Checks if the OS is Windows.
         * @returns True if Windows, false otherwise.
         */
        bool get_is_windows() const;

        /**
         * @brief Gets the operating system name.
         * @returns OS name string.
         */
        std::string get_name() const;

        /**
         * @brief Gets the OS family category.
         * @returns OperatingSystemType enum.
         */
        OperatingSystemType get_os_type() const;
    };

    /**
     * @brief Contains detailed hardware and OS profile information of the host system.
     */
    class HostInfo
    {
        std::vector<Cpu> cpus;
        std::vector<Gpu> gpus;
        OperatingSystem os;
        Ram ram;

    public:
        /**
         * @brief Constructs HostInfo by passing already-detected hardware component objects.
         * @param cpus A vector of Cpu objects.
         * @param gpus A vector of Gpu objects.
         * @param ram The Ram object.
         * @param os The OperatingSystem object.
         */
        HostInfo(const std::vector<Cpu> &cpus, const std::vector<Gpu> &gpus, const Ram &ram, const OperatingSystem &os);

        /**
         * @brief Default constructor for HostInfo.
         */
        HostInfo();

        /**
         * @brief Destructor for HostInfo.
         */
        ~HostInfo();

        /**
         * @brief Gets the list of detected CPUs.
         * @returns A vector of Cpu objects.
         */
        std::vector<Cpu> get_cpus() const;

        /**
         * @brief Gets the list of detected GPUs.
         * @returns A vector of Gpu objects.
         */
        std::vector<Gpu> get_gpus() const;

        /**
         * @brief Gets the operating system hardware information.
         * @returns The OperatingSystem object.
         */
        OperatingSystem get_os() const;

        /**
         * @brief Gets the system RAM hardware information.
         * @returns The Ram object.
         */
        Ram get_ram() const;
    };

    /**
     * @brief Utility to convert raw memory bytes into Gigabytes (GB).
     * @param bytes Number of bytes to convert.
     * @returns Capacity rounded up in gigabytes.
     */
    uint32_t convert_bytes_to_gb(uint64_t bytes);

    /**
     * @brief Automatically queries system APIs to discover host CPU, GPU, RAM, and OS parameters.
     * @returns A fully populated HostInfo object.
     */
    HostInfo detect_host_info();
}
