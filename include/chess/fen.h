/*
 *   Copyright (c) 2025 Juan Minor
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef FEN_H
#define FEN_H

#include <stdint.h>
#include <vector>

namespace loki
{
    class Fen
    {
    private:
        char *placement;
        char *castling;
        char *en_passant;
        uint8_t color;
        uint8_t halfmove_clock;
        uint8_t fullmoves;

        // Helper method to split a FEN string into components
        void __split__(const char *__fen, const char *__delimiters, std::vector<char *> &__tokens) const;
        void __validate_placement__(const char *__placement) const;

    public:
        Fen(const char *__fen);
        ~Fen();

        // Getters for FEN components
        char *get_placement(void) const;
        char *get_castling(void) const;
        char *get_en_passant(void) const;
        uint8_t get_color(void) const;
        uint8_t get_halfmove_clock(void) const;
        uint8_t get_fullmoves(void) const;
    };
}

#endif // FEN_H