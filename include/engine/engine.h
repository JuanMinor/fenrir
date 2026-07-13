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

#include "include/chess/board.h"
#include "include/core/core.h"
#include "include/logger/logger.h"
#include "include/chess/moves.h"
#include "include/utils/utils.h"
#include "include/abstract/board.h"

namespace fenrir
{

    class FENRIR_API Engine final
    {
        static constexpr const char *DEFAULT_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        std::string fen;
        Board board;

        std::vector<UndoState> undo_stack;

    public:
        Engine(const std::string &fen_string = DEFAULT_FEN);
        ~Engine();

        /**
         * @brief Check if current position is checkmate.
         * @returns True if the side to move is in checkmate, false otherwise.
         */
        bool is_checkmate();

        /**
         * @brief Check if current position is a draw (50-move rule or stalemate).
         * @returns True if game is drawn, false otherwise.
         */
        bool is_draw();

        /**
         * @brief Check if current position is stalemate.
         * @returns True if the side to move is stalemated, false otherwise.
         */
        bool is_stalemate();

        /**
         * @brief Generate all legal moves for the side to move.
         * @returns Vector of all legal moves in the current position.
         */
        std::vector<Move> generate_all_moves();

        /**
         * @brief Generate all legal moves for a piece at a specific square.
         * @param algebraic_address Square in algebraic notation (e.g., "e2").
         * @returns Vector of legal moves for the piece on that square.
         */
        std::vector<Move> generate_moves(const std::string &algebraic_address);

        /**
         * @brief Get combined terminal state information in a single query.
         * @returns TerminalState struct with is_terminal flag and game score.
         */
        struct TerminalState
        {
            bool is_terminal;
            double score; // 0.0 = loss for side to move, 0.5 = draw
        };
        TerminalState get_terminal_state();

        /**
         * @brief Get a const view of the board for inspection (e.g., by evaluators).
         * @returns Const reference to the AbstractBoard interface.
         */
        const AbstractBoard &get_board_view() const;

        /**
         * @brief Get FEN notation of current board position.
         * @returns Full FEN string.
         */
        std::string get_fen(void);

        /**
         * @brief Make a move on the board with full validation.
         * @param move Move to apply.
         */
        void make_move(const Move &move);

        /**
         * @brief Make a move without recording undo state (for performance-critical paths).
         * @param move Move to apply.
         */
        void make_move_fast(const Move &move);

        /**
         * @brief Print the current board state in text format.
         */
        void print_board(void) const;

        /**
         * @brief Reset the board to the starting position.
         */
        void reset();

        /**
         * @brief Undo the last move made.
         */
        void undo_move();

        /**
         * @brief Get the library version string.
         * @returns Version string (e.g., "0.3.0").
         */
        static const char *version();
    };
}
