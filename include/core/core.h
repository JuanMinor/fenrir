/*
 *   Copyright (c) 2025 Juan Minor
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

#include <unordered_map>
#include <cstdint>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifdef FENRIR_BUILD_DLL
#define FENRIR_API __declspec(dllexport)
#else
#define FENRIR_API
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#define FENRIR_API __attribute__((visibility("default")))
#else
#define FENRIR_API
#endif

namespace fenrir
{
    /**
     * @brief Performs a forward bitscan to find the index of the least significant set bit (LSB).
     * @param bb The 64-bit bitboard to scan.
     * @returns The 0-indexed position of the least significant set bit.
     */
    inline uint8_t bitscan_forward(uint64_t bb)
    {
#if defined(_MSC_VER)
        unsigned long index;
        _BitScanForward64(&index, bb);
        return static_cast<uint8_t>(index);
#else
        return static_cast<uint8_t>(__builtin_ctzll(bb));
#endif
    }

    /**
     * @brief Performs a reverse bitscan to find the index of the most significant set bit (MSB).
     * @param bb The 64-bit bitboard to scan.
     * @returns The 0-indexed position of the most significant set bit.
     */
    inline uint8_t bitscan_reverse(uint64_t bb)
    {
#if defined(_MSC_VER)
        unsigned long index;
        _BitScanReverse64(&index, bb);
        return static_cast<uint8_t>(index);
#else
        return static_cast<uint8_t>(63 - __builtin_clzll(bb));
#endif
    }

    constexpr uint8_t BLACK = 1;

    constexpr int BOARD_MAX_LEFT = 0;
    constexpr int BOARD_MAX_RIGHT = 7;
    constexpr int BOARD_SIZE = 8;

#ifdef NDEBUG
    constexpr bool DEBUG = false;
#else
    constexpr bool DEBUG = true;
#endif

    enum class MoveType : uint8_t
    {
        CAPTURE,
        CASTLE_KINGSIDE,
        CASTLE_QUEENSIDE,
        EN_PASSANT,
        NORMAL,
        PROMOTION
    };

    constexpr uint8_t WHITE = 0;
}

namespace io
{
    constexpr const char *PGN_FILE = "pgn/fenrir.pgn";
    constexpr const char *PGN_FILE_STORE = "pgn/fenrir.store.txt";
}

namespace logger
{
    enum class LEVEL : uint8_t
    {
        CRITICAL,
        DEBUG,
        ERROR,
        INFO,
        WARN
    };

    constexpr const char *LOG_FILE = "logs/fenrir.log";
    constexpr const long MAX_LOG_SIZE = 5 * 1024 * 1024;
}

namespace test
{
    /**
     * @brief Retrieves the value of the "CI" environment variable.
     * @returns A pointer to the CI environment variable's value string, or nullptr if not defined.
     */
    inline const char *get_ci()
    {
#ifdef _MSC_VER
        char *val = nullptr;
        size_t len = 0;
        _dupenv_s(&val, &len, "CI");
        static const char *CI = val;
        return CI;
#else
        static const char *CI = getenv("CI");
        return CI;
#endif
    }
}
