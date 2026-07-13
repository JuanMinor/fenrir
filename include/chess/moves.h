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

#include <string>
#include <sstream>
#include <vector>
#include "include/logger/logger.h"
#include "include/chess/move.h"
#include "include/utils/utils.h"
#include "include/abstract/board.h"

namespace chess
{
    class Moves
    {
    private:
        Moves() = delete;
        ~Moves() = delete;
        Moves(const Moves &) = delete;
        Moves &operator=(const Moves &) = delete;
        Moves(Moves &&) = delete;
        Moves &operator=(Moves &&) = delete;

        static void generate_bishop_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves);
        static void add_capture_move(uint8_t from_rank, uint8_t from_file, char piece_char, uint8_t to_rank, uint8_t to_file, char target_char, std::vector<Move> &moves);
        static void generate_king_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves);
        static void generate_knight_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves);
        static void generate_pawn_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves);
        static void generate_queen_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves);
        static void generate_rook_moves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves);
        static void slide_in_directions(uint8_t from_rank, uint8_t from_file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves, const int8_t direction_vectors[][2], size_t num_directions, bool single_depth = false);

    public:
        static void generate_moves(uint8_t rank, uint8_t file, const AbstractBoard &board, std::vector<Move> &moves);
    };
}
