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
        {'p', "Pawn"},
        {'n', "Knight"},
        {'b', "Bishop"},
        {'r', "Rook"},
        {'q', "Queen"},
        {'k', "King"}};

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
                    const char *error = "Invalid FEN string: Rank does not have 8 squares";
                    logger::LOG_ERROR(error);
                    throw std::invalid_argument(error);
                }
                rank--;
                file = 0;
                continue;
            }
            if (std::isdigit(c))
            {
                uint8_t empties = c - '0';
                squares += empties;

                for (uint8_t j = 0; j < empties; ++j)
                {
                    this->board[rank][file++] = nullptr;
                }
                continue;
            }
            if (piece_names.find(std::tolower(c, std::locale())) == piece_names.end())
            {
                throw std::invalid_argument("Invalid FEN string: Unknown piece character");
            }
            squares++;
            Piece *piece = new Piece(c, rank, file);
            this->board[rank][file++] = piece;
            std::stringstream ss;
            std::string color = piece->get_color() == WHITE ? "White" : "Black";
            ss << "Created "
               << color
               << " "
               << piece_names.at(std::tolower(c, std::locale()))
               << " in position "
               << this->__get_algebraic_notation__(piece->get_rank(), piece->get_file())
               << " ✅";
            logger::LOG_DEBUG(ss.str());
        }

        if (file != BOARD_SIZE)
        {
            const char *error = "Invalid FEN string: Rank does not have 8 squares";
            logger::LOG_ERROR(error);
            throw std::invalid_argument(error);
        }

        if (rank != 0)
        {
            const char *error = "Invalid FEN string: Board does not have 8 ranks";
            logger::LOG_ERROR(error);
            throw std::invalid_argument(error);
        }

        if (squares != 64)
        {
            const char *error = "Invalid FEN string: Board does not have 64 squares";
            logger::LOG_ERROR(error);
            throw std::invalid_argument(error);
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
                    std::stringstream ss;
                    std::string color = piece->get_color() == WHITE ? "White" : "Black";
                    ss << "Destroyed "
                       << color
                       << " "
                       << piece_names.at(std::tolower(piece->get_alias(), std::locale()))
                       << " in position "
                       << this->__get_algebraic_notation__(rank, file)
                       << " ❌";
                    logger::LOG_DEBUG(ss.str());
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
            ss << "Board address <rank, file> <" << unsigned(__rank) << ", " << unsigned(__file) << "> is out of bounds!";
            logger::LOG_ERROR(ss.str());
            return;
        }
        uint8_t rank = __piece->get_rank(), file = __piece->get_file();
        if (this->board[rank][file] == nullptr)
        {
            ss << "Piece holds invalid address <rank, file> <" << unsigned(rank) << ", " << unsigned(file) << ">!";
            logger::LOG_ERROR(ss.str());
            return;
        }

        if (this->board[__rank][__file] != nullptr &&
            this->board[__rank][__file]->get_color() == __piece->get_color())
        {
            ss << "Cannot move to <rank, file> <" << unsigned(__rank) << ", " << unsigned(__file)
               << "> because it is occupied by a piece of the same color!";
            logger::LOG_ERROR(ss.str());
            return;
        }

        __piece->set_rank(__rank);
        __piece->set_file(__file);
        this->board[__rank][__file] = __piece;
        this->board[rank][file] = nullptr;

        std::string color = __piece->get_color() == WHITE ? "White" : "Black";

        ss << "Moved "
           << color
           << " "
           << piece_names.at(std::tolower(__piece->get_alias(), std::locale()))
           << " from "
           << this->__get_algebraic_notation__(rank, file)
           << " to "
           << this->__get_algebraic_notation__(__rank, __file)
           << " 🚀";

        logger::LOG_INFO(ss.str());

        ss.str("");
        ss << this->__get_algebraic_notation__(rank, file)
           << " "
           << this->__get_algebraic_notation__(__rank, __file);
        io::PGN_RECORD(ss.str());
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
