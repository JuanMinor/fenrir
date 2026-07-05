/*
 *   Copyright (c) 2025 Juan Minor
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

#include "include/logger/logger.h"

namespace logger
{
	std::unordered_map<LEVEL, const char *> level_types = {
		{LEVEL::DEBUG, "[DEBUG] - "},
		{LEVEL::INFO, "[INFO] - "},
		{LEVEL::WARN, "[WARN] - "},
		{LEVEL::ERROR, "[ERROR] - "},
		{LEVEL::CRITICAL, "[CRITICAL] - "}};

	void rotate_logs()
	{
		namespace fs = std::filesystem;

		if (fs::exists(LOG_FILE) && fs::file_size(LOG_FILE) > MAX_LOG_SIZE)
		{
			std::string backup_file = std::string(LOG_FILE) + ".1";
			if (fs::exists(backup_file))
			{
				fs::remove(backup_file);
			}
			fs::rename(LOG_FILE, backup_file);
		}
	}

	Logger::Logger() {}
	Logger::~Logger() {}

	Logger &Logger::get_instance()
	{
		static Logger instance;
		return instance;
	}

	void Logger::log(const std::string &message, const char *file, uint32_t line_number, LEVEL level) const
	{
		if (level == LEVEL::DEBUG && !fenrir::DEBUG)
		{
			return;
		}

		auto timestamp = chrono::Chrono().get_time_with_format("%a %b %d, %Y @ %H:%M:%S");

		std::lock_guard<std::mutex> guard(log_mutex);

		rotate_logs();

		std::ofstream log_file(LOG_FILE, std::ios_base::app);
		if (!log_file)
		{
			return;
		}

		try
		{
			const char *type = level_types.at(level);
			log_file << "[" << timestamp << "] [" << file << " @ Line " << line_number << "]::" << type << message << std::endl;
		}
		catch (const std::exception &)
		{
			/* Fallback: no-op */
		}
	}

}
