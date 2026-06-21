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

#pragma once

#include "include/chess/board.h"
#include "include/core/core.h"
#include "include/logger/logger.h"
#include "include/chess/moves.h"
#include "include/utils/utils.h"
#include "include/abstract/board.h"

namespace fenrir
{

	class FENRIR_API Engine final
	{
		static constexpr const char *defaultFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		std::string fen;
		Board board;

	public:
		Engine(const std::string &fenString = defaultFen);
		~Engine();

		std::vector<Move> generateMoves(const std::string &algebraicAddress) const;
		std::string getFen(void);
		void makeMove(const Move &move);

		void printBoard(void) const;
		void reset();

		const AbstractBoard &getBoardView() const;
	};
}
