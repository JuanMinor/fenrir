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

#include <fstream>
#include <unordered_map>

#include "include/chrono/chrono.h"
#include "include/logger/logger.h"

namespace logger
{

    std::unordered_map<uint8_t, const char *> level_types = {
        {DEBUG, "[DEBUG] - "},
        {INFO, "[INFO] - "},
        {WARN, "[WARN] - "},
        {ERROR, "[ERROR] - "},
        {CRITICAL, "[CRITICAL] - "}};

    Logger::Logger() {}
    Logger::~Logger() {}

    void Logger::log(const std::string &__message, const char *__file, const uint8_t &__lineno, const LEVEL &__level) const
    {
        if (__level == DEBUG && !DEBUG_ENABLED)
        {
            return;
        }

        std::_Put_time<char> timestamp = chrono::Chrono().get_time_with_format("%a %b %d, %Y @ %H:%M:%S");

        std::lock_guard<std::mutex> guard(this->log_mutex);

        // @file
        std::fstream log_file;
        log_file.open(LOG_FILE, std::ios_base::app);
        if (!log_file)
        {
            return;
        }
        try
        {
            const char *type = level_types.at(__level);
            log_file << "[" << timestamp << "] [" << __file << " @ Line " << unsigned(__lineno) << "]::" << type << __message << std::endl;
            log_file.close();
        }
        catch (...)
        {
            // @close
            log_file.close();
        }
    }
}