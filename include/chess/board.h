/*
    board.h
    Author: M., Juan
    Date: 10/31/2023
*/

#ifndef BOARD_H
#define BOARD_H

#include <vector>

#include "include/chess/piece.h"

namespace Loki
{
    class Board
    {
        std::vector<std::vector<Piece *>> board;

    public:
        Board(const char *__placement);
        ~Board();

        // @methods
        std::vector<std::vector<Piece *>> get_board();
        void move(Piece *__piece, uint8_t __rank, uint8_t __file);
        void print();
    };
}

#endif