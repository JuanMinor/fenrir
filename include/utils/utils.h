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
#include <unordered_map>
#include "include/core/core.h"
#include "include/logger/logger.h"

namespace utils
{
    bool are_chess_piece_count_rules_valid(const std::unordered_map<char, uint8_t> &piece_counts);
    std::string get_algebraic_notation(uint8_t rank, uint8_t file);
    void log_throw_error(const std::string &error, bool throw_error, const char *file, int lineno);
    void parse_algebraic_notation(const std::string &algebraic_notation, uint8_t &rank, uint8_t &file);

/* Macros */
#define LOG_THROW_ERROR(ERROR, THROW) utils::log_throw_error(ERROR, THROW, __FILE__, __LINE__)
}
