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

#include "include/engine/engine.h"

namespace fenrir
{

	Engine::Engine(const std::string &fenString) : fen(fenString), board(fenString)
	{
		logger::INFO("Engine initialized with FEN: " + fenString);
	}

	Engine::~Engine() {}

#ifndef NDEBUG
	char Engine::getPiece(const std::string &algebraicAddress) const
	{
		uint8_t rank, file;
		utils::parseAlgebraicNotation(algebraicAddress, rank, file);

		const Piece *piece = board.getPiece(rank, file);
		if (piece == nullptr)
		{
			return '.'; // Empty square
		}
		return piece->getAlias();
	}
#endif

	std::vector<std::pair<const std::string, const std::string>> Engine::generateMoves(const std::string &algebraicAddress) const
	{
		std::vector<std::pair<const std::string, const std::string>> moves;
		u_int8_t rank, file;
		utils::parseAlgebraicNotation(algebraicAddress, rank, file);
		if (!board.getBoard().at(rank).at(file))
		{
			LOG_THROW_ERROR(
				(std::string("Board address ") + algebraicAddress + " does not contain a piece").c_str(),
				false);
			return moves;
		}
		const Piece *piece = board.getPiece(rank, file);

		Moves::getInstance().generateMoves(piece, &board, moves);

		logger::DEBUG("Generated moves for piece at address: " + algebraicAddress);

		return moves;
	}

	std::string Engine::getFen(void)
	{
		std::string current_fen = board.getFen();
		logger::DEBUG("Current FEN: " + current_fen);
		return current_fen;
	}

	void Engine::makeMove(const std::string &fromAlgebraicAddress, const std::string &toAlgebraicAddress)
	{
		u_int8_t fromRank, fromFile, toRank, toFile;
		utils::parseAlgebraicNotation(fromAlgebraicAddress, fromRank, fromFile);
		utils::parseAlgebraicNotation(toAlgebraicAddress, toRank, toFile);

		Piece *piece = board.getBoard().at(fromRank).at(fromFile);
		if (!piece)
		{
			logger::ERROR("No piece found at " + fromAlgebraicAddress);
			return;
		}
		board.move(piece, toRank, toFile);

		logger::DEBUG("Made move from " + fromAlgebraicAddress + " to " + toAlgebraicAddress);
	}

	void Engine::printBoard(void) const
	{
		board.print();
	}

	void Engine::reset()
	{
		board.~Board();
		new (&board) Board(fen);
		logger::INFO("Reset the board to the initial state with FEN: " + fen);
	}
}
