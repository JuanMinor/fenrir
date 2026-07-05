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

#include "include/chess/attacks.h"

namespace fenrir
{
	/* White pawn attacks = 0, black pawn attacks = 1 */
	static constexpr int ATTACK_WHITE = 0;
	static constexpr int ATTACK_BLACK = 1;

uint64_t KNIGHT_ATTACKS[64];
uint64_t KING_ATTACKS[64];
uint64_t PAWN_ATTACKS[2][64];
uint64_t RAY[64][8];

void init_attack_tables()
{
	/* Knight move deltas */
	constexpr int knight_rank_deltas[8] = {2,  2, -2, -2,  1,  1, -1, -1};
	constexpr int knight_file_deltas[8] = {1, -1,  1, -1,  2, -2,  2, -2};

	/* King move deltas */
	constexpr int king_rank_deltas[8] = {1, 1, 0, -1, -1, -1,  0,  1};
	constexpr int king_file_deltas[8] = {0, 1, 1,  1,  0, -1, -1, -1};

	/* Ray direction deltas: N, NE, E, SE, S, SW, W, NW */
	constexpr int ray_rank_deltas[8] = {1,  1,  0, -1, -1, -1,  0,  1};
	constexpr int ray_file_deltas[8] = {0,  1,  1,  1,  0, -1, -1, -1};

	for (int sq = 0; sq < 64; ++sq)
	{
		int rank = sq / 8;
		int file = sq % 8;

		/* Precompute knight attacks */
		uint64_t knight_bb = 0ULL;
		for (int i = 0; i < 8; ++i)
		{
			int nr = rank + knight_rank_deltas[i];
			int nf = file + knight_file_deltas[i];
			if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8)
			{
				knight_bb |= (1ULL << (nr * 8 + nf));
			}
		}
		KNIGHT_ATTACKS[sq] = knight_bb;

		/* Precompute king attacks */
		uint64_t king_bb = 0ULL;
		for (int i = 0; i < 8; ++i)
		{
			int nr = rank + king_rank_deltas[i];
			int nf = file + king_file_deltas[i];
			if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8)
			{
				king_bb |= (1ULL << (nr * 8 + nf));
			}
		}
		KING_ATTACKS[sq] = king_bb;

		/* White pawn attacks (rank + 1) */
		uint64_t white_pawn_bb = 0ULL;
		if (rank + 1 < 8)
		{
			if (file - 1 >= 0)  white_pawn_bb |= (1ULL << ((rank + 1) * 8 + (file - 1)));
			if (file + 1 < 8)   white_pawn_bb |= (1ULL << ((rank + 1) * 8 + (file + 1)));
		}
		PAWN_ATTACKS[ATTACK_WHITE][sq] = white_pawn_bb;

		/* Black pawn attacks (rank - 1) */
		uint64_t black_pawn_bb = 0ULL;
		if (rank - 1 >= 0)
		{
			if (file - 1 >= 0)  black_pawn_bb |= (1ULL << ((rank - 1) * 8 + (file - 1)));
			if (file + 1 < 8)   black_pawn_bb |= (1ULL << ((rank - 1) * 8 + (file + 1)));
		}
		PAWN_ATTACKS[ATTACK_BLACK][sq] = black_pawn_bb;

		/* Precompute rays */
		for (int dir = 0; dir < 8; ++dir)
		{
			uint64_t ray_bb = 0ULL;
			int nr = rank + ray_rank_deltas[dir];
			int nf = file + ray_file_deltas[dir];
			while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8)
			{
				ray_bb |= (1ULL << (nr * 8 + nf));
				nr += ray_rank_deltas[dir];
				nf += ray_file_deltas[dir];
			}
			RAY[sq][dir] = ray_bb;
		}
	}
}

} /* namespace fenrir */
