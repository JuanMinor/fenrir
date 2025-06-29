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

#include "include/chess/fen.h"

namespace fenrir
{
    Fen::Fen(const std::string &__fen)
    {
        if (__fen.empty())
        {
            LOG_THROW_ERROR("FEN string cannot be empty", true);
        }
        const std::regex fen_regex(
            "^(([rnbqkpRNBQKP1-8]+/){7}[rnbqkpRNBQKP1-8]+) [wb] (-|[KQkq]+) (-|[a-h][36]) (\\d+) (\\d+)$");
        if (!std::regex_match(__fen, fen_regex))
        {
            LOG_THROW_ERROR("Invalid FEN string", true);
        }

        // Split the FEN string into components
        std::vector<std::string> tokens;
        this->__split__(__fen, " ", tokens);

        // Validate the placement section
        this->__validate_placement__(tokens[0]);

        // Assign components to member variables
        this->placement = tokens[0];
        this->color = (tokens[1] == "w" || tokens[1] == "W") ? WHITE : BLACK;
        this->castling = tokens[2];
        this->en_passant = tokens[3];
        this->halfmove_clock = static_cast<uint32_t>(std::stoi(tokens[4]));
        this->fullmoves = static_cast<uint32_t>(std::stoi(tokens[5]));

        logger::INFO("FEN initialized with: " + __fen);
    }

    Fen::~Fen() {}

    void Fen::__split__(const std::string &__fen, const std::string &__delimiters, std::vector<std::string> &__tokens) const
    {
        __tokens.clear();
        std::istringstream iss(__fen);
        std::string token;

        while (std::getline(iss, token, __delimiters[0])) // Assuming delimiter is a single char
        {
            __tokens.emplace_back(token);
        }
        return;
    }

    void Fen::__validate_placement__(const std::string &__placement) const
    {
        int squares = 0;
        for (char c : __placement)
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
            LOG_THROW_ERROR("Invalid FEN string: placement section does not represent 64 squares", true);
        }
    }

    // Getters for FEN components
    std::string Fen::get_placement(void) const
    {
        return this->placement;
    }

    std::string Fen::get_castling(void) const
    {
        return this->castling;
    }

    std::string Fen::get_en_passant(void) const
    {
        return this->en_passant;
    }

    uint8_t Fen::get_color(void) const
    {
        return this->color;
    }

    uint32_t Fen::get_halfmove_clock(void) const
    {
        return this->halfmove_clock;
    }

    uint32_t Fen::get_fullmoves(void) const
    {
        return this->fullmoves;
    }

    // Setters for FEN components
    void Fen::set_placement(const std::string &__placement)
    {
        std::regex placement_regex(
            "^(([rnbqkpRNBQKP1-8]+/){7}[rnbqkpRNBQKP1-8]+)$");

        if (!std::regex_match(__placement, placement_regex))
        {
            LOG_THROW_ERROR("Invalid placement section: " + __placement, true);
        }

        this->__validate_placement__(__placement);
        this->placement = __placement;
        return;
    }

    void Fen::set_castling(const std::string &__castling)
    {
        if (__castling != "-" && !std::regex_match(__castling, std::regex("^[KQkq]+$")))
        {
            LOG_THROW_ERROR("Invalid castling rights: " + __castling, true);
        }
        this->castling = __castling;
        return;
    }

    void Fen::set_en_passant(const std::string &__en_passant)
    {
        if (__en_passant != "-" && !std::regex_match(__en_passant, std::regex("^[a-h][36]$")))
        {
            LOG_THROW_ERROR("Invalid en passant square: " + __en_passant, true);
        }
        this->en_passant = __en_passant;
        return;
    }

    void Fen::set_color(const uint8_t &__color)
    {
        if (__color != WHITE && __color != BLACK)
        {
            LOG_THROW_ERROR("Invalid color value: " + std::to_string(__color), true);
        }
        this->color = __color;
        return;
    }

    void Fen::set_halfmove_clock(const uint32_t &__halfmove_clock)
    {
        this->halfmove_clock = __halfmove_clock;
        return;
    }

    void Fen::set_fullmoves(const uint32_t &__fullmoves)
    {
        if (__fullmoves == 0)
        {
            LOG_THROW_ERROR("Fullmoves must be at least 1: " + std::to_string(__fullmoves), true);
        }
        this->fullmoves = __fullmoves;
        return;
    }

    std::string Fen::generate_fen(void) const
    {
        std::ostringstream oss;
        oss << this->placement << " "
            << (this->color == WHITE ? "w" : "b") << " "
            << this->castling << " "
            << (this->en_passant.empty() ? "-" : this->en_passant) << " "
            << this->halfmove_clock << " "
            << this->fullmoves;
        return oss.str();
    }

}