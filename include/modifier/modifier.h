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

#include <ostream>
#include "include/core/core.h"

namespace color
{
    enum class Color : uint8_t
    {
        RESET = 0,
        FG_BLACK = 30,
        BG_BLACK = 40,
        FG_RED = 31,
        BG_RED = 41,
        FG_GREEN = 32,
        BG_GREEN = 42,
        FG_YELLOW = 33,
        BG_YELLOW = 43,
        FG_BLUE = 34,
        BG_BLUE = 44,
        FG_MAGENTA = 35,
        BG_MAGENTA = 45,
        FG_CYAN = 36,
        BG_CYAN = 46,
        FG_WHITE = 37,
        BG_WHITE = 47
    };
    class Modifier
    {
    private:
        Color color;

    public:
        Modifier(Color color_value);
        ~Modifier();

        friend std::ostream &operator<<(std::ostream &os, const Modifier &modifier);
    };
}
