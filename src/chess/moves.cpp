/*
 *   Copyright (c) 2026 Juan Minor

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

	void Moves::generateBishopMoves(const Piece *piece, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		constexpr int8_t directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
		this->slideInDirections(piece, board, moves, directions, 4);
	}

	void Moves::addCaptureMove(const Piece *piece, const Piece *targetPiece, std::vector<Move> &moves) const
	{
		if (piece == nullptr || targetPiece == nullptr)
		{
			logger::WARN(!piece ? "Piece is null 😢 and capture is not possible" : "Target piece is null 😢 and capture is not possible");
			return;
		}
		if (piece->getColor() != targetPiece->getColor())
		{
			uint8_t from_sq = static_cast<uint8_t>(piece->getRank() * 8 + piece->getFile());
			uint8_t to_sq = static_cast<uint8_t>(targetPiece->getRank() * 8 + targetPiece->getFile());
			Move move = Move(from_sq, to_sq, MoveType::CAPTURE);
			moves.emplace_back(move);
			logger::DEBUG("Capture move generated from " + utils::getAlgebraicNotation(piece->getRank(), piece->getFile()) + " to " + utils::getAlgebraicNotation(targetPiece->getRank(), targetPiece->getFile()));
		}
	}

	void Moves::generateKingMoves(const Piece *piece, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		constexpr int8_t directionVectors[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
		this->slideInDirections(piece, board, moves, directionVectors, 8, true);
	}

	void Moves::generateKnightMoves(const Piece *piece, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		const uint8_t currentRank = piece->getRank();
		const uint8_t currentFile = piece->getFile();
		const uint8_t from_sq = static_cast<uint8_t>(currentRank * 8 + currentFile);

		constexpr int8_t knightMoves[8][2] = {
			{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

		for (const auto &knightMove : knightMoves)
		{
			const int8_t newRank = static_cast<int8_t>(static_cast<int>(currentRank) + knightMove[0]);
			const int8_t newFile = static_cast<int8_t>(static_cast<int>(currentFile) + knightMove[1]);

			if (newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
			{
				const Piece *targetPiece = board.getPiece(static_cast<uint8_t>(newRank), static_cast<uint8_t>(newFile));
				if (!targetPiece)
				{
					uint8_t to_sq = static_cast<uint8_t>(newRank * 8 + newFile);
					moves.emplace_back(Move(from_sq, to_sq, MoveType::NORMAL));
					continue;
				}
				this->addCaptureMove(piece, targetPiece, moves);
			}
		}

		logGeneratedMoves(piece, moves);
	}

	void Moves::logGeneratedMoves(const Piece *piece, const std::vector<Move> &moves) const
	{
		std::stringstream ss;
		ss << "Generated moves for "
		   << (piece->getColor() == WHITE ? "white" : "black")
		   << " "
		   << PIECE_NAMES.at(static_cast<char>(std::tolower(piece->getAlias(), std::locale())))
		   << " at " << utils::getAlgebraicNotation(piece->getRank(), piece->getFile())
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

	void Moves::generatePawnMoves(const Piece *piece, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		uint8_t rank = piece->getRank();
		uint8_t file = piece->getFile();
		uint8_t color = piece->getColor();
		int direction = (color == WHITE) ? 1 : -1;
		uint8_t from_sq = static_cast<uint8_t>(rank * 8 + file);

		int newRank = static_cast<int>(rank) + direction;
		if (newRank >= 0 && newRank < BOARD_SIZE && !board.getPiece(static_cast<uint8_t>(newRank), file))
		{
			uint8_t to_sq = static_cast<uint8_t>(newRank * 8 + file);
			Move move = Move(from_sq, to_sq, MoveType::NORMAL);
			moves.emplace_back(move);
		}

		if (file > 0)
		{
			this->addCaptureMove(piece, board.getPiece(static_cast<uint8_t>(newRank), static_cast<uint8_t>(file - 1)), moves);
		}

		if (file < BOARD_SIZE - 1)
		{
			this->addCaptureMove(piece, board.getPiece(static_cast<uint8_t>(newRank), static_cast<uint8_t>(file + 1)), moves);
		}

		if (!piece->getMoved())
		{
			newRank = static_cast<int>(rank) + (2 * direction);
			if (newRank >= 0 && newRank < BOARD_SIZE && !board.getPiece(static_cast<uint8_t>(newRank), file))
			{
				uint8_t to_sq = static_cast<uint8_t>(newRank * 8 + file);
				Move move = Move(from_sq, to_sq, MoveType::NORMAL);
				moves.emplace_back(move);
			}
		}

		const std::string &enPassant = board.getEnPassant();
		if (!enPassant.empty())
		{
			uint8_t enPassantRank, enPassantFile;
			utils::parseAlgebraicNotation(enPassant.c_str(), enPassantRank, enPassantFile);
			if (enPassantRank == static_cast<uint8_t>(static_cast<int>(rank) + direction) && std::abs(static_cast<int>(enPassantFile) - static_cast<int>(file)) == 1)
			{
				const Piece *targetPiece = board.getPiece(rank, enPassantFile);
				if (targetPiece && std::tolower(targetPiece->getAlias()) == 'p')
				{
					uint8_t to_sq = static_cast<uint8_t>(enPassantRank * 8 + enPassantFile);
					Move move = Move(from_sq, to_sq, MoveType::EN_PASSANT);
					moves.emplace_back(move);
				}
			}
		}

		logGeneratedMoves(piece, moves);
	}

	void Moves::generateQueenMoves(const Piece *piece, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		constexpr int8_t directionVectors[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
		this->slideInDirections(piece, board, moves, directionVectors, 8);
	}

	void Moves::generateRookMoves(const Piece *piece, const AbstractBoard &board, std::vector<Move> &moves) const
	{
		constexpr int8_t directionVectors[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
		this->slideInDirections(piece, board, moves, directionVectors, 4);
	}

	void Moves::slideInDirections(const Piece *piece, const AbstractBoard &board, std::vector<Move> &moves, const int8_t directionVectors[][2], size_t numDirections, bool singleDepth) const
	{
		const uint8_t currentRank = piece->getRank();
		const uint8_t currentFile = piece->getFile();
		const uint8_t from_sq = static_cast<uint8_t>(currentRank * 8 + currentFile);

		for (size_t i = 0; i < numDirections; ++i)
		{
			const int8_t rankDelta = directionVectors[i][0];
			const int8_t fileDelta = directionVectors[i][1];

			int8_t newRank = static_cast<int8_t>(static_cast<int>(currentRank) + rankDelta);
			int8_t newFile = static_cast<int8_t>(static_cast<int>(currentFile) + fileDelta);
			uint8_t depth = 0;

			while (newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
			{
				if (singleDepth && depth == 1)
				{
					break;
				}
				depth++;
				const Piece *targetPiece = board.getPiece(static_cast<uint8_t>(newRank), static_cast<uint8_t>(newFile));
				if (!targetPiece)
				{
					uint8_t to_sq = static_cast<uint8_t>(newRank * 8 + newFile);
					moves.emplace_back(Move(from_sq, to_sq, MoveType::NORMAL));
					newRank = static_cast<int8_t>(static_cast<int>(newRank) + rankDelta);
					newFile = static_cast<int8_t>(static_cast<int>(newFile) + fileDelta);
					continue;
				}
				this->addCaptureMove(piece, targetPiece, moves);
				break;
			}
		}

		logGeneratedMoves(piece, moves);
	}

	Moves &Moves::getInstance()
	{
		static Moves instance;
		logger::DEBUG("Moves instance created and returned. Only one instance will be used throughout the application.");
		return instance;
	}

	void Moves::generateMoves(const Piece *piece, const AbstractBoard &board, std::vector<Move> &moves) const
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
