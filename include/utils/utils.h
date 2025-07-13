/*
 *   Copyright (C) 2025 Juan Minor
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>
#include <string>
#include <stdexcept>
#include "include/core/core.h"
#include "include/logger/logger.h"

namespace utils
{
    const std::string get_algebraic_notation(const uint8_t &__rank, const uint8_t &__file);
    void log_throw_error(const std::string &__error, const bool &__throw, const char *__file, const uint8_t &__lineno);
    void parse_algebraic_notation(const std::string &__algebraic_notation, uint8_t &__rank, uint8_t &__file);

/* Macros */
#define LOG_THROW_ERROR(ERROR, THROW) utils::log_throw_error(ERROR, THROW, __FILE__, __LINE__)
}
