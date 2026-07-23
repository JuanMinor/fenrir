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

#include "include/chrono/chrono.h"
#include <sstream>

namespace chrono
{
    /**
     * @brief Constructs a new Chrono timer object.
     */
    Chrono::Chrono() {}

    /**
     * @brief Destroys the Chrono timer object.
     */
    Chrono::~Chrono() {}

    /**
     * @brief Retrieves the local time structure based on a system timer.
     * @param timer Optional pointer to a time_t value to convert. If nullptr, current system time is used.
     * @returns A pointer to a thread-local tm structure containing local time components.
     */
    tm *Chrono::get_local_time(time_t *timer) const noexcept
    {
        static thread_local tm local_time_buf;
        time_t temp_timer;
        if (!timer)
        {
            temp_timer = std::time(nullptr);
            timer = &temp_timer;
        }
#ifdef _MSC_VER
        localtime_s(&local_time_buf, timer);
#else
        localtime_r(timer, &local_time_buf);
#endif
        return &local_time_buf;
    }

    /**
     * @brief Gets the current calendar raw time.
     * @returns The current system time as a time_t value.
     */
    time_t Chrono::get_raw_time(void) const noexcept
    {
        return std::time(nullptr);
    }

    /**
     * @brief Returns a formatted string representing the current system local time.
     * @param format The format specifier string (compatible with std::put_time).
     * @returns A std::string containing the formatted date/time.
     */
    std::string Chrono::get_time_with_format(const char *format) const
    {
        tm *local_time = this->get_local_time(nullptr);
        std::stringstream ss;
        ss << std::put_time(local_time, format);
        return ss.str();
    }
}
