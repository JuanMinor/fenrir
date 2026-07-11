/*
 *   Copyright (c) 2025 Juan Minor

 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <stdint.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include "include/core/core.h"
#include "include/chrono/chrono.h"

namespace logger
{
    class Logger
    {
    private:
        mutable std::mutex log_mutex;
        Logger();
        ~Logger();

    public:
        static Logger &get_instance();

        Logger(const Logger &) = delete;
        Logger &operator=(const Logger &) = delete;
        Logger(Logger &&) = delete;
        Logger &operator=(Logger &&) = delete;

        void log(const std::string &message, const char *file, uint32_t line_number, LEVEL level) const;
    };

#define DEBUG(MESSAGE) Logger::get_instance().log(MESSAGE, __FILE__, __LINE__, logger::LEVEL::DEBUG)
#define INFO(MESSAGE) Logger::get_instance().log(MESSAGE, __FILE__, __LINE__, logger::LEVEL::INFO)
#define WARN(MESSAGE) Logger::get_instance().log(MESSAGE, __FILE__, __LINE__, logger::LEVEL::WARN)
#define ERROR(MESSAGE) Logger::get_instance().log(MESSAGE, __FILE__, __LINE__, logger::LEVEL::ERROR)
#define CRITICAL(MESSAGE) Logger::get_instance().log(MESSAGE, __FILE__, __LINE__, logger::LEVEL::CRITICAL)

}
