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

	void Moves::generateBishopMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves)
	{
		int8_t directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
		this->slideInDirections(piece, board, moves, directions, 4);
	}

	void Moves::addCaptureMove(const Piece *piece, const Piece *targetPiece, std::vector<std::pair<const std::string, const std::string>> &moves)
	{
		if (piece == nullptr || targetPiece == nullptr)
		{
			logger::WARN(!piece ? "Piece is null 😢 and capture is not possible" : "Target piece is null 😢 and capture is not possible");
			return;
		}
		if (piece->getColor() != targetPiece->getColor())
		{
			moves.emplace_back(std::make_pair(utils::getAlgebraicNotation(piece->getRank(), piece->getFile()),
											  utils::getAlgebraicNotation(targetPiece->getRank(), targetPiece->getFile())));
			logger::DEBUG("Capture move generated from " + utils::getAlgebraicNotation(piece->getRank(), piece->getFile()) +
						  " to " + utils::getAlgebraicNotation(targetPiece->getRank(), targetPiece->getFile()));
		}
	}

	void Moves::generateKingMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves)
	{
		constexpr int8_t direction_vectors[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
		this->slideInDirections(piece, board, moves, direction_vectors, 8, true);
	}

	void Moves::generateKnightMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves)
	{
		const uint8_t current_rank = piece->getRank();
		const uint8_t current_file = piece->getFile();
		const std::string from_position = utils::getAlgebraicNotation(current_rank, current_file);

		constexpr int8_t knight_moves[8][2] = {
			{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

		for (const auto &move : knight_moves)
		{
			const int8_t new_rank = current_rank + move[0];
			const int8_t new_file = current_file + move[1];

			if (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
			{
				const Piece *target_piece = board->getPiece(new_rank, new_file);
				if (!target_piece)
				{
					moves.emplace_back(from_position, utils::getAlgebraicNotation(new_rank, new_file));
					continue;
				}
				this->addCaptureMove(piece, target_piece, moves);
			}
		}

		logGeneratedMoves(piece, moves);
	}

	void Moves::logGeneratedMoves(const Piece *piece, const std::vector<std::pair<const std::string, const std::string>> &moves) const
	{
		std::stringstream ss;
		ss << "Generated moves for "
		   << (piece->getColor() == WHITE ? "white" : "black")
		   << " "
		   << PIECE_NAMES.at(std::tolower(piece->getAlias(), std::locale()))
		   << " at " << utils::getAlgebraicNotation(piece->getRank(), piece->getFile())
		   << ": ";

		if (moves.empty())
		{
			ss << "No moves were generated 😢";
		}

		for (size_t i = 0; i < moves.size(); ++i)
		{
			ss << moves.at(i).first << "->" << moves.at(i).second;
			if (i != moves.size() - 1)
			{
				ss << ", ";
			}
		}
		logger::DEBUG(ss.str());
	}

	void Moves::generatePawnMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves)
	{
		uint8_t rank = piece->getRank();
		uint8_t file = piece->getFile();
		uint8_t color = piece->getColor();
		uint8_t direction = (color == WHITE) ? 1 : -1;

		uint8_t newRank = rank + direction;
		if (newRank >= 0 && newRank < BOARD_SIZE && !board->getPiece(newRank, file))
		{
			moves.emplace_back(std::make_pair(utils::getAlgebraicNotation(rank, file),
											  utils::getAlgebraicNotation(newRank, file)));
		}

		// Left diagonal capture
		if (file > 0)
		{
			this->addCaptureMove(piece, board->getPiece(newRank, file - 1), moves);
		}
		// Right diagonal capture
		if (file < BOARD_SIZE - 1)
		{
			this->addCaptureMove(piece, board->getPiece(newRank, file + 1), moves);
		}

		if (!piece->getMoved())
		{
			newRank = rank + (2 * direction);
			if (newRank >= 0 && newRank < BOARD_SIZE && !board->getPiece(newRank, file))
			{
				moves.emplace_back(std::make_pair(utils::getAlgebraicNotation(rank, file),
												  utils::getAlgebraicNotation(newRank, file)));
			}
		}

		// En passant
		const std::string &en_passant = board->getEnPassant();
		if (!en_passant.empty())
		{
			uint8_t en_passant_rank, en_passant_file;
			utils::parseAlgebraicNotation(en_passant.c_str(), en_passant_rank, en_passant_file);
			if (en_passant_rank == newRank && std::abs(int(en_passant_file) - int(file)) == 1)
			{
				const Piece *target_piece = board->getPiece(rank, en_passant_file);
				if (target_piece && std::tolower(target_piece->getAlias()) == 'p')
				{
					moves.emplace_back(
						std::make_pair(
							utils::getAlgebraicNotation(rank, file),
							utils::getAlgebraicNotation(en_passant_rank, en_passant_file)));
				}
			}
		}

		logGeneratedMoves(piece, moves);
	}

	void Moves::generateQueenMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves)
	{
		constexpr int8_t direction_vectors[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
		this->slideInDirections(piece, board, moves, direction_vectors, 8);
	}

	void Moves::generateRookMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves)
	{
		constexpr int8_t direction_vectors[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
		this->slideInDirections(piece, board, moves, direction_vectors, 4);
	}

	void Moves::slideInDirections(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves, const int8_t directionVectors[][2], size_t numDirections, bool singleDepth)
	{
		const uint8_t current_rank = piece->getRank();
		const uint8_t current_file = piece->getFile();
		const std::string from_position = utils::getAlgebraicNotation(current_rank, current_file);

		for (size_t i = 0; i < numDirections; ++i)
		{
			const int8_t rank_delta = directionVectors[i][0];
			const int8_t file_delta = directionVectors[i][1];

			int8_t newRank = current_rank + rank_delta;
			int8_t newFile = current_file + file_delta;
			uint8_t depth = 0;

			while (newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
			{
				if (singleDepth && depth == 1)
				{
					break;
				}
				depth++;
				const Piece *target_piece = board->getPiece(newRank, newFile);
				if (!target_piece)
				{
					moves.emplace_back(from_position, utils::getAlgebraicNotation(newRank, newFile));
					newRank += rank_delta;
					newFile += file_delta;
					continue;
				}
				this->addCaptureMove(piece, target_piece, moves);
				break;
			}
		}

		logGeneratedMoves(piece, moves);
	}

	// Public
	Moves &Moves::getInstance()
	{
		static Moves instance;
		logger::DEBUG("Moves instance created and returned. Only one instance will be used throughout the application.");
		return instance;
	}

	void Moves::generateMoves(const Piece *piece, const Board *board, std::vector<std::pair<const std::string, const std::string>> &moves)
	{
		if (piece == nullptr)
		{
			logger::ERROR("Piece is null. Moves cannot be generated 😢");
			return;
		}
		logger::DEBUG("Generating moves for piece at " + utils::getAlgebraicNotation(piece->getRank(), piece->getFile()));
		switch (std::tolower(piece->getAlias()))
		{
		case 'b':
			this->generateBishopMoves(piece, board, moves);
			break;
		case 'k':
			this->generateKingMoves(piece, board, moves);
			break;
		case 'n':
			this->generateKnightMoves(piece, board, moves);
			break;
		case 'p':
			this->generatePawnMoves(piece, board, moves);
			break;
		case 'q':
			this->generateQueenMoves(piece, board, moves);
			break;
		case 'r':
			this->generateRookMoves(piece, board, moves);
			break;
		}
	}
}
