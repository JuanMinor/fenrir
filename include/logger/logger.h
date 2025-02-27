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

#include <mutex>
#include <stdint.h>

#include "include/core/core.h"

namespace logger
{
    class Logger
    {
        mutable std::mutex log_mutex;

    public:
        Logger();
        ~Logger();

        // @methods
        void log(const std::string &__message, const char *__file, const uint8_t &__lineno, const LEVEL &__level) const;
    };

    // @macros
#define LOG Logger()

#define LOG_DEBUG(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::DEBUG);
#define LOG_INFO(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::INFO);
#define LOG_WARN(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::WARN);
#define LOG_ERROR(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::ERROR);
#define LOG_CRITICAL(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::CRITICAL);
}

#endif