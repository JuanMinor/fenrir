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
    /**
     * @brief Checks if white and black piece counts strictly satisfy standard chess starting and promotion rules.
     * @param piece_counts An unordered map mapping piece characters ('K', 'q', 'p', etc.) to their counts.
     * @returns True if piece counts obey chess rules, false if counts are invalid.
     */
    bool are_chess_piece_count_rules_valid(const std::unordered_map<char, uint8_t> &piece_counts);

    /**
     * @brief Converts 0-indexed board coordinates (rank, file) to standard 2-character algebraic notation (e.g. "e4").
     * @param rank The 0-based rank index (0 for 1st rank, 7 for 8th rank).
     * @param file The 0-based file index (0 for 'a', 7 for 'h').
     * @returns A std::string representation of algebraic coordinates.
     */
    std::string get_algebraic_notation(uint8_t rank, uint8_t file);

    /**
     * @brief Helper function to log an error and throw a std::runtime_error.
     * @param error The message describing the error.
     * @param throw_error Flag specifying whether to raise a runtime exception.
     * @param file The source file where the error occurred.
     * @param lineno The line number of the source file.
     */
    void log_throw_error(const std::string &error, bool throw_error, const char *file, int lineno);

    /**
     * @brief Parses an algebraic chess square string into rank and file indices.
     * @param algebraic_notation The 2-character algebraic notation string (e.g. "h8").
     * @param rank Reference to store the 0-indexed rank result.
     * @param file Reference to store the 0-indexed file result.
     */
    void parse_algebraic_notation(const std::string &algebraic_notation, uint8_t &rank, uint8_t &file);

#define LOG_THROW_ERROR(ERROR, THROW) utils::log_throw_error(ERROR, THROW, __FILE__, __LINE__)
}
