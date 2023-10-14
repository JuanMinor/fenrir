//
//  chess.c
//  @loki
//
//  Created by Juan Minor on 3/19/23.
//

#include "include/chess/chess.h"

void draw(const char* __fen_str)
{
    // @draw board in terminal
    struct Fen *fen = create_fen(__fen_str);
    struct Board *board = create_board(fen->placement);

    print_board(board);

    // @cleanup
    destroy_board(board);
    destroy_fen(fen);

    return;
}