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

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <format>
#include "include/core/core.h"
#include "include/chess/fen.h"
#include "include/logger/logger.h"
#include "include/modifier/modifier.h"
#include "include/pgn/pgn.h"
#include "include/utils/utils.h"
#include "include/abstract/board.h"

namespace fenrir
{
	/**
	 * Board manages the 12 chess piece bitboards and board state metadata.
	 * It contains zero heap-allocated objects and operates purely using
	 * bitwise operations for high search performance.
	 */
	class Board : public AbstractBoard
	{
	private:
		uint64_t bitboards[12]; // 0-5: White (P, N, B, R, Q, K), 6-11: Black (p, n, b, r, q, k)
		uint64_t white_occupancy;
		uint64_t black_occupancy;
		uint64_t combined_occupancy;

		std::string castling;
		std::string enPassant;
		uint8_t color;
		uint8_t halfMoveClock;
		uint8_t fullMoves;
		Fen fen;

		inline int getBitboardIndex(char alias) const
		{
			switch (alias)
			{
				case 'P': return 0;
				case 'N': return 1;
				case 'B': return 2;
				case 'R': return 3;
				case 'Q': return 4;
				case 'K': return 5;
				case 'p': return 6;
				case 'n': return 7;
				case 'b': return 8;
				case 'r': return 9;
				case 'q': return 10;
				case 'k': return 11;
				default: return -1;
			}
		}

		inline void set_bit(uint64_t &bb, uint8_t square)
		{
			bb |= (1ULL << square);
		}

		inline void clear_bit(uint64_t &bb, uint8_t square)
		{
			bb &= ~(1ULL << square);
		}

		inline bool test_bit(uint64_t bb, uint8_t square) const
		{
			return (bb & (1ULL << square)) != 0;
		}

		void buildBoard(const std::string &placement);
		std::string generatePlacementFromBoard(void) const;
		void logPieceAction(const std::string &action, char piece_char, uint8_t rank, uint8_t file, const std::string &emoji) const;

	public:
		explicit Board(const std::string &fenString);
		~Board() = default;

		Board(const Board &) = delete;
		Board &operator=(const Board &) = delete;
		Board(Board &&) = delete;
		Board &operator=(Board &&) = delete;

		std::string getFen(void);
		const std::string &getEnPassant(void) const override;
		char getPiece(uint8_t rank, uint8_t file) const override;
		uint64_t getBitboard(int index) const { return bitboards[index]; }
		uint64_t getCombinedOccupancy() const { return combined_occupancy; }
		uint64_t getWhiteOccupancy() const { return white_occupancy; }
		uint64_t getBlackOccupancy() const { return black_occupancy; }
		uint8_t getColor() const { return color; }

		void move(uint8_t fromRank, uint8_t fromFile, uint8_t toRank, uint8_t toFile);
		void print(void) const;

		void reset(const std::string &fenString);
	};
}
