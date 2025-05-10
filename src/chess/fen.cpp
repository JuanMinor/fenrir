/*
 *   Copyright (c) 2025 Juan Minor

 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <cstring>
#include <cstddef>
#include <cstdlib>
#include <regex>
#include "include/core/core.h"
#include "include/chess/fen.h"
#include "include/logger/logger.h"

namespace loki
{
    Fen::Fen(const char *__fen)
    {
        const std::regex fen_regex(
            "^(([rnbqkpRNBQKP1-8]+/){7}[rnbqkpRNBQKP1-8]+) [wb] (-|[KQkq]+) (-|[a-h][36]) (\\d+) (\\d+)$");
        if (!std::regex_match(__fen, fen_regex))
        {
            const char *error = "Invalid FEN string";
            logger::LOG_ERROR(error);
            throw std::invalid_argument(error);
        }

        // Split the FEN string into components
        std::vector<char *> tokens = std::vector<char *>();
        this->__split__(__fen, " ", tokens);
        if (tokens.size() != 6)
        {
            const char *error = "Invalid FEN string";
            logger::LOG_ERROR(error);
            throw std::invalid_argument(error);
        }

        // Validate the placement section
        this->__validate_placement__(tokens[0]);

        // Assign components to member variables
        this->placement = strdup(tokens[0]);
        this->color = (*tokens[1] == 'w' || *tokens[1] == 'W') ? WHITE : BLACK;
        this->castling = strdup(tokens[2]);
        this->en_passant = strdup(tokens[3]);
        this->halfmove_clock = static_cast<uint8_t>(std::stoi(tokens[4]));
        this->fullmoves = static_cast<uint8_t>(std::stoi(tokens[5]));

        // Free temporary tokens
        for (char *token : tokens)
        {
            free(token);
        }
    }

    Fen::~Fen() {}

    void Fen::__split__(const char *__fen, const char *__delimiters, std::vector<char *> &__tokens) const
    {
        __tokens.clear();
        char *fen_copy = strdup(__fen);
        char *token = strtok(fen_copy, __delimiters);
        while (token != nullptr)
        {
            __tokens.emplace_back(strdup(token));
            token = strtok(nullptr, __delimiters);
        }
        free(fen_copy);
        return;
    }

    void Fen::__validate_placement__(const char *__placement) const
    {
        int squares = 0;
        for (char c : std::string(__placement))
        {
            if (isdigit(c))
            {
                squares += c - '0';
            }
            else if (isalpha(c))
            {
                squares++;
            }
        }
        if (squares != 64)
        {
            const char *error = "Invalid FEN string: placement section does not represent 64 squares";
            logger::LOG_ERROR(error);
            throw std::invalid_argument(error);
        }
    }

    char *Fen::get_placement(void) const
    {
        return this->placement;
    }

    char *Fen::get_castling(void) const
    {
        return this->castling;
    }

    char *Fen::get_en_passant(void) const
    {
        return this->en_passant;
    }

    uint8_t Fen::get_color(void) const
    {
        return this->color;
    }

    uint8_t Fen::get_halfmove_clock(void) const
    {
        return this->halfmove_clock;
    }

    uint8_t Fen::get_fullmoves(void) const
    {
        return this->fullmoves;
    }
}