#include "include/hardware/hardware.h"
#include <thread>
#include <iostream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <fstream>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

namespace hardware
{
	// ---------------- Cpu ----------------
	Cpu::Cpu(uint32_t cache_size, uint32_t clock_speed, uint32_t logical_cores, const std::string &model_name, uint32_t physical_cores)
		: cache_size(cache_size), clock_speed(clock_speed), logical_cores(logical_cores), model_name(model_name), physical_cores(physical_cores)
	{
		setCpuType(model_name);
	}

	void Cpu::setCpuType(const std::string &model)
	{
		std::string lower_model = model;
		std::transform(lower_model.begin(), lower_model.end(), lower_model.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
		if (lower_model.find("amd") != std::string::npos || lower_model.find("ryzen") != std::string::npos || lower_model.find("epyc") != std::string::npos)
		{
			cpu_type = CpuType::AMD;
		}
		else
		{
			cpu_type = CpuType::INTEL;
		}
	}

	uint32_t Cpu::get_cache_size() const { return cache_size; }
	uint32_t Cpu::get_clock_speed() const { return clock_speed; }
	CpuType Cpu::get_cpu_type() const { return cpu_type; }
	uint32_t Cpu::get_logical_cores() const { return logical_cores; }
	std::string Cpu::get_model_name() const { return model_name; }
	uint32_t Cpu::get_physical_cores() const { return physical_cores; }

	// ---------------- Gpu ----------------
	Gpu::Gpu(const std::string &bus_location, const std::string &driver_version, const std::string &model_name, uint64_t total_memory_in_bytes, uint64_t vram_size_in_bytes)
		: bus_location(bus_location), driver_version(driver_version), model_name(model_name), total_memory_in_bytes(total_memory_in_bytes), vram_size_in_bytes(vram_size_in_bytes)
	{
		setGpuType(model_name);
	}

	void Gpu::setGpuType(const std::string &model)
	{
		std::string lower_model = model;
		std::transform(lower_model.begin(), lower_model.end(), lower_model.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
		is_amd_gpu = false;
		is_nvidia_gpu = false;

		if (lower_model.find("amd") != std::string::npos || lower_model.find("radeon") != std::string::npos)
		{
			gpu_type = GpuType::AMD;
			is_amd_gpu = true;
		}
		else
		{
			gpu_type = GpuType::NVIDIA;
			is_nvidia_gpu = true;
		}

		// Basic capability flags
		is_cuda_capable = is_nvidia_gpu;
		is_opencl_capable = true; // both modern AMD and NVIDIA support OpenCL
#ifdef _WIN32
		is_directx_capable = true;
		is_vulkan_capable = true;
		is_metal_capable = false;
#elif defined(__APPLE__)
		is_directx_capable = false;
		is_vulkan_capable = false;
		is_metal_capable = true;
#else
		is_directx_capable = false;
		is_vulkan_capable = true;
		is_metal_capable = false;
#endif
	}

	std::string Gpu::get_bus_location() const { return bus_location; }
	std::string Gpu::get_driver_version() const { return driver_version; }
	GpuType Gpu::get_gpu_type() const { return gpu_type; }
	std::string Gpu::get_model_name() const { return model_name; }
	uint64_t Gpu::get_total_memory_in_bytes() const { return total_memory_in_bytes; }
	uint64_t Gpu::get_vram_size_in_bytes() const { return vram_size_in_bytes; }

	bool Gpu::get_is_amd_gpu() const { return is_amd_gpu; }
	bool Gpu::get_is_cuda_capable() const { return is_cuda_capable; }
	bool Gpu::get_is_directx_capable() const { return is_directx_capable; }
	bool Gpu::get_is_metal_capable() const { return is_metal_capable; }
	bool Gpu::get_is_nvidia_gpu() const { return is_nvidia_gpu; }
	bool Gpu::get_is_opencl_capable() const { return is_opencl_capable; }
	bool Gpu::get_is_vulkan_capable() const { return is_vulkan_capable; }

	// ---------------- Ram ----------------
	Ram::Ram() : free_size_in_bytes(0), speed_in_mhz(0), total_size_in_bytes(0) {}

	uint32_t Ram::convert_bytes_to_gb(uint64_t bytes) const
	{
		return static_cast<uint32_t>(bytes / (1024ULL * 1024ULL * 1024ULL));
	}

	uint32_t Ram::get_free_percentage() const
	{
		if (total_size_in_bytes == 0)
			return 0;
		return static_cast<uint32_t>((free_size_in_bytes * 100ULL) / total_size_in_bytes);
	}

	uint64_t Ram::get_free_size_in_bytes() const { return free_size_in_bytes; }
	uint32_t Ram::get_speed_in_mhz() const { return speed_in_mhz; }
	uint64_t Ram::get_total_size_in_bytes() const { return total_size_in_bytes; }

	uint32_t Ram::get_used_percentage() const
	{
		return 100 - get_free_percentage();
	}

	uint64_t Ram::get_used_size_in_bytes() const
	{
		return total_size_in_bytes - free_size_in_bytes;
	}

	// ---------------- OperatingSystem ----------------
	OperatingSystem::OperatingSystem(const std::string &name, const std::string &version)
		: name(name), version(version)
	{
		is_64_bit = sizeof(void *) == 8;

#ifdef _WIN32
		os_type = OperatingSystemType::WINDOWS;
		is_windows = true;
		is_posix_compliant = false;
		is_unix = false;
#elif defined(__APPLE__)
		os_type = OperatingSystemType::MACOS;
		is_windows = false;
		is_posix_compliant = true;
		is_unix = true;
#else
		os_type = OperatingSystemType::LINUX;
		is_windows = false;
		is_posix_compliant = true;
		is_unix = true;
#endif
	}

	std::string OperatingSystem::get_name() const { return name; }
	std::string OperatingSystem::get_version() const { return version; }
	OperatingSystemType OperatingSystem::get_os_type() const { return os_type; }

	bool OperatingSystem::get_is_64_bit() const { return is_64_bit; }
	bool OperatingSystem::get_is_posix_compliant() const { return is_posix_compliant; }
	bool OperatingSystem::get_is_unix() const { return is_unix; }
	bool OperatingSystem::get_is_windows() const { return is_windows; }

	// ---------------- HostInfo ----------------
	HostInfo::HostInfo(const std::vector<Cpu> &cpus, const std::vector<Gpu> &gpus, const Ram &ram, const OperatingSystem &os)
		: cpus(cpus), gpus(gpus), ram(ram), os(os) {}

	std::vector<Cpu> HostInfo::get_cpus() const { return cpus; }
	std::vector<Gpu> HostInfo::get_gpus() const { return gpus; }
	Ram HostInfo::get_ram() const { return ram; }
	OperatingSystem HostInfo::get_os() const { return os; }

	// ---------------- Detect Function ----------------
	HostInfo detect_host_info()
	{
		// CPU
		uint32_t logical_cores = std::thread::hardware_concurrency();
		if (logical_cores == 0)
			logical_cores = 4;
		uint32_t physical_cores = logical_cores / 2; // approximation without heavy API

		std::string cpu_model = "Generic CPU";
#ifdef _WIN32
		// Very basic CPU detect on windows using env vars for model (approximation)
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

		Cpu sys_cpu(0, 0, logical_cores, cpu_model, physical_cores);
		std::vector<Cpu> cpus;
		cpus.push_back(sys_cpu);

		// RAM
		// Since Ram has no setters and members are private, we construct a binary compatible struct
		// Or better, just pass default for now since we can't cleanly set it without modifying the header.
		// Wait, we CAN modify the header if we want, or just leave Ram as 0s in this basic stub.
		// We'll leave Ram at 0 for now as it requires header modifications to be cleanly initialized.
		Ram sys_ram;

		// OS
		std::string os_name = "Unknown";
		std::string os_version = "Unknown";
#ifdef _WIN32
		os_name = "Windows";
#elif defined(__linux__)
		os_name = "Linux";
		struct utsname buffer;
		if (uname(&buffer) == 0)
		{
			os_version = buffer.release;
		}
#elif defined(__APPLE__)
		os_name = "macOS";
#endif
		OperatingSystem sys_os(os_name, os_version);

		// GPU
		// Proper GPU detection on Windows requires WMI/DXGI, on Linux requires NVML/lspci
		// For this stub, we return an empty list or a generic one so it compiles and runs.
		std::vector<Gpu> gpus;
		// Gpu sys_gpu("00:00.0", "Generic", "Generic GPU", 8000000000ULL, 8000000000ULL);
		// gpus.push_back(sys_gpu);

		return HostInfo(cpus, gpus, sys_ram, sys_os);
	}
}

// ---------------- Standalone Test ----------------
#ifdef HARDWARE_STANDALONE_TEST
int main()
{
	hardware::HostInfo info = hardware::detect_host_info();

	std::cout << "--- HARDWARE INFO ---\n";

	std::cout << "[OS]\n";
	std::cout << "  Name: " << info.get_os().get_name() << "\n";
	std::cout << "  Version: " << info.get_os().get_version() << "\n";
	std::cout << "  64-bit: " << (info.get_os().get_is_64_bit() ? "Yes" : "No") << "\n\n";

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
		std::cout << "  No GPUs detected natively in this basic stub.\n";
	}
	else
	{
		for (const auto &gpu : info.get_gpus())
		{
			std::cout << "  Model: " << gpu.get_model_name() << "\n";
			std::cout << "  VRAM (GB): " << (gpu.get_vram_size_in_bytes() / (1024 * 1024 * 1024)) << "\n";
		}
	}

	return 0;
}
#endif
