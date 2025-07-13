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
    private:
        tm *__get_local_time__(time_t *__timer) noexcept(true);
        time_t __get_raw_time__(void) noexcept(true);
        std::_Put_time<char> __get_time_with_format__(const char *__format);

    public:
        Chrono();
        ~Chrono();

        // Public methods
        tm *get_local_time(time_t *__timer) noexcept(true);
        time_t get_raw_time(void) noexcept(true);
        std::_Put_time<char> get_time_with_format(const char *__format);
    };
}