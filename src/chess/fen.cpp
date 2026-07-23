/*
 *   Copyright (c) 2026 Juan Minor
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

#include "include/chess/fen.h"

namespace chess
{
    Fen::Fen(const std::string &fen_string)
        : placement(""), castling(""), en_passant(""), color(WHITE), half_move_clock(0), full_moves(1)
    {
        if (fen_string.empty())
        {
            LOG_THROW_ERROR("FEN string cannot be empty", true);
        }
        const std::regex fen_regex(
            "^(([rnbqkpRNBQKP1-8]+/){7}[rnbqkpRNBQKP1-8]+) [wb] (-|[KQkq]+) (-|[a-h][36]) (\\d+) (\\d+)$");
        if (!std::regex_match(fen_string, fen_regex))
        {
            LOG_THROW_ERROR("Invalid FEN string", true);
        }

        std::vector<std::string> tokens;
        this->split_string(fen_string, " ", tokens);

        this->validate_placement(tokens[0]);

        this->placement = tokens[0];
        this->color = (tokens[1] == "w" || tokens[1] == "W") ? WHITE : BLACK;
        this->castling = tokens[2];
        this->en_passant = tokens[3];
        this->half_move_clock = static_cast<uint32_t>(std::stoi(tokens[4]));
        this->full_moves = static_cast<uint32_t>(std::stoi(tokens[5]));

        if (this->half_move_clock > 100)
        {
            LOG_THROW_ERROR("Invalid FEN: halfmove clock cannot exceed 100 (50-move rule)", true);
        }
        if (this->full_moves == 0)
        {
            LOG_THROW_ERROR("Invalid FEN: fullmoves must be at least 1", true);
        }

        /* DEBUG, not INFO: constructed per thread-engine in the search hot
         * path; INFO would cost a filesystem write per construction. */
        logger::DEBUG("FEN initialized: " + fen_string);
    }

    Fen::~Fen() {}

    void Fen::split_string(const std::string &fen_string, const std::string &delimiters, std::vector<std::string> &tokens) const
    {
        tokens.clear();
        std::istringstream iss(fen_string);
        std::string token;

        while (std::getline(iss, token, delimiters[0]))
        {
            tokens.emplace_back(token);
        }
        return;
    }

    void Fen::validate_chess_rules(const std::string &placement_string) const
    {
        std::unordered_map<char, uint8_t> piece_counts = {
            {'K', static_cast<uint8_t>(0)}, {'k', static_cast<uint8_t>(0)}, {'Q', static_cast<uint8_t>(0)}, {'q', static_cast<uint8_t>(0)}, {'R', static_cast<uint8_t>(0)}, {'r', static_cast<uint8_t>(0)}, {'B', static_cast<uint8_t>(0)}, {'b', static_cast<uint8_t>(0)}, {'N', static_cast<uint8_t>(0)}, {'n', static_cast<uint8_t>(0)}, {'P', static_cast<uint8_t>(0)}, {'p', static_cast<uint8_t>(0)}};

        for (char c : placement_string)
        {
            if (isalpha(c))
            {
                if (piece_counts.find(c) != piece_counts.end())
                {
                    piece_counts[c]++;
                }
            }
        }

        if (!utils::are_chess_piece_count_rules_valid(piece_counts))
        {
            LOG_THROW_ERROR("FEN placement does not comply with chess piece count rules", true);
        }

        std::vector<std::string> ranks;
        this->split_string(placement_string, "/", ranks);

        this->validate_pawn_placement(ranks);
        this->validate_king_safety(ranks);
    }

    void Fen::validate_king_safety(const std::vector<std::string> &ranks) const
    {
        int white_king_rank = -1, white_king_file = -1;
        int black_king_rank = -1, black_king_file = -1;

        for (int rank = 0; rank < chess::BOARD_SIZE; rank++)
        {
            int file = 0;
            for (char c : ranks[static_cast<size_t>(rank)])

            {
                if (isdigit(c))
                {
                    file += c - '0';
                    continue;
                }
                if (c == 'K')
                {
                    white_king_rank = rank;
                    white_king_file = file;
                    file++;
                    continue;
                }
                if (c == 'k')
                {
                    black_king_rank = rank;
                    black_king_file = file;
                    file++;
                    continue;
                }
                file++;
            }
        }

        uint8_t rank_diff = static_cast<uint8_t>(std::abs(white_king_rank - black_king_rank));
        uint8_t file_diff = static_cast<uint8_t>(std::abs(white_king_file - black_king_file));

        if (rank_diff <= 1 && file_diff <= 1 && !(rank_diff == 0 && file_diff == 0))
        {
            LOG_THROW_ERROR("Invalid FEN: kings cannot be adjacent to each other", true);
        }
    }

    void Fen::validate_pawn_placement(const std::vector<std::string> &ranks) const
    {
        for (char c : ranks[0])
        {
            if (c == 'P')
            {
                LOG_THROW_ERROR("Invalid FEN: white pawns must promote upon reaching 8th rank", true);
            }
        }

        for (char c : ranks[7])
        {
            if (c == 'p')
            {
                LOG_THROW_ERROR("Invalid FEN: black pawns must promote upon reaching 1st rank", true);
            }
        }
    }

    void Fen::validate_placement(const std::string &placement_string) const
    {
        int squares = 0;
        for (char c : placement_string)
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

        this->validate_chess_rules(placement_string);
    }

    /**
     * @brief Generate FEN string from current position state.
     * @returns Complete FEN notation string.
     */
    std::string Fen::generate_fen(void) const
    {
        std::ostringstream oss;
        oss << this->placement << " "
            << (this->color == WHITE ? "w" : "b") << " "
            << this->castling << " "
            << (this->en_passant.empty() ? "-" : this->en_passant) << " "
            << this->half_move_clock << " "
            << this->full_moves;
        return oss.str();
    }

    /**
     * @brief Get castling rights string representation.
     * @returns Castling rights (e.g., "KQkq", "-").
     */
    std::string Fen::get_castling(void) const
    {
        return this->castling;
    }

    /**
     * @brief Get color (side to move).
     * @returns Color value: 0 for white, 1 for black.
     */
    uint8_t Fen::get_color(void) const
    {
        return this->color;
    }

    /**
     * @brief Get en passant target square.
     * @returns En passant square (e.g., "e3", "-").
     */
    std::string Fen::get_en_passant(void) const
    {
        return this->en_passant;
    }

    /**
     * @brief Get full move number (increments each time black moves).
     * @returns Full move count.
     */
    uint32_t Fen::get_full_moves(void) const
    {
        return this->full_moves;
    }

    /**
     * @brief Get half-move clock (moves since last pawn move or capture).
     * @returns Half-move clock value (0-50 for draw rule).
     */
    uint32_t Fen::get_half_move_clock(void) const
    {
        return this->half_move_clock;
    }

    /**
     * @brief Get board placement string (the rank/file portion of FEN).
     * @returns Placement string (e.g., "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR").
     */
    std::string Fen::get_placement(void) const
    {
        return this->placement;
    }

    /**
     * @brief Set castling rights.
     * @param castling_rights Castling rights string (e.g., "KQkq", "-").
     */
    void Fen::set_castling(const std::string &castling_rights)
    {
        if (castling_rights != "-" && !std::regex_match(castling_rights, std::regex("^[KQkq]+$")))
        {
            LOG_THROW_ERROR("Invalid castling rights: " + castling_rights, true);
        }
        this->castling = castling_rights;
        return;
    }

    /**
     * @brief Set color (side to move).
     * @param color_value Color value: 0 for white, 1 for black.
     */
    void Fen::set_color(uint8_t color_value)
    {
        if (color_value != WHITE && color_value != BLACK)
        {
            LOG_THROW_ERROR("Invalid color value: " + std::to_string(color_value), true);
        }
        this->color = color_value;
        return;
    }

    /**
     * @brief Set en passant target square.
     * @param en_passant_square En passant square (e.g., "e3", "-").
     */
    void Fen::set_en_passant(const std::string &en_passant_square)
    {
        if (en_passant_square != "-" && !std::regex_match(en_passant_square, std::regex("^[a-h][36]$")))
        {
            LOG_THROW_ERROR("Invalid en passant square: " + en_passant_square, true);
        }
        this->en_passant = en_passant_square;
        return;
    }

    /**
     * @brief Set full move number (increments each time black moves).
     * @param full_moves_value Full move count.
     */
    void Fen::set_full_moves(uint32_t full_moves_value)
    {
        if (full_moves_value == 0)
        {
            LOG_THROW_ERROR("Full moves must be at least 1: " + std::to_string(full_moves_value), true);
        }
        this->full_moves = full_moves_value;
        return;
    }

    /**
     * @brief Set half-move clock (moves since last pawn move or capture).
     * @param half_move_clock_value Half-move clock value (0-50 for draw rule).
     */
    void Fen::set_half_move_clock(uint32_t half_move_clock_value)
    {
        if (half_move_clock_value > 100)
        {
            LOG_THROW_ERROR("Invalid half move clock: cannot exceed 100 (50-move rule)", true);
        }
        this->half_move_clock = half_move_clock_value;
        return;
    }

    /**
     * @brief Set board placement string (rank/file portion of FEN).
     * @param placement_string Placement string (e.g., "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR").
     */
    void Fen::set_placement(const std::string &placement_string)
    {
        std::regex placement_regex(
            "^(([rnbqkpRNBQKP1-8]+/){7}[rnbqkpRNBQKP1-8]+)$");

        if (!std::regex_match(placement_string, placement_regex))
        {
            LOG_THROW_ERROR("Invalid placement section: " + placement_string, true);
        }

        this->validate_placement(placement_string);
        this->placement = placement_string;
        return;
    }

}
