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

#pragma once

#include <cstdint>

namespace fenrir
{
	/* Precomputed attack tables (initialized via init_attack_tables()) */

	/* KNIGHT_ATTACKS[sq]: Knight attack bitboard. */
	extern uint64_t KNIGHT_ATTACKS[64];

	/* KING_ATTACKS[sq]: King attack bitboard. */
	extern uint64_t KING_ATTACKS[64];

	/* PAWN_ATTACKS[color][sq]: Pawn attack bitboard indexed by [color][square]. */
	extern uint64_t PAWN_ATTACKS[2][64];

	/* RAY[sq][dir]: Ray bitboards in 8 directions (0=N, 1=NE, 2=E, 3=SE, 4=S, 5=SW, 6=W, 7=NW). */
	extern uint64_t RAY[64][8];

	/* Direction indices for RAY. */
	constexpr int RAY_N  = 0;
	constexpr int RAY_NE = 1;
	constexpr int RAY_E  = 2;
	constexpr int RAY_SE = 3;
	constexpr int RAY_S  = 4;
	constexpr int RAY_SW = 5;
	constexpr int RAY_W  = 6;
	constexpr int RAY_NW = 7;

	/* Initializes attack tables (must be called before move generation). */
	void init_attack_tables();

} /* namespace fenrir */
