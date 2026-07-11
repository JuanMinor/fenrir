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

#include "include/modifier/modifier.h"

namespace color
{
    Modifier::Modifier(Color color_value) : color(color_value) {}

    Modifier::~Modifier() {}

    std::ostream &operator<<(std::ostream &os, const Modifier &modifier)
    {
        return os << "\033[" << static_cast<int>(modifier.color) << "m";
    }
}
