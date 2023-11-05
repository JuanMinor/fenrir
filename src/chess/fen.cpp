/*
    fen.cpp
    Author: M., Juan
    Date: 10/31/2023
*/

#include <cstring>
#include <cstddef>

#include "include/chess/fen.h"
#include "include/core/core.h"

namespace Loki
{
    Fen::Fen(char *__fen)
    {
        std::vector<char *> tokens = this->__split__(__fen, " ");
        this->placement = tokens[0];
        this->color = (*tokens[1] == 'w' || *tokens[1] == 'W') ? WHITE : BLACK;
        this->castling = tokens[2];
        this->en_passant = tokens[3];
        this->h_moves = (uint8_t)(*tokens[4] - '0');
        this->f_moves = (uint8_t)(*tokens[5] - '0');
    }

    Fen::~Fen() {}

    std::vector<char *> Fen::__split__(char *__fen, const char *__delimiters)
    {
        std::vector<char *> tokens;
        char *token = strtok(__fen, __delimiters);
        while (token != nullptr)
        {
            tokens.emplace_back(token);
            token = strtok(nullptr, __delimiters);
        }
        return tokens;
    }

    char *Fen::get_placement()
    {
        return this->placement;
    }

    char *Fen::get_castling()
    {
        return this->castling;
    }

    char *Fen::get_en_passant()
    {
        return this->en_passant;
    }

    uint8_t Fen::get_color()
    {
        return this->color;
    }

    uint8_t Fen::get_h_moves()
    {
        return this->h_moves;
    }

    uint8_t Fen::get_f_moves()
    {
        return this->f_moves;
    }
}