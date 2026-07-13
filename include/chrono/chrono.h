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

#include <ctime>
#include <iomanip>
#include <string>

namespace chrono
{
    class Chrono
    {
    public:
        /**
         * @brief Constructs a new Chrono timer object.
         */
        Chrono();

        /**
         * @brief Destroys the Chrono timer object.
         */
        ~Chrono();

        /**
         * @brief Retrieves the local time structure based on a system timer.
         * @param timer Optional pointer to a time_t value to convert. If nullptr, current system time is used.
         * @returns A pointer to a thread-local tm structure containing local time components.
         */
        tm *get_local_time(time_t *timer) const noexcept(true);

        /**
         * @brief Gets the current calendar raw time.
         * @returns The current system time as a time_t value.
         */
        time_t get_raw_time(void) const noexcept(true);

        /**
         * @brief Returns a formatted string representing the current system local time.
         * @param format The format specifier string (compatible with std::put_time).
         * @returns A std::string containing the formatted date/time.
         */
        std::string get_time_with_format(const char *format) const;
    };
}
