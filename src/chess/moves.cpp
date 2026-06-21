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

#include "include/chess/moves.h"
#include <locale>
#include <cctype>

namespace fenrir
{
	Moves::Moves()
	{
		logger::INFO("Moves instance created");
	}

	Moves::~Moves() {}

	void Moves::generateBishopMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		constexpr int8_t directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
		this->slideInDirections(rank, file, piece_char, board, moves, directions, 4);
	}

	void Moves::addCaptureMove(uint8_t fromRank, uint8_t fromFile, char piece_char, uint8_t toRank, uint8_t toFile, char target_char, std::vector<Move> &moves) const
	{
		bool is_white = std::isupper(static_cast<unsigned char>(piece_char));
		bool target_white = std::isupper(static_cast<unsigned char>(target_char));
		if (is_white != target_white)
		{
			uint8_t from_sq = static_cast<uint8_t>(fromRank * 8 + fromFile);
			uint8_t to_sq = static_cast<uint8_t>(toRank * 8 + toFile);
			Move move = Move(from_sq, to_sq, MoveType::CAPTURE);
			moves.emplace_back(move);
			logger::DEBUG("Capture move generated from " + utils::getAlgebraicNotation(fromRank, fromFile) + " to " + utils::getAlgebraicNotation(toRank, toFile));
		}
	}

	void Moves::generateKingMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		constexpr int8_t directionVectors[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
		this->slideInDirections(rank, file, piece_char, board, moves, directionVectors, 8, true);
	}

	void Moves::generateKnightMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		const uint8_t from_sq = static_cast<uint8_t>(rank * 8 + file);

		constexpr int8_t knightMoves[8][2] = {
			{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

		for (const auto &knightMove : knightMoves)
		{
			const int8_t newRank = static_cast<int8_t>(static_cast<int>(rank) + knightMove[0]);
			const int8_t newFile = static_cast<int8_t>(static_cast<int>(file) + knightMove[1]);

			if (newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
			{
				char targetPiece = board.getPiece(static_cast<uint8_t>(newRank), static_cast<uint8_t>(newFile));
				if (targetPiece == '\0')
				{
					uint8_t to_sq = static_cast<uint8_t>(newRank * 8 + newFile);
					moves.emplace_back(Move(from_sq, to_sq, MoveType::NORMAL));
					continue;
				}
				this->addCaptureMove(rank, file, piece_char, static_cast<uint8_t>(newRank), static_cast<uint8_t>(newFile), targetPiece, moves);
			}
		}

		logGeneratedMoves(piece_char, rank, file, moves);
	}

	void Moves::logGeneratedMoves(char piece_char, uint8_t rank, uint8_t file, const std::vector<Move> &moves) const
	{
		std::stringstream ss;
		bool is_white = std::isupper(static_cast<unsigned char>(piece_char));
		char lower_alias = static_cast<char>(std::tolower(static_cast<unsigned char>(piece_char)));
		ss << "Generated moves for "
		   << (is_white ? "white" : "black")
		   << " "
		   << PIECE_NAMES.at(lower_alias)
		   << " at " << utils::getAlgebraicNotation(rank, file)
		   << ": ";

		if (moves.empty())
		{
			ss << "No moves were generated 😢";
		}

		bool first = true;
		for (const Move &move : moves)
		{
			if (!first)
			{
				ss << ", ";
			}
			ss << move.getFrom() << "->" << move.getTo();
			first = false;
		}
		logger::DEBUG(ss.str());
	}

	void Moves::generatePawnMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		bool is_white = std::isupper(static_cast<unsigned char>(piece_char));
		int direction = is_white ? 1 : -1;
		uint8_t from_sq = static_cast<uint8_t>(rank * 8 + file);

		int newRank = static_cast<int>(rank) + direction;
		if (newRank >= 0 && newRank < BOARD_SIZE && board.getPiece(static_cast<uint8_t>(newRank), file) == '\0')
		{
			uint8_t to_sq = static_cast<uint8_t>(newRank * 8 + file);
			Move move = Move(from_sq, to_sq, MoveType::NORMAL);
			moves.emplace_back(move);
		}

		if (file > 0)
		{
			char target = board.getPiece(static_cast<uint8_t>(newRank), static_cast<uint8_t>(file - 1));
			if (target != '\0')
			{
				this->addCaptureMove(rank, file, piece_char, static_cast<uint8_t>(newRank), static_cast<uint8_t>(file - 1), target, moves);
			}
		}

		if (file < BOARD_SIZE - 1)
		{
			char target = board.getPiece(static_cast<uint8_t>(newRank), static_cast<uint8_t>(file + 1));
			if (target != '\0')
			{
				this->addCaptureMove(rank, file, piece_char, static_cast<uint8_t>(newRank), static_cast<uint8_t>(file + 1), target, moves);
			}
		}

		if ((is_white && rank == 1) || (!is_white && rank == 6))
		{
			if (board.getPiece(static_cast<uint8_t>(rank + direction), file) == '\0')
			{
				newRank = static_cast<int>(rank) + (2 * direction);
				if (newRank >= 0 && newRank < BOARD_SIZE && board.getPiece(static_cast<uint8_t>(newRank), file) == '\0')
				{
					uint8_t to_sq = static_cast<uint8_t>(newRank * 8 + file);
					Move move = Move(from_sq, to_sq, MoveType::NORMAL);
					moves.emplace_back(move);
				}
			}
		}

		const std::string &enPassant = board.getEnPassant();
		if (!enPassant.empty())
		{
			uint8_t enPassantRank, enPassantFile;
			utils::parseAlgebraicNotation(enPassant.c_str(), enPassantRank, enPassantFile);
			if (enPassantRank == static_cast<uint8_t>(static_cast<int>(rank) + direction) && std::abs(static_cast<int>(enPassantFile) - static_cast<int>(file)) == 1)
			{
				char targetPiece = board.getPiece(rank, enPassantFile);
				if (targetPiece != '\0' && std::tolower(static_cast<unsigned char>(targetPiece)) == 'p')
				{
					bool target_white = std::isupper(static_cast<unsigned char>(targetPiece));
					if (is_white != target_white)
					{
						uint8_t to_sq = static_cast<uint8_t>(enPassantRank * 8 + enPassantFile);
						Move move = Move(from_sq, to_sq, MoveType::EN_PASSANT);
						moves.emplace_back(move);
					}
				}
			}
		}

		logGeneratedMoves(piece_char, rank, file, moves);
	}

	void Moves::generateQueenMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		constexpr int8_t directionVectors[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
		this->slideInDirections(rank, file, piece_char, board, moves, directionVectors, 8);
	}

	void Moves::generateRookMoves(uint8_t rank, uint8_t file, char piece_char, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		constexpr int8_t directionVectors[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
		this->slideInDirections(rank, file, piece_char, board, moves, directionVectors, 4);
	}

	void Moves::slideInDirections(uint8_t fromRank, uint8_t fromFile, char piece_char, const AbstractBoard &board, std::vector<Move> &moves, const int8_t directionVectors[][2], size_t numDirections, bool singleDepth) const
	{
		const uint8_t from_sq = static_cast<uint8_t>(fromRank * 8 + fromFile);

		for (size_t i = 0; i < numDirections; ++i)
		{
			const int8_t rankDelta = directionVectors[i][0];
			const int8_t fileDelta = directionVectors[i][1];

			int8_t newRank = static_cast<int8_t>(static_cast<int>(fromRank) + rankDelta);
			int8_t newFile = static_cast<int8_t>(static_cast<int>(fromFile) + fileDelta);
			uint8_t depth = 0;

			while (newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
			{
				if (singleDepth && depth == 1)
				{
					break;
				}
				depth++;
				char targetPiece = board.getPiece(static_cast<uint8_t>(newRank), static_cast<uint8_t>(newFile));
				if (targetPiece == '\0')
				{
					uint8_t to_sq = static_cast<uint8_t>(newRank * 8 + newFile);
					moves.emplace_back(Move(from_sq, to_sq, MoveType::NORMAL));
					newRank = static_cast<int8_t>(static_cast<int>(newRank) + rankDelta);
					newFile = static_cast<int8_t>(static_cast<int>(newFile) + fileDelta);
					continue;
				}
				this->addCaptureMove(fromRank, fromFile, piece_char, static_cast<uint8_t>(newRank), static_cast<uint8_t>(newFile), targetPiece, moves);
				break;
			}
		}

		logGeneratedMoves(piece_char, fromRank, fromFile, moves);
	}

	Moves &Moves::getInstance()
	{
		static Moves instance;
		logger::DEBUG("Moves instance created and returned. Only one instance will be used throughout the application.");
		return instance;
	}

	void Moves::generateMoves(uint8_t rank, uint8_t file, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		char piece_char = board.getPiece(rank, file);
		if (piece_char == '\0')
		{
			logger::ERROR("Piece is null/empty. Moves cannot be generated 😢");
			return;
		}
		logger::DEBUG("Generating moves for piece at " + utils::getAlgebraicNotation(rank, file));
		switch (std::tolower(static_cast<unsigned char>(piece_char)))
		{
		case 'b':
			this->generateBishopMoves(rank, file, piece_char, board, moves);
			break;
		case 'k':
			this->generateKingMoves(rank, file, piece_char, board, moves);
			break;
		case 'n':
			this->generateKnightMoves(rank, file, piece_char, board, moves);
			break;
		case 'p':
			this->generatePawnMoves(rank, file, piece_char, board, moves);
			break;
		case 'q':
			this->generateQueenMoves(rank, file, piece_char, board, moves);
			break;
		case 'r':
			this->generateRookMoves(rank, file, piece_char, board, moves);
			break;
		}
	}

}
