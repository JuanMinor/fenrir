//
//  piece.h
//  @atom-ce
//
//  Created by Juan Minor on 3/20/23.
//

#ifndef PIECE_H
#define PIECE_H

#include "include/core/core.h"

struct Piece
{
    char *alias;
    uint8_t value;
};

uint8_t __get_piece_value__(const char *__alias);

struct Piece *create_piece(const char *__alias);

void destroy_piece(struct Piece *__piece);

#endif
