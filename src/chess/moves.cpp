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

#include "include/chess/moves.h"

namespace fenrir
{
    Moves::Moves()
    {
        logger::INFO("Moves instance created");
    }

    Moves::~Moves() {}

    void Moves::__bishop__(const Piece *__piece, const Board *__board, std::vector<std::pair<const std::string, const std::string>> &__moves)
    {
        int8_t directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        this->__slide__(__piece, __board, __moves, directions, 4);
    }

    void Moves::__capture__(const Piece *__piece, const Piece *__target_piece, std::vector<std::pair<const std::string, const std::string>> &__moves)
    {
        if (__piece == nullptr || __target_piece == nullptr)
        {
            logger::WARN(!__piece ? "Piece is null 😢 and capture is not possible" : "Target piece is null 😢 and capture is not possible");
            return;
        }
        if (__piece->get_color() != __target_piece->get_color())
        {
            __moves.emplace_back(std::make_pair(utils::get_algebraic_notation(__piece->get_rank(), __piece->get_file()),
                                                utils::get_algebraic_notation(__target_piece->get_rank(), __target_piece->get_file())));
            logger::DEBUG("Capture move generated from " + utils::get_algebraic_notation(__piece->get_rank(), __piece->get_file()) +
                          " to " + utils::get_algebraic_notation(__target_piece->get_rank(), __target_piece->get_file()));
        }
    }

    void Moves::__king__(const Piece *__piece, const Board *__board, std::vector<std::pair<const std::string, const std::string>> &__moves)
    {
        constexpr int8_t direction_vectors[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
        this->__slide__(__piece, __board, __moves, direction_vectors, 8, true);
    }

    void Moves::__knight__(const Piece *__piece, const Board *__board, std::vector<std::pair<const std::string, const std::string>> &__moves)
    {
        const uint8_t current_rank = __piece->get_rank();
        const uint8_t current_file = __piece->get_file();
        const std::string from_position = utils::get_algebraic_notation(current_rank, current_file);

        constexpr int8_t knight_moves[8][2] = {
            {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

        for (const auto &move : knight_moves)
        {
            const int8_t new_rank = current_rank + move[0];
            const int8_t new_file = current_file + move[1];

            if (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
            {
                const Piece *target_piece = __board->get_piece(new_rank, new_file);
                if (!target_piece)
                {
                    __moves.emplace_back(from_position, utils::get_algebraic_notation(new_rank, new_file));
                    continue;
                }
                this->__capture__(__piece, target_piece, __moves);
            }
        }

        __log_generated_moves__(__piece, __moves);
    }

    void Moves::__log_generated_moves__(const Piece *__piece, const std::vector<std::pair<const std::string, const std::string>> &__moves) const
    {
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
        logger::DEBUG(ss.str());
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

        // Left diagonal capture
        if (file > 0)
        {
            this->__capture__(__piece, __board->get_piece(_rank, file - 1), __moves);
        }
        // Right diagonal capture
        if (file < BOARD_SIZE - 1)
        {
            this->__capture__(__piece, __board->get_piece(_rank, file + 1), __moves);
        }

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

    void Moves::__queen__(const Piece *__piece, const Board *__board, std::vector<std::pair<const std::string, const std::string>> &__moves)
    {
        constexpr int8_t direction_vectors[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
        this->__slide__(__piece, __board, __moves, direction_vectors, 8);
    }

    void Moves::__rook__(const Piece *__piece, const Board *__board, std::vector<std::pair<const std::string, const std::string>> &__moves)
    {
        constexpr int8_t direction_vectors[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        this->__slide__(__piece, __board, __moves, direction_vectors, 4);
    }

    void Moves::__slide__(const Piece *__piece, const Board *__board, std::vector<std::pair<const std::string, const std::string>> &__moves, const int8_t direction_vectors[][2], size_t num_directions, bool __single_depth)
    {
        const uint8_t current_rank = __piece->get_rank();
        const uint8_t current_file = __piece->get_file();
        const std::string from_position = utils::get_algebraic_notation(current_rank, current_file);

        for (size_t i = 0; i < num_directions; ++i)
        {
            const int8_t rank_delta = direction_vectors[i][0];
            const int8_t file_delta = direction_vectors[i][1];

            int8_t _rank = current_rank + rank_delta;
            int8_t _file = current_file + file_delta;
            uint8_t depth = 0;

            while (_rank >= 0 && _rank <= 7 && _file >= 0 && _file <= 7)
            {
                if (__single_depth && depth == 1)
                {
                    break;
                }
                depth++;
                const Piece *target_piece = __board->get_piece(_rank, _file);
                if (!target_piece)
                {
                    __moves.emplace_back(from_position, utils::get_algebraic_notation(_rank, _file));
                    _rank += rank_delta;
                    _file += file_delta;
                    continue;
                }
                this->__capture__(__piece, target_piece, __moves);
                break;
            }
        }

        __log_generated_moves__(__piece, __moves);
    }

    // Public
    Moves &Moves::get_instance()
    {
        static Moves instance;
        logger::DEBUG("Moves instance created and returned. Only one instance will be used throughout the application.");
        return instance;
    }

    void Moves::generate_moves(const Piece *__piece, const Board *__board, std::vector<std::pair<const std::string, const std::string>> &__moves)
    {
        if (__piece == nullptr)
        {
            logger::ERROR("Piece is null. Moves cannot be generated 😢");
            return;
        }
        logger::DEBUG("Generating moves for piece at " + utils::get_algebraic_notation(__piece->get_rank(), __piece->get_file()));
        switch (std::tolower(__piece->get_alias()))
        {
        case 'b':
            this->__bishop__(__piece, __board, __moves);
            break;
        case 'k':
            this->__king__(__piece, __board, __moves);
            break;
        case 'n':
            this->__knight__(__piece, __board, __moves);
            break;
        case 'p':
            this->__pawn__(__piece, __board, __moves);
            break;
        case 'q':
            this->__queen__(__piece, __board, __moves);
            break;
        case 'r':
            this->__rook__(__piece, __board, __moves);
            break;
        }
    }
}
