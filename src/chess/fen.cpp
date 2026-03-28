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
	Fen::Fen(const std::string &fenString, GameMode gameMode) : gameMode(gameMode)
	{
		if (fenString.empty())
		{
			LOG_THROW_ERROR("FEN string cannot be empty", true);
		}
		const std::regex fen_regex(
			"^(([rnbqkpRNBQKP1-8]+/){7}[rnbqkpRNBQKP1-8]+) [wb] (-|[KQkq]+) (-|[a-h][36]) (\\d+) (\\d+)$");
		if (!std::regex_match(fenString, fen_regex))
		{
			LOG_THROW_ERROR("Invalid FEN string", true);
		}

		std::vector<std::string> tokens;
		this->splitString(fenString, " ", tokens);

		this->validatePlacement(tokens[0]);

		this->placement = tokens[0];
		this->color = (tokens[1] == "w" || tokens[1] == "W") ? WHITE : BLACK;
		this->castling = tokens[2];
		this->enPassant = tokens[3];
		this->halfMoveClock = static_cast<uint32_t>(std::stoi(tokens[4]));
		this->fullMoves = static_cast<uint32_t>(std::stoi(tokens[5]));

		if (gameMode == GameMode::TOURNAMENT)
		{
			if (this->halfMoveClock > 100)
			{
				LOG_THROW_ERROR("Invalid FEN: halfmove clock cannot exceed 100 (50-move rule)", true);
			}
			if (this->fullMoves == 0)
			{
				LOG_THROW_ERROR("Invalid FEN: fullmoves must be at least 1", true);
			}
		}

		const std::string mode_str = (gameMode == GameMode::PERMISSIVE) ? "PERMISSIVE" : "TOURNAMENT";
		logger::INFO("FEN initialized with " + mode_str + " mode: " + fenString);
	}

	Fen::~Fen() {}

	void Fen::splitString(const std::string &fenString, const std::string &delimiters, std::vector<std::string> &tokens) const
	{
		tokens.clear();
		std::istringstream iss(fenString);
		std::string token;

		while (std::getline(iss, token, delimiters[0]))
		{
			tokens.emplace_back(token);
		}
		return;
	}

	void Fen::validateChessRules(const std::string &placement) const
	{
		std::unordered_map<char, uint8_t> piece_counts = {
			{'K', 0}, {'k', 0}, {'Q', 0}, {'q', 0}, {'R', 0}, {'r', 0}, {'B', 0}, {'b', 0}, {'N', 0}, {'n', 0}, {'P', 0}, {'p', 0}};

		for (char c : placement)
		{
			if (isalpha(c))
			{
				if (piece_counts.find(c) != piece_counts.end())
				{
					piece_counts[c]++;
				}
			}
		}

		if (!utils::areChessPieceCountRulesValid(piece_counts))
		{
			LOG_THROW_ERROR("FEN placement does not comply with chess piece count rules", true);
		}

		std::vector<std::string> ranks;
		this->splitString(placement, "/", ranks);

		this->validatePawnPlacement(ranks);
		this->validateKingSafety(ranks);
	}

	void Fen::validatePawnPlacement(const std::vector<std::string> &ranks) const
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

	void Fen::validateKingSafety(const std::vector<std::string> &ranks) const
	{
		int white_king_rank = -1, white_king_file = -1;
		int black_king_rank = -1, black_king_file = -1;

		for (int rank = 0; rank < fenrir::BOARD_SIZE; rank++)
		{
			int file = 0;
			for (char c : ranks[rank])
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

		uint8_t rank_diff = abs(white_king_rank - black_king_rank);
		uint8_t file_diff = abs(white_king_file - black_king_file);

		if (rank_diff <= 1 && file_diff <= 1 && !(rank_diff == 0 && file_diff == 0))
		{
			LOG_THROW_ERROR("Invalid FEN: kings cannot be adjacent to each other", true);
		}
	}

	void Fen::validatePlacement(const std::string &placement) const
	{
		int squares = 0;
		for (char c : placement)
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

		if (this->gameMode == GameMode::TOURNAMENT)
		{
			this->validateChessRules(placement);
		}
	}

	std::string Fen::getPlacement(void) const
	{
		return this->placement;
	}

	std::string Fen::getCastling(void) const
	{
		return this->castling;
	}

	std::string Fen::getEnPassant(void) const
	{
		return this->enPassant;
	}

	uint8_t Fen::getColor(void) const
	{
		return this->color;
	}

	uint32_t Fen::getHalfMoveClock(void) const
	{
		return this->halfMoveClock;
	}

	uint32_t Fen::getFullMoves(void) const
	{
		return this->fullMoves;
	}

	void Fen::setPlacement(const std::string &placement)
	{
		std::regex placement_regex(
			"^(([rnbqkpRNBQKP1-8]+/){7}[rnbqkpRNBQKP1-8]+)$");

		if (!std::regex_match(placement, placement_regex))
		{
			LOG_THROW_ERROR("Invalid placement section: " + placement, true);
		}

		this->validatePlacement(placement);
		this->placement = placement;
		return;
	}

	void Fen::setCastling(const std::string &castling)
	{
		if (castling != "-" && !std::regex_match(castling, std::regex("^[KQkq]+$")))
		{
			LOG_THROW_ERROR("Invalid castling rights: " + castling, true);
		}
		this->castling = castling;
		return;
	}

	void Fen::setEnPassant(const std::string &enPassant)
	{
		if (enPassant != "-" && !std::regex_match(enPassant, std::regex("^[a-h][36]$")))
		{
			LOG_THROW_ERROR("Invalid en passant square: " + enPassant, true);
		}
		this->enPassant = enPassant;
		return;
	}

	void Fen::setColor(const uint8_t &color)
	{
		if (color != WHITE && color != BLACK)
		{
			LOG_THROW_ERROR("Invalid color value: " + std::to_string(color), true);
		}
		this->color = color;
		return;
	}

	void Fen::setHalfMoveClock(const uint32_t &halfMoveClock)
	{
		if (this->gameMode == GameMode::TOURNAMENT && halfMoveClock > 100)
		{
			LOG_THROW_ERROR("Invalid half move clock: cannot exceed 100 in tournament mode (50-move rule)", true);
		}
		this->halfMoveClock = halfMoveClock;
		return;
	}

	void Fen::setFullMoves(const uint32_t &fullMoves)
	{
		if (fullMoves == 0)
		{
			LOG_THROW_ERROR("Full moves must be at least 1: " + std::to_string(fullMoves), true);
		}
		this->fullMoves = fullMoves;
		return;
	}

	std::string Fen::generateFen(void) const
	{
		std::ostringstream oss;
		oss << this->placement << " "
			<< (this->color == WHITE ? "w" : "b") << " "
			<< this->castling << " "
			<< (this->enPassant.empty() ? "-" : this->enPassant) << " "
			<< this->halfMoveClock << " "
			<< this->fullMoves;
		return oss.str();
	}

}
