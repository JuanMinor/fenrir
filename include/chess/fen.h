/*
    fen.h
    Author: M., Juan
    Date: 10/31/2023
*/

#ifndef FEN_H
#define FEN_H

#include <stdint.h>
#include <vector>

namespace loki
{
    class Fen
    {
        char *placement;
        char *castling;
        char *en_passant;
        uint8_t color;
        uint8_t h_moves; // @half
        uint8_t f_moves; // @full
        // @methods
        std::vector<char *> __split__(char *__fen, const char *__delimiters) noexcept;

    public:
        Fen(char *__fen);
        ~Fen();

        // @getters
        char *get_placement(void) noexcept;
        char *get_castling(void) noexcept;
        char *get_en_passant(void) noexcept;
        uint8_t get_color(void) noexcept;
        uint8_t get_h_moves(void) noexcept;
        uint8_t get_f_moves(void) noexcept;
    };
}

#endif