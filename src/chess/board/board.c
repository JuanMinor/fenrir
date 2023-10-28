//
//  board.c
//  @loki
//
//  Created by Juan Minor on 4/2/23.
//

#include "include/chess/board/board.h"

struct Board *create_board(const char *__placement)
{
    size_t size = strlen(__placement);
    uint8_t rank = 0, file = 0;

    // Initialize board.
    struct Board *board = malloc(sizeof(struct Board));

    // Iterate over string to get positions.
    for (size_t i = 0; i < size && rank < BOARD_SIZE; ++i)
    {
        char c = __placement[i];
        if (c == '/')
        {
            rank++;
            file = 0;
            continue;
        }
        if (isdigit(c))
        {
            uint8_t empty_squares = c - '0';
            for (uint8_t j = 0; j < empty_squares; ++j)
            {
                board->board[rank][file++] = NULL;
            }
            continue;
        }
        board->board[rank][file++] = create_piece(&c, rank, file);
    }

    return board;
}

void destroy_board(struct Board *__board)
{
    if (__board)
    {
        for (uint8_t i = 0; i < BOARD_SIZE; ++i)
        {
            for (uint8_t j = 0; j < BOARD_SIZE; ++j)
            {
                if (__board->board[i][j])
                    destroy_piece(__board->board[i][j]);
            }
        }
        free(__board);
    }
    return;
}

void print_board(const struct Board *__board)
{
    // Print piece char representation, ". " for empty squares.
    for (uint8_t i = 0; i < BOARD_SIZE; ++i)
    {
        for (uint8_t j = 0; j < BOARD_SIZE; ++j)
        {
            if (__board->board[i][j])
            {
                printf("%c ", *__board->board[i][j]->alias);
                continue;
            }
            printf(". ");
        }
        printf("\n");
    }
}
