/*
    main.cpp
    Author: M., Juan
    Date: 10/31/2023
*/

#include <iostream>

#include "include/chess/board.h"
#include "include/chess/fen.h"
#include "include/pgn/pgn.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "No arguments found. Supply a valid FEN string" << std::endl;
        return 1;
    }

    loki::Fen *fen = new loki::Fen(argv[1]);

    loki::Board *board = new loki::Board(fen->get_placement());

    board->print();

    board->move(board->get_board()[0][0], 5, 5);

    board->print();

    pgn::CREATE_PGN();

    delete board;

    return 0;
}
