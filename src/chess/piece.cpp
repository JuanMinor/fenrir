/*
    piece.cpp
    Author: M., Juan
    Date: 10/31/2023
*/

#include <unordered_map>

#include "include/chess/piece.h"
#include "include/core/core.h"

namespace Loki
{
    std::unordered_map<char, uint8_t>
        values = {
            {'P', 1},
            {'p', 1},
            {'N', 3},
            {'n', 3},
            {'B', 3},
            {'b', 3},
            {'R', 5},
            {'r', 5},
            {'Q', 9},
            {'q', 9},
            {'K', 25},
            {'k', 25}};

    Piece::Piece(char __alias, uint8_t __rank, uint8_t __file)
    {
        this->alias = __alias;
        this->value = values.at(__alias);
        this->color = isupper(__alias) ? WHITE : BLACK;
        this->rank = __rank;
        this->file = __file;
    }

    Piece::~Piece()
    {
    }

    char Piece::get_alias()
    {
        return this->alias;
    }

    uint8_t Piece::get_value()
    {
        return this->value;
    }

    uint8_t Piece::get_color()
    {
        return this->color;
    }

    uint8_t Piece::get_rank()
    {
        return this->rank;
    }

    void Piece::set_rank(uint8_t __rank)
    {
        this->rank = __rank;
    }

    uint8_t Piece::get_file()
    {
        return this->file;
    }

    void Piece::set_file(uint8_t __file)
    {
        this->file = __file;
    }
}