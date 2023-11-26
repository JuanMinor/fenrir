/*
    piece.h
    Author: M., Juan
    Date: 10/31/2023
*/

#ifndef PIECE_H
#define PIECE_H

#include <stdint.h>
#include <ctype.h>

namespace loki
{
    class Piece
    {
        char alias;
        uint8_t value;
        uint8_t color;
        uint8_t rank;
        uint8_t file;

    public:
        Piece(const char &__alias, const uint8_t &__rank, const uint8_t &__file);
        ~Piece();

        // @getters
        char get_alias(void) noexcept;
        uint8_t get_value(void) noexcept;
        uint8_t get_color(void) noexcept;
        uint8_t get_rank(void) noexcept;
        void set_rank(const uint8_t &__rank);
        uint8_t get_file(void) noexcept;
        void set_file(const uint8_t &__file);
    };
}

#endif