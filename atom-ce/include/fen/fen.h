//
//  fen.h
//  @atom-ce
//
//  Created by Juan Minor on 3/19/23.
//

#ifndef FEN_H
#define FEN_H

#include <ctype.h>
#include <string.h>

#include "include/core/core.h"

#define WHITE 1
#define BLACK 0

struct Fen
{
    char *placement;
    uint8_t color;
    char *castling;
    char *en_passant;
    uint8_t half_moves;
    uint8_t full_moves;
};

struct Fen *create_fen(const char *__fen);

void destroy_fen(struct Fen *__fen);

#endif
