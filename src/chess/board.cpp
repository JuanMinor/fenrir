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

#include <iostream>
#include <sstream>
#include <unordered_map>
#include "include/chess/board.h"
#include "include/core/core.h"
#include "include/logger/logger.h"
#include "include/modifier/modifier.h"
#include "include/pgn/pgn.h"

namespace fenrir
{
    Board::Board(const std::string &__placement, const std::string &__en_passant)
    {
        size_t size = __placement.size();
        this->en_passant = __en_passant;
        uint8_t rank = BOARD_SIZE - 1, file = 0, squares = 0;

        this->board.resize(BOARD_SIZE, std::vector<Piece *>(BOARD_SIZE, nullptr));

        for (size_t i = 0; i < size && rank < BOARD_SIZE; ++i)
        {
            char c = __placement[i];
            if (c == '/')
            {
                if (file != BOARD_SIZE)
                {
                    LOG_THROW_ERROR("Invalid FEN string: Rank does not have 8 squares", true);
                }
                rank--;
                file = 0;
                continue;
            }
            if (std::isdigit(c))
            {
                uint8_t empties = c - '0';
                for (uint8_t j = 0; j < empties; ++j)
                {
                    this->board[rank][file++] = nullptr;
                    squares++;
                }
                continue;
            }
            if (PIECE_NAMES.find(std::tolower(c, std::locale())) == PIECE_NAMES.end())
            {
                LOG_THROW_ERROR("Invalid FEN string: Unknown piece character", true);
            }
            squares++;
            Piece *piece = new Piece(c, rank, file);
            this->board[rank][file++] = piece;
            this->__log_piece_action__("Created", piece, utils::get_algebraic_notation(rank, file - 1), "✅");
        }

        if (file != BOARD_SIZE || rank != 0 || squares != 64)
        {
            LOG_THROW_ERROR("Invalid FEN string: Board could not be created correctly.", true);
        }
    }

    Board::~Board()
    {
        for (int rank = BOARD_SIZE - 1; rank >= 0; --rank)
        {
            for (uint8_t file = 0; file < BOARD_SIZE; ++file)
            {
                Piece *piece = this->board.at(rank).at(file);
                if (piece != nullptr)
                {
                    this->__log_piece_action__("Destroyed", piece, utils::get_algebraic_notation(rank, file), "❌");
                    delete piece;
                }
            }
        }
    }

    std::vector<std::vector<Piece *>> Board::get_board(void) const
    {
        return this->board;
    }

    const std::string Board::get_en_passant(void) const
    {
        return this->en_passant;
    }

    Piece *Board::get_piece(const uint8_t &__rank, const uint8_t &__file) const
    {
        if (__rank < 0 || __rank >= BOARD_SIZE || __file < 0 || __file >= BOARD_SIZE)
        {
            LOG_THROW_ERROR(
                ("Address <" + std::to_string(unsigned(__rank)) + ", " + std::to_string(unsigned(__file)) + "> is invalid").c_str(),
                false);
            return nullptr;
        }
        return this->board.at(__rank).at(__file);
    }

    void Board::move(Piece *&__piece, const uint8_t &__rank, const uint8_t &__file)
    {
        std::stringstream ss;
        if (__rank >= BOARD_SIZE || __file >= BOARD_SIZE)
        {
            LOG_THROW_ERROR(
                ("Address <" + std::to_string(unsigned(__rank)) + ", " + std::to_string(unsigned(__file)) + "> is invalid").c_str(),
                false);
            return;
        }
        uint8_t rank = __piece->get_rank(), file = __piece->get_file();
        if (this->board[rank][file] == nullptr)
        {
            LOG_THROW_ERROR(
                ("Piece at board address <" + std::to_string(unsigned(rank)) + ", " + std::to_string(unsigned(file)) + "> is invalid").c_str(),
                false);
            return;
        }

        if (this->board[__rank][__file] != nullptr &&
            this->board[__rank][__file]->get_color() == __piece->get_color())
        {
            LOG_THROW_ERROR(
                ("Cannot move to address <" + std::to_string(unsigned(__rank)) + ", " + std::to_string(unsigned(__file)) + ">").c_str(),
                false);
            return;
        }

        // En passant
        this->en_passant = "";
        if (std::tolower(__piece->get_alias()) == 'p' && std::abs(__rank - __piece->get_rank()) == 2)
        {
            this->en_passant = std::string(utils::get_algebraic_notation(
                (__rank + __piece->get_rank()) / 2,
                __piece->get_file()));
        }

        __piece->set_rank(__rank);
        __piece->set_file(__file);
        this->board[__rank][__file] = __piece;
        this->board[rank][file] = nullptr;

        this->__log_piece_action__("Moved", __piece, utils::get_algebraic_notation(__rank, __file), "🚀");
        io::PGN_RECORD(std::string(utils::get_algebraic_notation(rank, file)) + " " + utils::get_algebraic_notation(__rank, __file));
    }

    void Board::__log_piece_action__(const std::string &__action, const Piece *__piece, const std::string &__position, const std::string &__emoji)
    {
        const std::string color = __piece->get_color() == WHITE ? "white" : "black";
        std::stringstream ss;
        ss << __action << " " << color << " " << PIECE_NAMES.at(std::tolower(__piece->get_alias(), std::locale()))
           << " in position " << __position << " " << __emoji;
        logger::LOG_DEBUG(ss.str());
    }

    void Board::print(void) const
    {
        if (!DEBUG_ENABLED)
        {
            return;
        }
        std::cout << "    ";
        for (uint8_t j = 0; j < BOARD_SIZE; ++j)
        {
            std::cout << color::Modifier(color::Color::FG_YELLOW) << char(97 + j) << " ";
        }
        std::cout << color::Modifier(color::Color::RESET) << std::endl;
        for (int i = BOARD_SIZE - 1; i >= 0; i--)
        {
            std::cout << color::Modifier(color::Color::FG_YELLOW) << unsigned(i + 1) << " - "
                      << color::Modifier(color::Color::RESET);
            for (uint8_t j = 0; j < BOARD_SIZE; ++j)
            {
                if (this->board.at(i).at(j))
                {
                    std::cout << color::Modifier(this->board.at(i).at(j)->get_color() == BLACK
                                                     ? color::Color::FG_CYAN
                                                     : color::Color::RESET)
                              << this->board.at(i).at(j)->get_alias()
                              << color::Modifier(color::Color::RESET) << " ";
                    continue;
                }
                std::cout << ". ";
            }
            std::cout << color::Modifier(color::Color::FG_YELLOW) << "- " << unsigned(i + 1)
                      << color::Modifier(color::Color::RESET) << std::endl;
        }
        std::cout << "    ";
        for (uint8_t j = 0; j < BOARD_SIZE; ++j)
        {
            std::cout << color::Modifier(color::Color::FG_YELLOW) << char(97 + j) << " ";
        }
        std::cout << color::Modifier(color::Color::RESET) << std::endl;
    }
}
