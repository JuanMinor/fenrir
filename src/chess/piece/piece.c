//
//  piece.c
//  @loki
//
//  Created by Juan Minor on 3/20/23.
//

#include "include/chess/piece/piece.h"
#include "include/string/string.h"

/* private functions */
uint8_t __get_piece_value__(const char *__alias)
{
    static const uint8_t piece_values[128] = {
        ['P'] = 1, ['p'] = 1, ['N'] = 3, ['n'] = 3, ['B'] = 3, ['b'] = 3, ['R'] = 5, ['r'] = 5, ['Q'] = 9, ['q'] = 9, ['K'] = 25, ['k'] = 25};

    return piece_values[(unsigned char)*__alias];
}

/* constructor */
struct Piece *create_piece(const char *__alias, uint8_t __rank, uint8_t __file)
{
    struct Piece *piece = malloc(sizeof(struct Piece));
    piece->alias = strdup(__alias);
    piece->value = __get_piece_value__(__alias);
    piece->rank = __rank;
    piece->file = __file;
    return piece;
}

void destroy_piece(struct Piece *__piece)
{
    if (__piece)
    {
        if (__piece->alias)
            free(__piece->alias);
        free(__piece);
    }
    return;
}
