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
    bool are_chess_piece_count_rules_valid(const std::unordered_map<char, uint8_t> &piece_counts)
    {
        if (piece_counts.at('K') != 1 || piece_counts.at('k') != 1)
        {
            return false;
        }
        if (piece_counts.at('Q') > 9 || piece_counts.at('q') > 9)
        {
            return false;
        }
        if (piece_counts.at('R') > 10 || piece_counts.at('r') > 10)
        {
            return false;
        }
        if (piece_counts.at('B') > 10 || piece_counts.at('b') > 10)
        {
            return false;
        }
        if (piece_counts.at('N') > 10 || piece_counts.at('n') > 10)
        {
            return false;
        }
        if (piece_counts.at('P') > 8 || piece_counts.at('p') > 8)
        {
            return false;
        }

        int white_extra_queens = std::max(0, static_cast<int>(piece_counts.at('Q')) - 1);
        int white_extra_rooks = std::max(0, static_cast<int>(piece_counts.at('R')) - 2);
        int white_extra_bishops = std::max(0, static_cast<int>(piece_counts.at('B')) - 2);
        int white_extra_knights = std::max(0, static_cast<int>(piece_counts.at('N')) - 2);

        int white_promoted_pieces = white_extra_queens + white_extra_rooks + white_extra_bishops + white_extra_knights;
        int white_missing_pawns = 8 - static_cast<int>(piece_counts.at('P'));

        if (white_promoted_pieces > white_missing_pawns)
        {
            return false;
        }

        int black_extra_queens = std::max(0, static_cast<int>(piece_counts.at('q')) - 1);
        int black_extra_rooks = std::max(0, static_cast<int>(piece_counts.at('r')) - 2);
        int black_extra_bishops = std::max(0, static_cast<int>(piece_counts.at('b')) - 2);
        int black_extra_knights = std::max(0, static_cast<int>(piece_counts.at('n')) - 2);

        int black_promoted_pieces = black_extra_queens + black_extra_rooks + black_extra_bishops + black_extra_knights;
        int black_missing_pawns = 8 - static_cast<int>(piece_counts.at('p'));

        if (black_promoted_pieces > black_missing_pawns)
        {
            return false;
        }

        return true;
    }

    std::string get_algebraic_notation(uint8_t rank, uint8_t file)
    {
        if (rank >= fenrir::BOARD_SIZE || file >= fenrir::BOARD_SIZE)
        {
            LOG_THROW_ERROR("Cannot get algebraic notation for invalid board address", true);
        }
        return std::string(1, static_cast<char>(97 + file)) + std::to_string(rank + 1);
    }

    void log_throw_error(const std::string &error, bool throw_error, const char *file, int lineno)
    {
        if (error.empty())
        {
            logger::Logger::get_instance().log("Error message cannot be null", file, static_cast<uint32_t>(lineno), logger::LEVEL::ERROR);
            if (throw_error)
            {
                throw std::runtime_error("Error message cannot be null");
            }
        }

        logger::Logger::get_instance().log(error, file, static_cast<uint32_t>(lineno), logger::LEVEL::ERROR);
        if (throw_error)
        {
            throw std::runtime_error(error);
        }
    }

    void parse_algebraic_notation(const std::string &algebraic_notation, uint8_t &rank, uint8_t &file)
    {
        if (algebraic_notation.empty() || algebraic_notation.length() != 2U || !std::isalpha(algebraic_notation[0]) || !std::isdigit(algebraic_notation[1]))
        {
            LOG_THROW_ERROR("Invalid algebraic notation", true);
        }

        int f = std::tolower(algebraic_notation[0]) - 'a';
        int r = algebraic_notation[1] - '1';

        if (r < 0 || r >= fenrir::BOARD_SIZE || f < 0 || f >= fenrir::BOARD_SIZE)
        {
            LOG_THROW_ERROR("Invalid algebraic notation", true);
        }

        rank = static_cast<uint8_t>(r);
        file = static_cast<uint8_t>(f);
    }
}
