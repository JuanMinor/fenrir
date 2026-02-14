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
	bool areChessPieceCountRulesValid(const std::unordered_map<char, uint8_t> &piece_counts);
	const std::string getAlgebraicNotation(const uint8_t &rank, const uint8_t &file);
	void logThrowError(const std::string &error, const bool &throw_error, const char *file, const int &lineno);
	void parseAlgebraicNotation(const std::string &algebraic_notation, uint8_t &rank, uint8_t &file);

/* Macros */
#define LOG_THROW_ERROR(ERROR, THROW) utils::logThrowError(ERROR, THROW, __FILE__, __LINE__)
}
