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
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Create a new Fenrir chess engine instance.
     * @returns Opaque pointer to engine (void*). Must be freed with fenrir_destroy().
     */
    void *fenrir_create();

    /**
     * @brief Destroy a Fenrir chess engine instance and free its resources.
     * @param engine Engine pointer returned by fenrir_create().
     */
    void fenrir_destroy(void *engine);

    /**
     * @brief Generate all legal moves for the side to move.
     * @param engine Engine pointer.
     * @param out_buffer Buffer to write move strings (format: "e2e4 e7e5 ...").
     * @param max_len Maximum buffer size in bytes.
     * @returns Number of characters written, or -1 on error.
     */
    int fenrir_generate_all_moves(void *engine, char *out_buffer, size_t max_len);

    /**
     * @brief Get FEN notation of current board position.
     * @param engine Engine pointer.
     * @param out_buffer Buffer to write FEN string.
     * @param max_len Maximum buffer size in bytes.
     */
    void fenrir_get_fen(void *engine, char *out_buffer, size_t max_len);

    /**
     * @brief Make a move on the board.
     * @param engine Engine pointer.
     * @param from Source square in algebraic notation (e.g., "e2").
     * @param to Destination square in algebraic notation (e.g., "e4").
     * @returns True if move was legal and applied, false otherwise.
     */
    bool fenrir_make_move(void *engine, const char *from, const char *to);

    /**
     * @brief Print the current board state to stdout.
     * @param engine Engine pointer.
     */
    void fenrir_print_board(void *engine);

#ifdef __cplusplus
}
#endif
