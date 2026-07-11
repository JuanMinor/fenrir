/*
 *   Copyright (c) 2025 Juan Minor
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

#include <regex>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "include/core/core.h"
#include "include/logger/logger.h"
#include "include/utils/utils.h"

namespace fenrir
{
    class Fen
    {
    private:
        std::string placement;
        std::string castling;
        std::string en_passant;
        uint8_t color;
        uint32_t half_move_clock;
        uint32_t full_moves;

        void split_string(const std::string &fen_string, const std::string &delimiters, std::vector<std::string> &tokens) const;
        void validate_chess_rules(const std::string &placement_string) const;
        void validate_placement(const std::string &placement_string) const;
        void validate_pawn_placement(const std::vector<std::string> &ranks) const;
        void validate_king_safety(const std::vector<std::string> &ranks) const;

    public:
        Fen(const std::string &fen_string);

        ~Fen();

        std::string get_placement(void) const;
        std::string get_castling(void) const;
        std::string get_en_passant(void) const;
        uint8_t get_color(void) const;
        uint32_t get_half_move_clock(void) const;
        uint32_t get_full_moves(void) const;

        void set_placement(const std::string &placement_string);
        void set_castling(const std::string &castling_rights);
        void set_en_passant(const std::string &en_passant_square);
        void set_color(uint8_t color_value);
        void set_half_move_clock(uint32_t half_move_clock_value);
        void set_full_moves(uint32_t full_moves_value);

        std::string generate_fen(void) const;
    };
}
