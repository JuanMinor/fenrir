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

#ifndef LOGGER_H
#define LOGGER_H

#include <filesystem>
#include <fstream>
#include <mutex>
#include <stdint.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include "include/core/core.h"
#include "include/chrono/chrono.h"
#include "include/logger/logger.h"

namespace logger
{
    class Logger
    {
    private:
        mutable std::mutex log_mutex;

    public:
        Logger();
        ~Logger();

        // Logging method
        void log(const std::string &__message, const char *__file, const uint8_t &__lineno, const LEVEL &__level) const;
    };

    // Macros for logging
#define LOG Logger()

#define DEBUG(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::DEBUG);
#define INFO(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::INFO);
#define WARN(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::WARN);
#define ERROR(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::ERROR);
#define CRITICAL(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::CRITICAL);
}

#endif // LOGGER_H