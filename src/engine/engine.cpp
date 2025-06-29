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

#include "include/engine/engine.h"

namespace fenrir
{

    Engine::Engine(const std::string &__fen) : fen(__fen), board(__fen)
    {
        logger::INFO("Engine initialized with FEN: " + __fen);
    }

    Engine::~Engine() {}

#ifndef NDEBUG
    char Engine::get_piece(const std::string &__algebraic_address) const
    {
        uint8_t rank, file;
        utils::parse_algebraic_notation(__algebraic_address, rank, file);

        const Piece *piece = board.get_piece(rank, file);
        if (piece == nullptr)
        {
            return '.'; // Empty square
        }
        return piece->get_alias();
    }
#endif

    std::vector<std::pair<const std::string, const std::string>> Engine::generate_moves(const std::string &__algebraic_address) const
    {
        std::vector<std::pair<const std::string, const std::string>> moves;
        u_int8_t rank, file;
        utils::parse_algebraic_notation(__algebraic_address, rank, file);
        if (!board.get_board().at(rank).at(file))
        {
            LOG_THROW_ERROR(
                (std::string("Board address ") + __algebraic_address + " does not contain a piece").c_str(),
                false);
            return moves;
        }
        const Piece *piece = board.get_piece(rank, file);

        Moves::get_instance().generate_moves(piece, &board, moves);

        logger::DEBUG("Generated moves for piece at address: " + __algebraic_address);

        return moves;
    }

    std::string Engine::get_fen(void)
    {
        std::string current_fen = board.get_fen();
        logger::DEBUG("Current FEN: " + current_fen);
        return current_fen;
    }

    void Engine::make_move(const std::string &__from_algebraic_address, const std::string &__to_algebraic_address)
    {
        u_int8_t __from_rank, __from_file, __to_rank, __to_file;
        utils::parse_algebraic_notation(__from_algebraic_address, __from_rank, __from_file);
        utils::parse_algebraic_notation(__to_algebraic_address, __to_rank, __to_file);

        Piece *piece = board.get_board().at(__from_rank).at(__from_file);
        if (!piece)
        {
            logger::ERROR("No piece found at " + __from_algebraic_address);
            return;
        }
        board.move(piece, __to_rank, __to_file);

        logger::DEBUG("Made move from " + __from_algebraic_address + " to " + __to_algebraic_address);
    }

    void Engine::print_board(void) const
    {
        board.print();
    }

    void Engine::reset()
    {
        board.~Board();
        new (&board) Board(fen);
        logger::INFO("Reset the board to the initial state with FEN: " + fen);
    }
}
