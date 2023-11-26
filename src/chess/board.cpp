/*
    board.cpp
    Author: M., Juan
    Date: 10/31/2023
*/

#include <cstring>
#include <iostream>
#include <stdint.h>

#include "include/chess/board.h"
#include "include/core/core.h"
#include "include/logger/logger.h"
#include "include/pgn/pgn.h"

namespace loki
{
    Board::Board(const char *__placement)
    {
        size_t size = std::strlen(__placement);
        uint8_t rank = 0, file = 0;
        this->board.resize(BOARD_SIZE, std::vector<Piece *>(BOARD_SIZE, nullptr));

        for (size_t i = 0; i < size && rank < BOARD_SIZE; ++i)
        {
            char c = __placement[i];
            if (c == '/')
            {
                rank++;
                file = 0;
                continue;
            }
            if (std::isdigit(c))
            {
                uint8_t empties = c - '0';
                for (uint8_t j = 0; j < empties; ++j)
                {
                    this->board[rank][file++] = nullptr;
                }
                continue;
            }
            this->board[rank][file++] = new Piece(c, rank, file);
            std::stringstream ss;
            ss << "Created piece with alias '" << c << "' with <rank, file> <" << unsigned(rank) << ", " << unsigned(file) << ">!";
            logger::LOG_DEBUG(ss.str());
        }
        logger::LOG_INFO("Created chess board!");
    }

    Board::~Board()
    {
        for (uint8_t rank = 0; rank < BOARD_SIZE; ++rank)
        {
            for (uint8_t file = 0; file < BOARD_SIZE; ++file)
            {
                Piece *piece = this->board[rank][file];
                if (piece != nullptr)
                {
                    std::stringstream ss;
                    ss << "Destroyed piece with alias '" << piece->get_alias() << "' with <rank, file> <" << unsigned(piece->get_rank()) << ", " << unsigned(piece->get_file()) << ">!";
                    logger::LOG_DEBUG(ss.str());
                    delete piece;
                }
            }
        }
        logger::LOG_INFO("Destroyed chess board and it's components!");
    }

    std::string Board::__get_algebraic_notation__(const uint8_t &__rank, const uint8_t &__file)
    {
        std::stringstream ss;
        ss << char(97 + __rank) << unsigned(__file);
        return ss.str();
    }

    std::vector<std::vector<Piece *>> Board::get_board()
    {
        return this->board;
    }

    void Board::move(Piece *__piece, const uint8_t &__rank, const uint8_t &__file)
    {
        std::stringstream ss;
        // @errors
        if (__rank < 0 || __rank >= BOARD_SIZE || __file < 0 || __file >= BOARD_SIZE)
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

        // @move
        __piece->set_rank(__rank);
        __piece->set_file(__file);
        this->board[__rank][__file] = __piece;
        this->board[rank][file] = nullptr;

        ss << "Moved piece with alias '" << __piece->get_alias()
           << "' from <rank, file> <" << unsigned(rank) << ", "
           << unsigned(file) << "> to <" << unsigned(__rank)
           << ", " << unsigned(__file) << ">!";
        logger::LOG_INFO(ss.str());

        // @pgn
        ss.str("");
        ss << this->__get_algebraic_notation__(rank, file) << " " << this->__get_algebraic_notation__(__rank, __file);
        pgn::RECORD(ss.str());
        return;
    }

    void Board::print(void)
    {
        if (!DEBUG_ENABLED)
        {
            return;
        }
        for (uint8_t i = 0; i < BOARD_SIZE; ++i)
        {
            for (uint8_t j = 0; j < BOARD_SIZE; ++j)
            {
                if (this->board[i][j])
                {
                    std::cout << this->board[i][j]->get_alias() << " ";
                    continue;
                }
                std::cout << ". ";
            }
            std::cout << std::endl;
        }
    }
}
