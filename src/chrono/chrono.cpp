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

	tm *Chrono::getLocalTime(time_t *timer) const noexcept
	{
		if (!timer)
		{
			static time_t current_time;
			current_time = std::time(nullptr);
			timer = &current_time;
		}
		return std::localtime(timer);
	}

	time_t Chrono::getRawTime(void) const noexcept
	{
		return std::time(nullptr);
	}

	std::_Put_time<char> Chrono::getTimeWithFormat(const char *format) const
	{
		tm *local_time = this->getLocalTime(nullptr);
		return std::put_time(local_time, format);
	}
}
