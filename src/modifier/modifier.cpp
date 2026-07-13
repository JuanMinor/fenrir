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
    /**
     * @brief Constructs an ANSI color modifier.
     * @param color_value The Color enum representing the desired text or background color.
     */
    Modifier::Modifier(Color color_value) : color(color_value) {}

    /**
     * @brief Destroys the ANSI color modifier.
     */
    Modifier::~Modifier() {}

    /**
     * @brief Overloaded stream insertion operator to output ANSI escape codes.
     * @param os The output stream.
     * @param modifier The color modifier object.
     * @returns The modified output stream.
     */
    std::ostream &operator<<(std::ostream &os, const Modifier &modifier)
    {
        return os << "\033[" << static_cast<int>(modifier.color) << "m";
    }
}
