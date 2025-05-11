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

#include <cstring>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include "include/chess/board.h"
#include "include/core/core.h"
#include "include/logger/logger.h"
#include "include/modifier/modifier.h"
#include "include/pgn/pgn.h"

namespace loki
{
    std::unordered_map<char, std::string> piece_names = {
        {'p', "pawn"},
        {'n', "knight"},
        {'b', "bishop"},
        {'r', "rook"},
        {'q', "queen"},
        {'k', "king"}};

    Board::Board(const char *__placement)
    {
        size_t size = std::strlen(__placement);
        uint8_t rank = BOARD_SIZE - 1, file = 0, squares = 0;

        this->board.resize(BOARD_SIZE, std::vector<Piece *>(BOARD_SIZE, nullptr));

        for (size_t i = 0; i < size && rank < BOARD_SIZE; ++i)
        {
            char c = __placement[i];
            if (c == '/')
            {
                if (file != BOARD_SIZE)
                {
                    this->__log_throw_error__("Invalid FEN string: Rank does not have 8 squares", true);
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
            if (piece_names.find(std::tolower(c, std::locale())) == piece_names.end())
            {
                this->__log_throw_error__("Invalid FEN string: Unknown piece character", true);
            }
            squares++;
            Piece *piece = new Piece(c, rank, file);
            this->board[rank][file++] = piece;
            this->__log_piece_action__("Created", piece, this->__get_algebraic_notation__(rank, file - 1).c_str(), "✅");
        }

        if (file != BOARD_SIZE || rank != 0 || squares != 64)
        {
            this->__log_throw_error__("Invalid FEN string: Board could not be created correctly.", true);
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
                    this->__log_piece_action__("Destroyed", piece, this->__get_algebraic_notation__(rank, file).c_str(), "❌");
                    delete piece;
                }
            }
        }
    }

    std::string Board::__get_algebraic_notation__(const uint8_t &__rank, const uint8_t &__file) const
    {
        std::stringstream ss;
        ss << char(97 + __file) << unsigned(__rank + 1);
        return ss.str();
    }

    std::vector<std::vector<Piece *>> Board::get_board(void) const
    {
        return this->board;
    }

    void Board::move(Piece *&__piece, const uint8_t &__rank, const uint8_t &__file)
    {
        std::stringstream ss;
        if (__rank >= BOARD_SIZE || __file >= BOARD_SIZE)
        {
            this->__log_throw_error__(
                ("Address <" + std::to_string(unsigned(__rank)) + ", " + std::to_string(unsigned(__file)) + "> is out of bounds!").c_str());
            return;
        }
        uint8_t rank = __piece->get_rank(), file = __piece->get_file();
        if (this->board[rank][file] == nullptr)
        {
            this->__log_throw_error__(
                ("Piece at board address <" + std::to_string(unsigned(rank)) + ", " + std::to_string(unsigned(file)) + "> is invalid").c_str());
            return;
        }

        if (this->board[__rank][__file] != nullptr &&
            this->board[__rank][__file]->get_color() == __piece->get_color())
        {
            this->__log_throw_error__(
                ("Cannot move to address <" + std::to_string(unsigned(__rank)) + ", " + std::to_string(unsigned(__file)) + ">").c_str());
            return;
        }

        __piece->set_rank(__rank);
        __piece->set_file(__file);
        this->board[__rank][__file] = __piece;
        this->board[rank][file] = nullptr;

        this->__log_piece_action__("Moved", __piece, this->__get_algebraic_notation__(__rank, __file).c_str(), "🚀");
        io::PGN_RECORD((this->__get_algebraic_notation__(rank, file) + " " + this->__get_algebraic_notation__(__rank, __file)).c_str());
    }

    void Board::__log_piece_action__(const char *__action, const Piece *__piece, const char *__position, const char *__emoji)
    {
        const char *color = __piece->get_color() == WHITE ? "white" : "black";
        std::stringstream ss;
        ss << __action << " " << color << " " << piece_names.at(std::tolower(__piece->get_alias(), std::locale()))
           << " in position " << __position << " " << __emoji;
        logger::LOG_DEBUG(ss.str());
    }

    void Board::__log_throw_error__(const char *__error, const bool &__throw)
    {
        logger::LOG_ERROR(__error);
        if (__throw)
        {
            throw std::runtime_error(__error);
        }
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
