/*
 *   Copyright (c) 2026 Juan Minor
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

namespace chess
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

        /**
         * @brief Get board placement string (the rank/file portion of FEN).
         * @returns Placement string (e.g., "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR").
         */
        std::string get_placement(void) const;

        /**
         * @brief Get castling rights string representation.
         * @returns Castling rights (e.g., "KQkq", "-").
         */
        std::string get_castling(void) const;

        /**
         * @brief Get color (side to move).
         * @returns Color value: 0 for white, 1 for black.
         */
        uint8_t get_color(void) const;

        /**
         * @brief Get en passant target square.
         * @returns En passant square (e.g., "e3", "-").
         */
        std::string get_en_passant(void) const;

        /**
         * @brief Get full move number (increments each time black moves).
         * @returns Full move count.
         */
        uint32_t get_full_moves(void) const;

        /**
         * @brief Get half-move clock (moves since last pawn move or capture).
         * @returns Half-move clock value (0-50 for draw rule).
         */
        uint32_t get_half_move_clock(void) const;

        /**
         * @brief Generate FEN string from current position state.
         * @returns Complete FEN notation string.
         */
        std::string generate_fen(void) const;

        /**
         * @brief Set board placement string (rank/file portion of FEN).
         * @param placement_string Placement string (e.g., "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR").
         */
        void set_placement(const std::string &placement_string);

        /**
         * @brief Set castling rights.
         * @param castling_rights Castling rights string (e.g., "KQkq", "-").
         */
        void set_castling(const std::string &castling_rights);

        /**
         * @brief Set color (side to move).
         * @param color_value Color value: 0 for white, 1 for black.
         */
        void set_color(uint8_t color_value);

        /**
         * @brief Set en passant target square.
         * @param en_passant_square En passant square (e.g., "e3", "-").
         */
        void set_en_passant(const std::string &en_passant_square);

        /**
         * @brief Set full move number (increments each time black moves).
         * @param full_moves_value Full move count.
         */
        void set_full_moves(uint32_t full_moves_value);

        /**
         * @brief Set half-move clock (moves since last pawn move or capture).
         * @param half_move_clock_value Half-move clock value (0-50 for draw rule).
         */
        void set_half_move_clock(uint32_t half_move_clock_value);
    };
}
