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
	/* CPU */
	Cpu::Cpu(uint32_t logical_cores, const std::string &model_name, uint32_t physical_cores)
		: logical_cores(logical_cores), model_name(model_name), physical_cores(physical_cores)
	{
		setCpuType(model_name);
	}
	Cpu::~Cpu() = default;

	void Cpu::setCpuType(const std::string &model)
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

	CpuType Cpu::get_cpu_type() const { return cpu_type; }
	uint32_t Cpu::get_logical_cores() const { return logical_cores; }
	std::string Cpu::get_model_name() const { return model_name; }
	uint32_t Cpu::get_physical_cores() const { return physical_cores; }

	/* GPU */
	Gpu::Gpu(int device_id, const std::string &model_name, uint64_t total_memory_in_bytes, uint64_t vram_size_in_bytes)
		: device_id(device_id), model_name(model_name), total_memory_in_bytes(total_memory_in_bytes), vram_size_in_bytes(vram_size_in_bytes)
	{
		setGpuType(model_name);
	}

	void Gpu::setGpuType(const std::string &model)
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
	Gpu::~Gpu() = default;

	int Gpu::get_device_id() const { return device_id; }
	GpuType Gpu::get_gpu_type() const { return gpu_type; }
	std::string Gpu::get_model_name() const { return model_name; }
	uint64_t Gpu::get_total_memory_in_bytes() const { return total_memory_in_bytes; }
	uint64_t Gpu::get_vram_size_in_bytes() const { return vram_size_in_bytes; }
	bool Gpu::get_is_amd_gpu() const { return is_amd_gpu; }
	bool Gpu::get_is_nvidia_gpu() const { return is_nvidia_gpu; }

	/* RAM */
	Ram::Ram(uint64_t total_size_in_bytes) : total_size_in_bytes(total_size_in_bytes) {}
	Ram::~Ram() = default;

	uint64_t Ram::get_total_size_in_bytes() const { return total_size_in_bytes; }

	/* OperatingSystem */
	OperatingSystem::OperatingSystem(const std::string &name)
		: name(name)
	{
#ifdef _WIN32
		os_type = OperatingSystemType::WINDOWS;
#elif defined(__APPLE__)
		os_type = OperatingSystemType::MACOS;
#else
		os_type = OperatingSystemType::LINUX;
#endif
	}
	OperatingSystem::~OperatingSystem() = default;

	std::string OperatingSystem::get_name() const { return name; }
	OperatingSystemType OperatingSystem::get_os_type() const { return os_type; }

	bool OperatingSystem::get_is_64_bit() const { return sizeof(void *) == 8; }
	bool OperatingSystem::get_is_windows() const { return os_type == OperatingSystemType::WINDOWS; }

	/* Host */
	HostInfo::HostInfo(const std::vector<Cpu> &cpus, const std::vector<Gpu> &gpus, const Ram &ram, const OperatingSystem &os)
		: cpus(cpus), gpus(gpus), ram(ram), os(os) {}
	HostInfo::~HostInfo() = default;

	std::vector<Cpu> HostInfo::get_cpus() const { return cpus; }
	std::vector<Gpu> HostInfo::get_gpus() const { return gpus; }
	Ram HostInfo::get_ram() const { return ram; }
	OperatingSystem HostInfo::get_os() const { return os; }

	/* Scope */
	uint32_t convert_bytes_to_gb(uint64_t bytes) { return static_cast<uint32_t>(bytes / BYTES_PER_GB); }
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
		cpus.push_back(sys_cpu);

		// RAM
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

		// OS
		std::string os_name = "Unknown";
#ifdef _WIN32
		os_name = "Windows";
#elif defined(__linux__)
		os_name = "Linux";
#elif defined(__APPLE__)
		os_name = "macOS";
#endif
		OperatingSystem sys_os(os_name);

		// GPU
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
				std::transform(ws.begin(), ws.end(), name.begin(), [](wchar_t c) { return static_cast<char>(c); });

				// Filter out Microsoft Basic Render Driver software adapter
				if (name.find("Basic Render Driver") == std::string::npos)
				{
					uint64_t vram = desc.DedicatedVideoMemory;
					gpus.push_back(Gpu(device_id++, name, vram, vram));
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
						uint64_t vram_mb = std::stoull(vram_str); // stoull automatically stops at " MiB"
						uint64_t vram_bytes = vram_mb * 1024ULL * 1024ULL;
						gpus.push_back(Gpu(id, name, vram_bytes, vram_bytes));
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
								gpus.push_back(Gpu(device_id++, name, vram_bytes, vram_bytes));
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
						gpus.push_back(Gpu(device_id++, name, estimated_vram, estimated_vram));
					}
				}
				pclose(pipe);
			}
		}
#endif

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
