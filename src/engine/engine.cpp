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

#include "include/engine/engine.h"
#include "include/chess/attacks.h"

#ifndef FENRIR_VERSION
#define FENRIR_VERSION "0.0.0-unknown"
#endif

namespace chess
{

    Engine::Engine(const std::string &fen_string) : fen(fen_string), board(fen_string)
    {
        init_attack_tables();
        /* DEBUG, not INFO: the MCTS search constructs one Engine per worker
         * thread per move (28+ at production settings); at INFO level each
         * construction cost a log-file open/stat/write/flush, which stalled
         * self-play to ~10% GPU utilization on network-volume hosts. */
        logger::DEBUG("Engine initialized with FEN: " + fen_string);
    }

    Engine::~Engine() {}

    /**
     * @brief Generate all legal moves for the side to move.
     * @returns Vector of all legal moves in the current position.
     */
    std::vector<Move> Engine::generate_all_moves()
    {
        uint8_t active_color = board.get_color();
        std::vector<Move> all_moves;
        /* Avoid per-piece heap allocations */
        std::vector<Move> piece_moves;
        piece_moves.reserve(32);

        /* Collect moves for active pieces */
        for (uint8_t rank = 0; rank < BOARD_SIZE; ++rank)
        {
            for (uint8_t file = 0; file < BOARD_SIZE; ++file)
            {
                char piece = board.get_piece(rank, file);
                if (piece == '\0')
                {
                    continue;
                }
                bool piece_is_white = std::isupper(static_cast<unsigned char>(piece));
                if ((active_color == WHITE) != piece_is_white)
                {
                    continue;
                }

                piece_moves.clear();
                Moves::generate_moves(rank, file, board, piece_moves);

                uint8_t opp_color = (active_color == WHITE) ? BLACK : WHITE;
                for (const Move &m : piece_moves)
                {
                    if (m.get_move_type() == MoveType::CASTLE_KINGSIDE || m.get_move_type() == MoveType::CASTLE_QUEENSIDE)
                    {
                        if (board.is_in_check(active_color))
                            continue;
                        uint8_t base_rank = (active_color == WHITE) ? 0 : 7;
                        if (m.get_move_type() == MoveType::CASTLE_KINGSIDE)
                        {
                            if (board.is_square_attacked_by(static_cast<uint8_t>(base_rank * 8 + 5), opp_color))
                                continue;
                        }
                        else
                        {
                            if (board.is_square_attacked_by(static_cast<uint8_t>(base_rank * 8 + 3), opp_color))
                                continue;
                        }
                    }

                    UndoState undo = board.apply_move(m);
                    bool in_check = board.is_in_check(active_color);
                    board.undo_move(undo);
                    if (!in_check)
                    {
                        all_moves.push_back(m);
                    }
                }
            }
        }

        return all_moves;
    }

    /**
     * @brief Generate all legal moves for a piece at a specific square.
     * @param algebraic_address Square in algebraic notation (e.g., "e2").
     * @returns Vector of legal moves for the piece on that square.
     */
    std::vector<Move> Engine::generate_moves(const std::string &algebraic_address)
    {
        std::vector<Move> pseudo_legal;
        uint8_t rank, file;
        utils::parse_algebraic_notation(algebraic_address, rank, file);
        char piece_char = board.get_piece(rank, file);
        if (piece_char == '\0')
        {
            LOG_THROW_ERROR(
                (std::string("Board address ") + algebraic_address + " does not contain a piece").c_str(),
                false);
            return pseudo_legal;
        }

        Moves::generate_moves(rank, file, board, pseudo_legal);

        /* Filter pseudo-legal moves via check verification */
        bool active_is_white = std::isupper(static_cast<unsigned char>(piece_char));
        uint8_t active_color = active_is_white ? WHITE : BLACK;

        std::vector<Move> legal;
        legal.reserve(pseudo_legal.size());

        uint8_t opp_color = (active_color == WHITE) ? BLACK : WHITE;
        for (const Move &m : pseudo_legal)
        {
            if (m.get_move_type() == MoveType::CASTLE_KINGSIDE || m.get_move_type() == MoveType::CASTLE_QUEENSIDE)
            {
                if (board.is_in_check(active_color))
                    continue;
                uint8_t base_rank = (active_color == WHITE) ? 0 : 7;
                if (m.get_move_type() == MoveType::CASTLE_KINGSIDE)
                {
                    if (board.is_square_attacked_by(static_cast<uint8_t>(base_rank * 8 + 5), opp_color))
                        continue;
                }
                else
                {
                    if (board.is_square_attacked_by(static_cast<uint8_t>(base_rank * 8 + 3), opp_color))
                        continue;
                }
            }

            UndoState undo = board.apply_move(m);
            bool in_check = board.is_in_check(active_color);
            board.undo_move(undo);
            if (!in_check)
            {
                legal.push_back(m);
            }
        }
        return legal;
    }

    /**
     * @brief Get a const view of the board for inspection (e.g., by evaluators).
     * @returns Const reference to the AbstractBoard interface.
     */
    const AbstractBoard &Engine::get_board_view() const
    {
        return board;
    }

    /**
     * @brief Get FEN notation of current board position.
     * @returns Full FEN string.
     */
    std::string Engine::get_fen(void)
    {
        std::string current_fen = board.get_fen();
        return current_fen;
    }

    /**
     * @brief Get combined terminal state information in a single query.
     * @returns TerminalState struct with is_terminal flag and game score.
     */
    Engine::TerminalState Engine::get_terminal_state()
    {
        if (board.get_half_move_clock() >= 100)
        {
            return {true, 0.5};
        }

        {
            UndoState current_state;
            for (int i = 0; i < 12; ++i)
                current_state.bitboards[i] = board.get_bitboard(i);
            current_state.castling_rights = board.get_castling_rights_mask();
            current_state.en_passant_square = board.get_en_passant_square();
            current_state.color = board.get_color();

            int repetitions = 1;
            int limit = board.get_half_move_clock();
            int stack_size = static_cast<int>(undo_stack.size());
            int lookback = (limit < stack_size) ? limit : stack_size;

            for (int i = 0; i < lookback; ++i)
            {
                const UndoState &past = undo_stack[static_cast<size_t>(stack_size - 1 - i)];
                bool match = true;
                for (int j = 0; j < 12; ++j)
                {
                    if (past.bitboards[j] != current_state.bitboards[j])
                    {
                        match = false;
                        break;
                    }
                }
                if (match &&
                    past.castling_rights == current_state.castling_rights &&
                    past.en_passant_square == current_state.en_passant_square &&
                    past.color == current_state.color)
                {
                    if (++repetitions >= 3)
                        return {true, 0.5};
                }
            }
        }

        auto all_moves = generate_all_moves();
        if (all_moves.empty())
        {
            uint8_t active_color = board.get_color();
            if (board.is_in_check(active_color))
                return {true, 0.0};
            else
                return {true, 0.5};
        }

        return {false, 0.5};
    }

    /**
     * @brief Check if current position is checkmate.
     * @returns True if the side to move is in checkmate, false otherwise.
     */
    bool Engine::is_checkmate()
    {
        uint8_t active_color = board.get_color();
        if (!board.is_in_check(active_color))
        {
            return false;
        }
        /* Verify legal moves exist */
        auto all_moves = generate_all_moves();
        return all_moves.empty();
    }

    /**
     * @brief Check if current position is a draw (50-move rule or stalemate).
     * @returns True if game is drawn, false otherwise.
     */
    bool Engine::is_draw()
    {
        if (is_stalemate())
        {
            return true;
        }

        if (board.get_half_move_clock() >= 100)
        {
            return true;
        }

        UndoState current_state;
        for (int i = 0; i < 12; ++i)
        {
            current_state.bitboards[i] = board.get_bitboard(i);
        }
        current_state.castling_rights = board.get_castling_rights_mask();
        current_state.en_passant_square = board.get_en_passant_square();
        current_state.color = board.get_color();

        int repetitions = 1;
        int limit = board.get_half_move_clock();
        int stack_size = static_cast<int>(undo_stack.size());
        int lookback = (limit < stack_size) ? limit : stack_size;

        for (int i = 0; i < lookback; ++i)
        {
            const UndoState &past_state = undo_stack[static_cast<size_t>(stack_size - 1 - i)];
            bool match = true;
            for (int j = 0; j < 12; ++j)
            {
                if (past_state.bitboards[j] != current_state.bitboards[j])
                {
                    match = false;
                    break;
                }
            }
            if (match &&
                past_state.castling_rights == current_state.castling_rights &&
                past_state.en_passant_square == current_state.en_passant_square &&
                past_state.color == current_state.color)
            {
                repetitions++;
                if (repetitions >= 3)
                {
                    return true;
                }
            }
        }

        return false;
    }

    /**
     * @brief Check if current position is stalemate.
     * @returns True if the side to move is stalemated, false otherwise.
     */
    bool Engine::is_stalemate()
    {
        uint8_t active_color = board.get_color();
        if (board.is_in_check(active_color))
        {
            return false;
        }
        /* Verify legal moves exist */
        auto all_moves = generate_all_moves();
        return all_moves.empty();
    }

    /**
     * @brief Make a move on the board with full validation.
     * @param move Move to apply.
     */
    void Engine::make_move(const Move &move)
    {
        /* Generate legal moves */
        std::vector<Move> legal_moves = generate_all_moves();

        /* Match move */
        bool found = false;
        Move matched_move = move;
        for (const auto &m : legal_moves)
        {
            if (m.get_from_square() == move.get_from_square() &&
                m.get_to_square() == move.get_to_square() &&
                m.get_promotion_piece() == move.get_promotion_piece())
            {
                matched_move = m;
                found = true;
                break;
            }
        }

        if (!found)
        {
            logger::ERROR("Illegal or out-of-turn move requested: " + move.get_from() + " -> " + move.get_to());
            throw std::invalid_argument("Illegal or out-of-turn move requested: " + move.get_from() + " -> " + move.get_to());
        }

        UndoState state = board.apply_move(matched_move);
        undo_stack.push_back(state);

        logger::DEBUG("Made move from " + matched_move.get_from() + " to " + matched_move.get_to());
    }

    /**
     * @brief Make a move without legality validation (for performance-critical
     * search paths). Undo state is still recorded; passing a move that is
     * not legal in the current position corrupts the board.
     * @param move Move to apply. Must be legal.
     */
    void Engine::make_move_fast(const Move &move)
    {
        UndoState state = board.apply_move(move);
        undo_stack.push_back(state);
    }

    void Engine::print_board(void) const
    {
        board.print();
    }

    /**
     * @brief Reset the board to the starting position.
     */
    void Engine::reset()
    {
        board.reset(fen);
        undo_stack.clear();
        logger::INFO("Reset the board to the initial state with FEN: " + fen);
    }

    /**
     * @brief Undo the last move made.
     */
    void Engine::undo_move()
    {
        if (undo_stack.empty())
        {
            logger::WARN("No moves to undo");
            return;
        }
        const UndoState &state = undo_stack.back();
        board.undo_move(state);
        undo_stack.pop_back();
        logger::DEBUG("Undid last move");
    }

    /**
     * @brief Get the library version string.
     * @returns Version string (e.g., "0.3.0").
     */
    const char *Engine::version()
    {
        return FENRIR_VERSION;
    }
}
