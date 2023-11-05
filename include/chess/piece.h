/*
    piece.h
    Author: M., Juan
    Date: 10/31/2023
*/

#ifndef PIECE_H
#define PIECE_H

#include <stdint.h>
#include <ctype.h>

namespace Loki
{
    class Piece
    {
        char alias;
        uint8_t value;
        uint8_t color;
        uint8_t rank;
        uint8_t file;

    public:
        Piece(char __alias, uint8_t __rank, uint8_t __file);
        ~Piece();

        // @getters
        char get_alias();
        uint8_t get_value();
        uint8_t get_color();
        uint8_t get_rank();
        void set_rank(uint8_t __rank);
        uint8_t get_file();
        void set_file(uint8_t __file);
    };
}

#endif