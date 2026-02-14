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

#include "include/engine/engine.h"

namespace fenrir
{

	Engine::Engine(const std::string &fenString) : fen(fenString), board(fenString)
	{
		logger::INFO("Engine initialized with FEN: " + fenString);
	}

	Engine::~Engine() {}

	const IBoardView &Engine::getBoardView() const
	{
		return board;
	}

	const std::vector<Move> Engine::generateMoves(const std::string &algebraicAddress) const
	{
		std::vector<Move> moves;
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

		Moves::getInstance().generateMoves(piece, board, moves);

		logger::DEBUG("Generated moves for piece at address: " + algebraicAddress);

		return moves;
	}

	std::string Engine::getFen(void)
	{
		std::string current_fen = board.getFen();
		logger::DEBUG("Current FEN: " + current_fen);
		return current_fen;
	}

	void Engine::makeMove(const Move &move)
	{
		u_int8_t fromRank, fromFile, toRank, toFile;
		utils::parseAlgebraicNotation(move.getFrom(), fromRank, fromFile);
		utils::parseAlgebraicNotation(move.getTo(), toRank, toFile);
		Piece *piece = board.getBoard().at(fromRank).at(fromFile);
		if (!piece)
		{
			logger::ERROR("No piece found at " + move.getFrom());
			return;
		}
		board.move(piece, toRank, toFile);

		logger::DEBUG("Made move from " + move.getFrom() + " to " + move.getTo());
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
