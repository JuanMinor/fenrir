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
	/* Precomputed attack tables — initialized once at startup via init_attack_tables() */

	/* Knight attacks: KNIGHT_ATTACKS[sq] = bitboard of squares a knight on sq can attack */
	extern uint64_t KNIGHT_ATTACKS[64];

	/* King attacks: KING_ATTACKS[sq] = bitboard of squares a king on sq can attack */
	extern uint64_t KING_ATTACKS[64];

	/* Pawn attacks: PAWN_ATTACKS[color][sq] — indexed by [WHITE/BLACK][square] */
	extern uint64_t PAWN_ATTACKS[2][64];

	/* Ray tables: RAY[sq][dir] — 8 directional rays (0=N,1=NE,2=E,3=SE,4=S,5=SW,6=W,7=NW) */
	extern uint64_t RAY[64][8];

	/* Direction indices for RAY table */
	constexpr int RAY_N  = 0;
	constexpr int RAY_NE = 1;
	constexpr int RAY_E  = 2;
	constexpr int RAY_SE = 3;
	constexpr int RAY_S  = 4;
	constexpr int RAY_SW = 5;
	constexpr int RAY_W  = 6;
	constexpr int RAY_NW = 7;

	/* Initialize all attack tables. Must be called once before any move generation. */
	void init_attack_tables();

} // namespace fenrir
