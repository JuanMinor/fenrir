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

#include "include/chess/board.h"

namespace fenrir
{
    Board::Board(const std::string &__fen)
        : fen(__fen)
    {
        this->castling = fen.get_castling();
        this->en_passant = fen.get_en_passant() == "-" ? "" : fen.get_en_passant();
        this->color = fen.get_color();
        this->halfmove_clock = fen.get_halfmove_clock();
        this->fullmoves = fen.get_fullmoves();

        this->__build_board__(fen.get_placement());

        logger::INFO("Board initialized with FEN: " + fen.get_placement());
    }

    Board::~Board()
    {
        for (int rank = BOARD_SIZE - 1; rank >= 0; --rank)
        {
            for (uint8_t file = 0; file < BOARD_SIZE; ++file)
            {
                Piece *piece = this->board.at(rank).at(file);
                if (piece != nullptr)
                {
                    this->__log_piece_action__("Destroyed", piece, utils::get_algebraic_notation(rank, file), "❌");
                    delete piece;
                }
            }
        }
        logger::INFO("Board destroyed and all pieces cleaned up.");
    }

    /* Private */
    void Board::__build_board__(const std::string &__placement)
    {
        size_t size = __placement.size();
        uint8_t rank = BOARD_SIZE - 1, file = 0, squares = 0;

        this->board.resize(BOARD_SIZE, std::vector<Piece *>(BOARD_SIZE, nullptr));

        for (size_t i = 0; i < size && rank < BOARD_SIZE; ++i)
        {
            char c = __placement[i];
            if (c == '/')
            {
                rank--;
                file = 0;
                continue;
            }
            if (std::isdigit(c))
            {
                uint8_t empties = c - '0';
                for (uint8_t j = 0; j < empties; ++j)
                {
                    this->board[rank][file++] = nullptr;
                }
                continue;
            }
            Piece *piece = new Piece(c, rank, file);
            this->board[rank][file++] = piece;
            this->__log_piece_action__("Created", piece, utils::get_algebraic_notation(rank, file - 1), "✅");
        }
    }

    std::string Board::__generate_placement_from_board__(void) const
    {
        std::string placement;
        placement.reserve(80);

        for (int rank = BOARD_SIZE - 1; rank >= 0; --rank)
        {
            uint8_t empty_squares = 0;
            for (uint8_t file = 0; file < BOARD_SIZE; ++file)
            {
                const Piece *piece = this->board[rank][file];
                if (!piece)
                {
                    ++empty_squares;
                }
                else
                {
                    if (empty_squares)
                    {
                        placement += static_cast<char>('0' + empty_squares);
                        empty_squares = 0;
                    }
                    placement += piece->get_alias();
                }
            }
            if (empty_squares)
            {
                placement += static_cast<char>('0' + empty_squares);
            }
            if (rank)
            {
                placement += '/';
            }
        }
        return placement;
    }

    void Board::__log_piece_action__(const std::string &__action, const Piece *__piece, const std::string &__position, const std::string &__emoji)
    {
        const std::string color = __piece->get_color() == WHITE ? "white" : "black";
        std::stringstream oss;
        oss << __action << " " << color << " " << PIECE_NAMES.at(std::tolower(__piece->get_alias(), std::locale()))
            << " in position " << __position << " " << __emoji;

        // Use stringstream as fallback for std::format compatibility
        std::stringstream detailed_oss;
        detailed_oss << oss.str() << " (" << __piece->get_value() << " value) at <"
                     << unsigned(__piece->get_rank()) << ", " << unsigned(__piece->get_file()) << ">";

        logger::DEBUG(detailed_oss.str());
    }

    std::vector<std::vector<Piece *>> Board::get_board(void) const
    {
        return this->board;
    }

    std::string Board::get_fen(void)
    {
        fen.set_placement(this->__generate_placement_from_board__());
        fen.set_castling(this->castling.empty() ? "-" : this->castling);
        fen.set_en_passant(this->en_passant.empty() ? "-" : this->en_passant);
        fen.set_color(this->color);
        fen.set_halfmove_clock(this->halfmove_clock);
        fen.set_fullmoves(this->fullmoves);

        return fen.generate_fen();
    }

    const std::string Board::get_en_passant(void) const
    {
        return this->en_passant;
    }

    Piece *Board::get_piece(const uint8_t &__rank, const uint8_t &__file) const
    {
        if (__rank < 0 || __rank >= BOARD_SIZE || __file < 0 || __file >= BOARD_SIZE)
        {
            LOG_THROW_ERROR(
                ("Address <" + std::to_string(unsigned(__rank)) + ", " + std::to_string(unsigned(__file)) + "> is invalid").c_str(),
                false);
            return nullptr;
        }
        return this->board.at(__rank).at(__file);
    }

    void Board::move(Piece *&__piece, const uint8_t &__rank, const uint8_t &__file)
    {
        if (__rank >= BOARD_SIZE || __file >= BOARD_SIZE)
        {
            LOG_THROW_ERROR(
                ("Address <" + std::to_string(unsigned(__rank)) + ", " + std::to_string(unsigned(__file)) + "> is invalid").c_str(),
                false);
            return;
        }
        uint8_t rank = __piece->get_rank(), file = __piece->get_file();
        if (this->board[rank][file] == nullptr)
        {
            LOG_THROW_ERROR(
                ("Piece at board address <" + std::to_string(unsigned(rank)) + ", " + std::to_string(unsigned(file)) + "> is invalid").c_str(),
                false);
            return;
        }

        if (this->board[__rank][__file] != nullptr &&
            this->board[__rank][__file]->get_color() == __piece->get_color())
        {
            LOG_THROW_ERROR(
                ("Cannot move to address <" + std::to_string(unsigned(__rank)) + ", " + std::to_string(unsigned(__file)) + ">").c_str(),
                false);
            return;
        }

        // En passant
        this->en_passant = "";
        if (std::tolower(__piece->get_alias()) == 'p' && std::abs(__rank - __piece->get_rank()) == 2)
        {
            this->en_passant = std::string(utils::get_algebraic_notation(
                (__rank + __piece->get_rank()) / 2,
                __piece->get_file()));
        }

        __piece->set_rank(__rank);
        __piece->set_file(__file);
        this->board[__rank][__file] = __piece;
        this->board[rank][file] = nullptr;

        this->__log_piece_action__("Moved", __piece, utils::get_algebraic_notation(__rank, __file), "🚀");
        io::PGN_RECORD(std::string(utils::get_algebraic_notation(rank, file)) + " " + utils::get_algebraic_notation(__rank, __file));
    }

    void Board::print(void) const
    {
        if (!DEBUG)
        {
            return;
        }
        std::cout << "    ";
        for (uint8_t j = 0; j < BOARD_SIZE; ++j)
        {
            std::cout << color::Modifier(color::Color::FG_YELLOW) << char(97 + j) << " ";
        }
        std::cout << color::Modifier(color::Color::RESET) << std::endl;
        for (int i = BOARD_SIZE - 1; i >= 0; i--)
        {
            std::cout << color::Modifier(color::Color::FG_YELLOW) << unsigned(i + 1) << " - "
                      << color::Modifier(color::Color::RESET);
            for (uint8_t j = 0; j < BOARD_SIZE; ++j)
            {
                if (this->board.at(i).at(j))
                {
                    std::cout << color::Modifier(this->board.at(i).at(j)->get_color() == BLACK
                                                     ? color::Color::FG_CYAN
                                                     : color::Color::RESET)
                              << this->board.at(i).at(j)->get_alias()
                              << color::Modifier(color::Color::RESET) << " ";
                    continue;
                }
                std::cout << ". ";
            }
            std::cout << color::Modifier(color::Color::FG_YELLOW) << "- " << unsigned(i + 1)
                      << color::Modifier(color::Color::RESET) << std::endl;
        }
        std::cout << "    ";
        for (uint8_t j = 0; j < BOARD_SIZE; ++j)
        {
            std::cout << color::Modifier(color::Color::FG_YELLOW) << char(97 + j) << " ";
        }
        std::cout << color::Modifier(color::Color::RESET) << std::endl;
    }
}
