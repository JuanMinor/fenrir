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

#include "include/hardware/hardware.h"
#include <thread>
#include <iostream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")
#elif defined(__linux__)
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <fstream>
#include <cstdio>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

namespace hardware
{
    /**
     * @brief Constructs a CPU representation.
     * @param logical_cores Number of logical processor cores.
     * @param model_name The brand or model name of the CPU.
     * @param physical_cores Number of physical processor cores.
     */
    Cpu::Cpu(uint32_t logical_cores, const std::string &model_name, uint32_t physical_cores)
        : logical_cores(logical_cores), model_name(model_name), physical_cores(physical_cores)
    {
        set_cpu_type(model_name);
    }

    /**
     * @brief Destructor for Cpu.
     */
    Cpu::~Cpu() = default;

    /**
     * @brief Retrieves the processor vendor type (AMD/Intel).
     * @returns The CpuType.
     */
    CpuType Cpu::get_cpu_type() const { return cpu_type; }

    /**
     * @brief Gets the number of logical CPU cores.
     * @returns Logical core count.
     */
    uint32_t Cpu::get_logical_cores() const { return logical_cores; }

    /**
     * @brief Gets the CPU model or brand name string.
     * @returns The CPU model string.
     */
    std::string Cpu::get_model_name() const { return model_name; }

    /**
     * @brief Gets the number of physical CPU cores.
     * @returns Physical core count.
     */
    uint32_t Cpu::get_physical_cores() const { return physical_cores; }

    /**
     * @brief Private helper to classify the CPU model vendor type (AMD or INTEL).
     * @param model The processor model name.
     */
    void Cpu::set_cpu_type(const std::string &model)
    {
        std::string lower_model = model;
        std::transform(lower_model.begin(), lower_model.end(), lower_model.begin(), [](unsigned char c)
                       { return static_cast<char>(::tolower(c)); });

        if (lower_model.find("amd") != std::string::npos || lower_model.find("ryzen") != std::string::npos || lower_model.find("epyc") != std::string::npos)
        {
            cpu_type = CpuType::AMD;
            return;
        }
        cpu_type = CpuType::INTEL;
    }

    /**
     * @brief Constructs a GPU representation.
     * @param device_id System index of the GPU device.
     * @param model_name Brand or model name of the GPU.
     * @param total_memory_in_bytes Total physical system memory associated with GPU context in bytes.
     * @param vram_size_in_bytes Dedicated video RAM size in bytes.
     */
    Gpu::Gpu(int device_id, const std::string &model_name, uint64_t total_memory_in_bytes, uint64_t vram_size_in_bytes)
        : device_id(device_id), gpu_type(GpuType::AMD), is_amd_gpu(false), is_nvidia_gpu(false), model_name(model_name), total_memory_in_bytes(total_memory_in_bytes), vram_size_in_bytes(vram_size_in_bytes)
    {
        set_gpu_type(model_name);
    }

    /**
     * @brief Destructor for Gpu.
     */
    Gpu::~Gpu() = default;

    /**
     * @brief Gets the GPU device ID.
     * @returns Integer device identifier.
     */
    int Gpu::get_device_id() const { return device_id; }

    /**
     * @brief Gets the GPU vendor type classification.
     * @returns GpuType enum (AMD or NVIDIA).
     */
    GpuType Gpu::get_gpu_type() const { return gpu_type; }

    /**
     * @brief Returns whether the GPU vendor is AMD.
     * @returns True if AMD, false otherwise.
     */
    bool Gpu::get_is_amd_gpu() const { return is_amd_gpu; }

    /**
     * @brief Returns whether the GPU vendor is NVIDIA.
     * @returns True if NVIDIA, false otherwise.
     */
    bool Gpu::get_is_nvidia_gpu() const { return is_nvidia_gpu; }

    /**
     * @brief Gets the GPU model or brand name string.
     * @returns Model descriptor string.
     */
    std::string Gpu::get_model_name() const { return model_name; }

    /**
     * @brief Gets the total physical memory capacity of the GPU in bytes.
     * @returns Total memory bytes.
     */
    uint64_t Gpu::get_total_memory_in_bytes() const { return total_memory_in_bytes; }

    /**
     * @brief Gets the dedicated VRAM size of the GPU in bytes.
     * @returns VRAM size in bytes.
     */
    uint64_t Gpu::get_vram_size_in_bytes() const { return vram_size_in_bytes; }

    /**
     * @brief Classifies the GPU vendor type based on model descriptor name.
     * @param model The model descriptor string.
     */
    void Gpu::set_gpu_type(const std::string &model)
    {
        std::string lower_model = model;
        std::transform(lower_model.begin(), lower_model.end(), lower_model.begin(), [](unsigned char c)
                       { return static_cast<char>(::tolower(c)); });

        is_amd_gpu = false;
        is_nvidia_gpu = false;

        if (lower_model.find("amd") != std::string::npos || lower_model.find("radeon") != std::string::npos)
        {
            gpu_type = GpuType::AMD;
            is_amd_gpu = true;
            return;
        }
        gpu_type = GpuType::NVIDIA;
        is_nvidia_gpu = true;
    }

    /**
     * @brief Constructs a RAM representation with specified total physical size.
     * @param total_size_in_bytes Total RAM size in bytes.
     */
    Ram::Ram(uint64_t total_size_in_bytes) : total_size_in_bytes(total_size_in_bytes) {}

    /**
     * @brief Destructor for Ram.
     */
    Ram::~Ram() = default;

    /**
     * @brief Gets the total size of RAM in bytes.
     * @returns Memory capacity in bytes.
     */
    uint64_t Ram::get_total_size_in_bytes() const { return total_size_in_bytes; }

    /**
     * @brief Constructs an OperatingSystem instance.
     * @param name The OS name string.
     */
    OperatingSystem::OperatingSystem(const std::string &name)
        : is_64_bit(sizeof(void *) == 8), is_windows(false), name(name), os_type(OperatingSystemType::LINUX)
    {
#ifdef _WIN32
        os_type = OperatingSystemType::WINDOWS;
        is_windows = true;
#elif defined(__APPLE__)
        os_type = OperatingSystemType::MACOS;
#else
        os_type = OperatingSystemType::LINUX;
#endif
    }

    /**
     * @brief Destructor for OperatingSystem.
     */
    OperatingSystem::~OperatingSystem() = default;

    /**
     * @brief Checks if the OS environment is 64-bit.
     * @returns True if 64-bit system, false otherwise.
     */
    bool OperatingSystem::get_is_64_bit() const { return sizeof(void *) == 8; }

    /**
     * @brief Checks if the OS is Windows.
     * @returns True if Windows, false otherwise.
     */
    bool OperatingSystem::get_is_windows() const { return os_type == OperatingSystemType::WINDOWS; }

    /**
     * @brief Gets the operating system name.
     * @returns OS name string.
     */
    std::string OperatingSystem::get_name() const { return name; }

    /**
     * @brief Gets the OS family category.
     * @returns OperatingSystemType enum.
     */
    OperatingSystemType OperatingSystem::get_os_type() const { return os_type; }

    /**
     * @brief Constructs HostInfo by passing already-detected hardware component objects.
     * @param cpus A vector of Cpu objects.
     * @param gpus A vector of Gpu objects.
     * @param ram The Ram object.
     * @param os The OperatingSystem object.
     */
    HostInfo::HostInfo(const std::vector<Cpu> &cpus, const std::vector<Gpu> &gpus, const Ram &ram, const OperatingSystem &os)
        : cpus(cpus), gpus(gpus), os(os), ram(ram) {}

    /**
     * @brief Default constructor for HostInfo.
     */
    HostInfo::HostInfo() : os("Unknown"), ram(0) {}

    /**
     * @brief Destructor for HostInfo.
     */
    HostInfo::~HostInfo() = default;

    /**
     * @brief Gets the list of detected CPUs.
     * @returns A vector of Cpu objects.
     */
    std::vector<Cpu> HostInfo::get_cpus() const { return cpus; }

    /**
     * @brief Gets the list of detected GPUs.
     * @returns A vector of Gpu objects.
     */
    std::vector<Gpu> HostInfo::get_gpus() const { return gpus; }

    /**
     * @brief Gets the operating system hardware information.
     * @returns The OperatingSystem object.
     */
    OperatingSystem HostInfo::get_os() const { return os; }

    /**
     * @brief Gets the system RAM hardware information.
     * @returns The Ram object.
     */
    Ram HostInfo::get_ram() const { return ram; }

    /**
     * @brief Utility to convert raw memory bytes into Gigabytes (GB).
     * @param bytes Number of bytes to convert.
     * @returns Capacity rounded up in gigabytes.
     */
    uint32_t convert_bytes_to_gb(uint64_t bytes) { return static_cast<uint32_t>(bytes / BYTES_PER_GB) + 1; }

    /**
     * @brief Automatically queries system APIs to discover host CPU, GPU, RAM, and OS parameters.
     * @returns A fully populated HostInfo object.
     */
    HostInfo detect_host_info()
    {
        uint32_t logical_cores = std::thread::hardware_concurrency();
        if (logical_cores == 0)
            logical_cores = 4;
        uint32_t physical_cores = logical_cores / 2;

        std::string cpu_model = "Not available";
#ifdef _WIN32
        char *proc_id = nullptr;
        size_t sz = 0;
        if (_dupenv_s(&proc_id, &sz, "PROCESSOR_IDENTIFIER") == 0 && proc_id != nullptr)
        {
            cpu_model = proc_id;
            free(proc_id);
        }
#elif defined(__linux__)
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        while (std::getline(cpuinfo, line))
        {
            if (line.find("model name") != std::string::npos)
            {
                auto pos = line.find(":");
                if (pos != std::string::npos)
                {
                    cpu_model = line.substr(pos + 2);
                    break;
                }
            }
        }
#elif defined(__APPLE__)
        char buffer[256];
        size_t bufferlen = 256;
        sysctlbyname("machdep.cpu.brand_string", &buffer, &bufferlen, NULL, 0);
        cpu_model = buffer;
#endif

        Cpu sys_cpu(logical_cores, cpu_model, physical_cores);
        std::vector<Cpu> cpus;
        cpus.emplace_back(sys_cpu);

        uint64_t sys_total_ram = 16ULL * BYTES_PER_GB;
#ifdef _WIN32
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo))
        {
            sys_total_ram = memInfo.ullTotalPhys;
        }
#elif defined(__linux__)
        struct sysinfo memInfo;
        if (sysinfo(&memInfo) == 0)
        {
            sys_total_ram = static_cast<uint64_t>(memInfo.totalram) * memInfo.mem_unit;
        }
#elif defined(__APPLE__)
        uint64_t mem;
        size_t len = sizeof(mem);
        if (sysctlbyname("hw.memsize", &mem, &len, NULL, 0) == 0)
        {
            sys_total_ram = mem;
        }
#endif
        Ram sys_ram(sys_total_ram);

        std::string os_name = "Unknown";
#ifdef _WIN32
        os_name = "Windows";
#elif defined(__linux__)
        os_name = "Linux";
#elif defined(__APPLE__)
        os_name = "macOS";
#endif
        OperatingSystem sys_os(os_name);

        std::vector<Gpu> gpus;
#ifdef _WIN32
        IDXGIFactory *pFactory = nullptr;
        if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory)))
        {
            IDXGIAdapter *pAdapter = nullptr;
            UINT i = 0;
            int device_id = 0;
            while (pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
            {
                DXGI_ADAPTER_DESC desc;
                pAdapter->GetDesc(&desc);

                std::wstring ws(desc.Description);
                std::string name(ws.length(), ' ');
                std::transform(ws.begin(), ws.end(), name.begin(), [](wchar_t c)
                               { return static_cast<char>(c); });

                if (name.find("Basic Render Driver") == std::string::npos)
                {
                    uint64_t vram = desc.DedicatedVideoMemory;
                    gpus.emplace_back(Gpu(device_id++, name, vram, vram));
                }
                pAdapter->Release();
                i++;
            }
            pFactory->Release();
        }
#elif defined(__linux__)
        FILE *pipe = popen("nvidia-smi --query-gpu=index,name,memory.total --format=csv,noheader 2>/dev/null", "r");
        if (pipe)
        {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                std::string line = buffer;
                size_t first_comma = line.find(',');
                size_t second_comma = line.find(',', first_comma + 1);
                if (first_comma != std::string::npos && second_comma != std::string::npos)
                {
                    try
                    {
                        int id = std::stoi(line.substr(0, first_comma));
                        std::string name = line.substr(first_comma + 2, second_comma - first_comma - 2);
                        std::string vram_str = line.substr(second_comma + 2);
                        uint64_t vram_mb = std::stoull(vram_str);
                        uint64_t vram_bytes = vram_mb * 1024ULL * 1024ULL;
                        gpus.emplace_back(Gpu(id, name, vram_bytes, vram_bytes));
                    }
                    catch (...)
                    {
                    }
                }
            }
            pclose(pipe);
        }

        if (gpus.empty())
        {
            pipe = popen("rocm-smi --showproductname --showmeminfo vram --csv 2>/dev/null", "r");
            if (pipe)
            {
                char buffer[256];
                int device_id = 0;
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
                {
                    std::string line = buffer;
                    if (line.find("card") == 0)
                    {
                        size_t first_comma = line.find(',');
                        size_t second_comma = line.find(',', first_comma + 1);
                        if (first_comma != std::string::npos && second_comma != std::string::npos)
                        {
                            try
                            {
                                std::string name = line.substr(first_comma + 1, second_comma - first_comma - 1);
                                std::string vram_str = line.substr(second_comma + 1);
                                uint64_t vram_bytes = std::stoull(vram_str);
                                gpus.emplace_back(Gpu(device_id++, name, vram_bytes, vram_bytes));
                            }
                            catch (...)
                            {
                            }
                        }
                    }
                }
                pclose(pipe);
            }
        }

        if (gpus.empty())
        {
            pipe = popen("lspci 2>/dev/null", "r");
            if (pipe)
            {
                char buffer[256];
                int device_id = 0;
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
                {
                    std::string lower_line = buffer;
                    std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), [](unsigned char c)
                                   { return static_cast<char>(::tolower(c)); });
                    if (lower_line.find("vga") != std::string::npos || lower_line.find("3d controller") != std::string::npos)
                    {
                        std::string name = "Generic Linux GPU";
                        std::string orig_line = buffer;
                        size_t colon = orig_line.find(": ");
                        if (colon != std::string::npos && colon + 2 < orig_line.length())
                        {
                            name = orig_line.substr(colon + 2);
                            name.erase(name.find_last_not_of(" \n\r\t") + 1);
                        }
                        uint64_t estimated_vram = 1024ULL * 1024ULL * 1024ULL;
                        gpus.emplace_back(Gpu(device_id++, name, estimated_vram, estimated_vram));
                    }
                }
                pclose(pipe);
            }
        }
#endif

        return HostInfo(cpus, gpus, sys_ram, sys_os);
    }
}

#ifdef HARDWARE_STANDALONE_TEST
int main()
{
    hardware::HostInfo info = hardware::detect_host_info();

    std::cout << "--- HARDWARE INFO ---\n";

    std::cout << "[OS]\n";
    std::cout << "  Name: " << info.get_os().get_name() << "\n";
    std::cout << "  64-bit: " << (info.get_os().get_is_64_bit() ? "Yes" : "No") << "\n\n";

    std::cout << "[RAM]\n";
    std::cout << "  Total Memory: " << hardware::convert_bytes_to_gb(info.get_ram().get_total_size_in_bytes()) << " GB\n\n";

    std::cout << "[CPU]\n";
    for (const auto &cpu : info.get_cpus())
    {
        std::cout << "  Model: " << cpu.get_model_name() << "\n";
        std::cout << "  Logical Cores: " << cpu.get_logical_cores() << "\n";
        std::cout << "  Vendor: " << (cpu.get_cpu_type() == hardware::CpuType::AMD ? "AMD" : "Intel") << "\n";
    }
    std::cout << "\n";

    std::cout << "[GPU]\n";
    if (info.get_gpus().empty())
    {
        std::cout << "  No GPUs detected natively in this basic stub. (Requires NVML/DXGI)\n";
    }
    else
    {
        for (const auto &gpu : info.get_gpus())
        {
            std::cout << "  Device ID: " << gpu.get_device_id() << "\n";
            std::cout << "  Model: " << gpu.get_model_name() << "\n";
            std::cout << "  VRAM (GB): " << hardware::convert_bytes_to_gb(gpu.get_vram_size_in_bytes()) << "\n\n";
        }
    }

    return 0;
}
#endif
