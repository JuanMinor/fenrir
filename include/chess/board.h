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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <format>
#include "include/core/core.h"
#include "include/chess/fen.h"
#include "include/chess/move.h"
#include "include/logger/logger.h"
#include "include/modifier/modifier.h"
#include "include/pgn/pgn.h"
#include "include/utils/utils.h"
#include "include/abstract/board.h"

namespace chess
{
    /*
     * UndoState captures state to reverse apply_move().
     * Zero heap allocation (all primitive values).
     */
    struct UndoState
    {
        uint64_t bitboards[12]; /* Snapshot of 12 piece bitboards. */
        uint64_t white_occupancy;
        uint64_t black_occupancy;
        uint64_t combined_occupancy;
        uint8_t castling_rights;
        uint8_t en_passant_square; /* 64 = none */
        uint8_t color;
        uint8_t half_move_clock;
        uint16_t full_moves; /* uint8_t overflows in games longer than 255 full moves */
    };

    /*
     * Manages chess piece bitboards and board state metadata.
     * Operates via bitwise operations with zero heap allocation.
     */
    class Board : public AbstractBoard
    {
    public:
        static constexpr uint8_t CASTLE_K = 1 << 0;
        static constexpr uint8_t CASTLE_Q = 1 << 1;
        static constexpr uint8_t CASTLE_k = 1 << 2;
        static constexpr uint8_t CASTLE_q = 1 << 3;

    private:
        uint64_t bitboards[12]; /* 0-5: White (P, N, B, R, Q, K), 6-11: Black (p, n, b, r, q, k) */
        uint64_t white_occupancy;
        uint64_t black_occupancy;
        uint64_t combined_occupancy;

        uint8_t castling_rights;
        uint8_t en_passant_square; /* 64 = none */

        mutable std::string castling_str;
        mutable std::string en_passant_str;
        mutable bool castling_dirty;
        mutable bool en_passant_dirty;

        uint8_t color;
        uint8_t half_move_clock;
        uint16_t full_moves;
        Fen fen;

        void build_board(const std::string &placement);

        inline void clear_bit(uint64_t &bb, uint8_t square)
        {
            bb &= ~(1ULL << square);
        }

        std::string generate_placement_from_board(void) const;

        inline int get_bitboard_index(char alias) const
        {
            switch (alias)
            {
            case 'P':
                return 0;
            case 'N':
                return 1;
            case 'B':
                return 2;
            case 'R':
                return 3;
            case 'Q':
                return 4;
            case 'K':
                return 5;
            case 'p':
                return 6;
            case 'n':
                return 7;
            case 'b':
                return 8;
            case 'r':
                return 9;
            case 'q':
                return 10;
            case 'k':
                return 11;
            default:
                return -1;
            }
        }

        inline void set_bit(uint64_t &bb, uint8_t square)
        {
            bb |= (1ULL << square);
        }

        inline bool test_bit(uint64_t bb, uint8_t square) const
        {
            return (bb & (1ULL << square)) != 0;
        }

    public:
        explicit Board(const std::string &fen_string);
        ~Board() = default;

        Board(const Board &) = delete;
        Board &operator=(const Board &) = delete;
        Board(Board &&) = delete;
        Board &operator=(Board &&) = delete;

        /**
         * @brief Apply a move to the board and return the state needed to undo it.
         * @param move Move to apply.
         * @returns Undo state capturing pre-move board state.
         */
        UndoState apply_move(const Move &move);

        /**
         * @brief Get bitboard for a specific piece type.
         * @param index Bitboard index (0-5: White P/N/B/R/Q/K, 6-11: Black p/n/b/r/q/k).
         * @returns 64-bit bitboard representing piece positions.
         */
        uint64_t get_bitboard(int index) const override { return bitboards[index]; }

        /**
         * @brief Get legacy black occupancy bitboard.
         * @returns 64-bit bitboard with all black pieces.
         */
        uint64_t get_black_occupancy() const { return black_occupancy; }

        /**
         * @brief Get the castling rights string representation.
         * @returns Castling rights string (e.g., "KQkq", "-").
         */
        const std::string &get_castling_rights() const override;

        /**
         * @brief Get castling rights as a bitmask.
         * @returns Bitmask with castling rights flags (CASTLE_K, CASTLE_Q, CASTLE_k, CASTLE_q).
         */
        uint8_t get_castling_rights_mask() const { return castling_rights; }

        /**
         * @brief Get the side to move.
         * @returns Color value: 0 for white, 1 for black.
         */
        uint8_t get_color() const override { return color; }

        /**
         * @brief Get combined occupancy bitboard (all pieces, both colors).
         * @returns 64-bit bitboard with all occupied squares.
         */
        uint64_t get_combined_occupancy() const override { return combined_occupancy; }

        /**
         * @brief Get en passant target square as string.
         * @returns En passant square (e.g., "e3", "-").
         */
        const std::string &get_en_passant(void) const override;

        /**
         * @brief Get en passant target square as bitboard.
         * @returns Bitboard with en passant square set, or 0 if none.
         */
        uint64_t get_en_passant_bb() const override;

        /**
         * @brief Get en passant target square as index.
         * @returns En passant square index (64 = none).
         */
        uint8_t get_en_passant_square() const { return en_passant_square; }

        /**
         * @brief Get FEN notation of current board position.
         * @returns Full FEN string.
         */
        std::string get_fen(void);

        /**
         * @brief Get half-move clock value.
         * @returns Half-move clock (moves since last pawn move or capture).
         */
        uint8_t get_half_move_clock(void) const override { return half_move_clock; }

        /**
         * @brief Get occupancy bitboard for a specific color.
         * @param clr Color (0 for white, 1 for black).
         * @returns 64-bit bitboard with pieces of that color.
         */
        uint64_t get_occupancy(uint8_t clr) const override
        {
            return (clr == WHITE) ? white_occupancy : black_occupancy;
        }

        /**
         * @brief Get piece character at given board coordinates.
         * @param rank Rank index (0-7).
         * @param file File index (0-7).
         * @returns Piece character ('P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k', or ' ').
         */
        char get_piece(uint8_t rank, uint8_t file) const override;

        /**
         * @brief Get legacy white occupancy bitboard.
         * @returns 64-bit bitboard with all white pieces.
         */
        uint64_t get_white_occupancy() const { return white_occupancy; }

        /**
         * @brief Check if a color is in check.
         * @param clr Color to check (0 for white, 1 for black).
         * @returns True if in check, false otherwise.
         */
        bool is_in_check(uint8_t clr) const;

        /**
         * @brief Check if a square is attacked by the opponent.
         * @param square Square index to check.
         * @param attacker_color Attacking color.
         * @returns True if square is attacked, false otherwise.
         */
        bool is_square_attacked_by(uint8_t square, uint8_t attacker_color) const;

        /**
         * @brief Display the board position in text format.
         */
        void print(void) const;

        /**
         * @brief Reset board to a given FEN position.
         * @param fen_string FEN notation string.
         */
        void reset(const std::string &fen_string);

        /**
         * @brief Undo a previously applied move using saved state.
         * @param state UndoState from the previous apply_move call.
         */
        void undo_move(const UndoState &state);
    };
}
