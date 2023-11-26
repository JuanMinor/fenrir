/*
    board.h
    Author: M., Juan
    Date: 10/31/2023
*/

#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <vector>

#include "include/chess/piece.h"

namespace loki
{
    class Board
    {
        std::vector<std::vector<Piece *>> board;

        // @methods
        std::string __get_algebraic_notation__(const uint8_t &__rank, const uint8_t &__file);

    public:
        Board(const char *__placement);
        ~Board();

        // @methods
        std::vector<std::vector<Piece *>> get_board();
        void move(Piece *__piece, const uint8_t &__rank, const uint8_t &__file);
        void print(void);
    };
}

#endif