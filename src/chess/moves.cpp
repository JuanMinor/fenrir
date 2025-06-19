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

#include <sstream>
#include "include/chess/moves.h"
#include "include/utils/utils.h"

namespace fenrir
{
    Moves::Moves() {}

    Moves::~Moves() {}

    void Moves::__capture__(const Piece *__piece, const Piece *__target_piece, std::vector<std::pair<const std::string, const std::string>> &__moves)
    {
        if (__piece == nullptr || __target_piece == nullptr)
        {
            return;
        }
        if (__piece->get_color() != __target_piece->get_color())
        {
            __moves.emplace_back(std::make_pair(utils::get_algebraic_notation(__piece->get_rank(), __piece->get_file()),
                                                utils::get_algebraic_notation(__target_piece->get_rank(), __target_piece->get_file())));
        }
    }

    void Moves::__log_generated_moves__(const Piece *__piece, const std::vector<std::pair<const std::string, const std::string>> &__moves) const
    {
        if (!__piece)
        {
            if (DEBUG_ENABLED)
            {
                logger::LOG_DEBUG("Piece is null. Generated moves cannot be logged 😢");
            }
            return;
        }
        std::stringstream ss;
        ss << "Generated moves for "
           << (__piece->get_color() == WHITE ? "white" : "black")
           << " "
           << PIECE_NAMES.at(std::tolower(__piece->get_alias(), std::locale()))
           << " at " << utils::get_algebraic_notation(__piece->get_rank(), __piece->get_file())
           << ": ";

        if (__moves.empty())
        {
            ss << "No moves were generated 😢";
        }

        for (size_t i = 0; i < __moves.size(); ++i)
        {
            ss << __moves.at(i).first << "->" << __moves.at(i).second;
            if (i != __moves.size() - 1)
            {
                ss << ", ";
            }
        }
        if (DEBUG_ENABLED)
        {
            logger::LOG_DEBUG(ss.str());
        }
    }

    void Moves::__pawn__(const Piece *__piece, const Board *__board, std::vector<std::pair<const std::string, const std::string>> &__moves)
    {
        uint8_t rank = __piece->get_rank();
        uint8_t file = __piece->get_file();
        uint8_t color = __piece->get_color();
        uint8_t direction = (color == WHITE) ? 1 : -1;

        uint8_t _rank = rank + direction;
        if (_rank >= 0 && _rank < BOARD_SIZE && !__board->get_piece(_rank, file))
        {
            __moves.emplace_back(std::make_pair(utils::get_algebraic_notation(rank, file),
                                                utils::get_algebraic_notation(_rank, file)));
        }

        this->__capture__(__piece, __board->get_piece(_rank, file - 1), __moves);
        this->__capture__(__piece, __board->get_piece(_rank, file + 1), __moves);

        if (!__piece->get_moved())
        {
            _rank = rank + (2 * direction);
            if (_rank >= 0 && _rank < BOARD_SIZE && !__board->get_piece(_rank, file))
            {
                __moves.emplace_back(std::make_pair(utils::get_algebraic_notation(rank, file),
                                                    utils::get_algebraic_notation(_rank, file)));
            }
        }

        // En passant
        const std::string &en_passant = __board->get_en_passant();
        if (!en_passant.empty())
        {
            uint8_t en_passant_rank, en_passant_file;
            utils::parse_algebraic_notation(en_passant.c_str(), en_passant_rank, en_passant_file);
            if (en_passant_rank == _rank && std::abs(int(en_passant_file) - int(file)) == 1)
            {
                const Piece *target_piece = __board->get_piece(rank, en_passant_file);
                if (target_piece && std::tolower(target_piece->get_alias()) == 'p')
                {
                    __moves.emplace_back(
                        std::make_pair(
                            utils::get_algebraic_notation(rank, file),
                            utils::get_algebraic_notation(en_passant_rank, en_passant_file)));
                }
            }
        }

        __log_generated_moves__(__piece, __moves);
    }

    // Public
    Moves &Moves::get_instance()
    {
        static Moves instance;
        return instance;
    }

    void Moves::generate_moves(const Piece *__piece, const Board *__board, std::vector<std::pair<const std::string, const std::string>> &__moves)
    {
        if (__piece == nullptr)
        {
            return;
        }
        switch (std::tolower(__piece->get_alias()))
        {
        case 'p':
            this->__pawn__(__piece, __board, __moves);
            break;
        // Add cases for other pieces (N, B, R, Q, K) here
        default:
            break;
        }
    }
}
