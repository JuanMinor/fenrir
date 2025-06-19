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

#include "include/utils/utils.h"

namespace utils
{
    const std::string get_algebraic_notation(const uint8_t &__rank, const uint8_t &__file)
    {
        if (__rank >= fenrir::BOARD_SIZE || __rank < 0 || __file >= fenrir::BOARD_SIZE || __file < 0)
        {
            LOG_THROW_ERROR("Cannot get algebraic notation for invalid board address", true);
            return "";
        }
        return ((char(97 + __file)) + std::to_string(unsigned(__rank + 1))).c_str();
    }

    void log_throw_error(const std::string &__error, const bool &__throw, const char *__file, const uint8_t &__lineno)
    {
        if (__error.empty())
        {
            logger::Logger().log("Error message cannot be null", __file, __lineno, logger::ERROR);
            if (__throw)
            {
                throw std::runtime_error("Error message cannot be null");
            }
            return;
        }

        logger::Logger().log(__error, __file, __lineno, logger::ERROR);
        if (__throw)
        {
            throw std::runtime_error(__error);
        }
    }

    void parse_algebraic_notation(const std::string &__algebraic_notation, uint8_t &__rank, uint8_t &__file)
    {
        if (__algebraic_notation.empty() || __algebraic_notation.length() != 2 || !std::isalpha(__algebraic_notation[0]) || !std::isdigit(__algebraic_notation[1]))
        {
            LOG_THROW_ERROR("Invalid algebraic notation", true);
            return;
        }

        __file = std::tolower(__algebraic_notation[0]) - 'a';
        __rank = __algebraic_notation[1] - '1';

        if (__file >= fenrir::BOARD_SIZE || __rank >= fenrir::BOARD_SIZE)
        {
            LOG_THROW_ERROR("Invalid algebraic notation", true);
            return;
        }
    }
}
