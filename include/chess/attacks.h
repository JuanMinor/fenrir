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
    extern uint64_t KNIGHT_ATTACKS[64];
    extern uint64_t KING_ATTACKS[64];
    extern uint64_t PAWN_ATTACKS[2][64];
    extern uint64_t RAY[64][8];

    constexpr int RAY_N = 0;
    constexpr int RAY_NE = 1;
    constexpr int RAY_E = 2;
    constexpr int RAY_SE = 3;
    constexpr int RAY_S = 4;
    constexpr int RAY_SW = 5;
    constexpr int RAY_W = 6;
    constexpr int RAY_NW = 7;

    /**
     * @brief Initialize precomputed attack tables for all piece types.
     */
    void init_attack_tables();

} /* namespace fenrir */
