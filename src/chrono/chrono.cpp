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

namespace chrono
{
    Chrono::Chrono() {}

    Chrono::~Chrono() {}

    tm *Chrono::__get_local_time__(time_t *__timer) noexcept(true)
    {
        if (!__timer)
        {
            time_t __timer__ = std::time(nullptr);
            __timer = &__timer__;
        }
        return std::localtime(__timer);
    }

    time_t Chrono::__get_raw_time__(void) noexcept(true)
    {
        return std::time(nullptr);
    }

    std::_Put_time<char> Chrono::__get_time_with_format__(const char *__format)
    {
        tm *localtime = this->__get_local_time__(nullptr);
        return std::put_time(localtime, __format);
    }

    // public
    tm *Chrono::get_local_time(time_t *__timer) noexcept(true)
    {
        return this->__get_local_time__(__timer);
    }
    time_t Chrono::get_raw_time(void) noexcept(true)
    {
        return this->__get_raw_time__();
    }
    std::_Put_time<char> Chrono::get_time_with_format(const char *__format)
    {
        return this->__get_time_with_format__(__format);
    }
}