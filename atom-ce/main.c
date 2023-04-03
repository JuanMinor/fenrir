//
//  main.c
//  @atom-ce
//
//  Created by Juan Minor on 3/19/23.
//

#include "include/chess/chess.h"
#include "include/fen/fen.h"
#include "include/string/string.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("No arguments found. Please supply a valid FEN string.\n");
        return 1;
    }
    if (argv[1] && !trim(argv[1])[0])
    {
        printf("FEN string cannot be empty or null.\n");
        return 2;
    }

    struct Fen *fen = create_fen(argv[1]);

    // Create board.
    struct Board *board = create_board(fen->placement);

    print_board(board);

    destroy_board(board);

    destroy_fen(fen);

    return 0;
}
