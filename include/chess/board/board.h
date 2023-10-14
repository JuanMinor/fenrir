//
//  board.h
//  @loki
//
//  Created by Juan Minor on 3/19/23.
//

#ifndef BOARD_H
#define BOARD_H

#include "include/core/core.h"
#include "include/string/string.h"
#include "include/chess/piece/piece.h"

#define BOARD_SIZE 8

struct Board {
	struct Piece* board[BOARD_SIZE][BOARD_SIZE];
};

struct Board *create_board(const char *__placement);

void destroy_board(struct Board *__board);

void print_board(const struct Board *__board);

#endif