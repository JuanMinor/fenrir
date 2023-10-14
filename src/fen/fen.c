//
//  fen.c
//  @loki
//
//  Created by Juan Minor on 3/17/23.
//

#include <stdio.h>

#include "include/fen/fen.h"
#include "include/string/string.h"

struct Fen *create_fen(const char *__fen)
{
    // Split Forsyth-Edwards Notation.
    size_t size = 0;
    char **components = split(__fen, " ", &size);

    // Size must be 6.
    if (size != 6)
        return NULL;

    // Build object.
    struct Fen *fen = malloc(sizeof(struct Fen));
    fen->placement = strdup(components[0]);
    fen->color = (*components[1] == 'w' || *components[1] == 'W') ? WHITE : BLACK;
    fen->castling = strdup(components[2]);
    fen->en_passant = strdup(components[3]);
    fen->half_moves = ((uint8_t)(*components[4] - '0'));
    fen->full_moves = ((uint8_t)(*components[5] - '0'));

    // Free allocated memory.
    for (size_t i = 0; i < size; i++)
    {
        free(components[i]);
    }
    free(components);

    return fen;
}

void destroy_fen(struct Fen *__fen)
{
    if (__fen)
    {
        if (__fen->placement)
            free(__fen->placement);
        if (__fen->castling)
            free(__fen->castling);
        if (__fen->en_passant)
            free(__fen->en_passant);
        free(__fen);
    }
    return;
}
