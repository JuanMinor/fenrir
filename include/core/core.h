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

/* Export dll symbols */
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

namespace color
{
	enum class Color : uint8_t
	{
		RESET = 0,
		FG_BLACK = 30,
		BG_BLACK = 40,
		FG_RED = 31,
		BG_RED = 41,
		FG_GREEN = 32,
		BG_GREEN = 42,
		FG_YELLOW = 33,
		BG_YELLOW = 43,
		FG_BLUE = 34,
		BG_BLUE = 44,
		FG_MAGENTA = 35,
		BG_MAGENTA = 45,
		FG_CYAN = 36,
		BG_CYAN = 46,
		FG_WHITE = 37,
		BG_WHITE = 47
	};
}

namespace fenrir
{
#ifdef NDEBUG
	constexpr bool DEBUG = false;
#else
	constexpr bool DEBUG = true;
#endif

	constexpr int BOARD_SIZE = 8;
	constexpr int BOARD_MAX_LEFT = 0;
	constexpr int BOARD_MAX_RIGHT = 7;

	constexpr uint8_t WHITE = 0;
	constexpr uint8_t BLACK = 1;



	enum class MoveType : uint8_t
	{
		NORMAL,
		CAPTURE,
		EN_PASSANT,
		CASTLE_KINGSIDE,
		CASTLE_QUEENSIDE,
		PROMOTION
	};

	// Cross-platform bitscan utilities
	inline uint8_t bitscan_forward(uint64_t bb) {
#if defined(_MSC_VER)
		unsigned long index;
		_BitScanForward64(&index, bb);
		return static_cast<uint8_t>(index);
#else
		return static_cast<uint8_t>(__builtin_ctzll(bb));
#endif
	}

	inline uint8_t bitscan_reverse(uint64_t bb) {
#if defined(_MSC_VER)
		unsigned long index;
		_BitScanReverse64(&index, bb);
		return static_cast<uint8_t>(index);
#else
		return static_cast<uint8_t>(63 - __builtin_clzll(bb));
#endif
	}
}

namespace io
{
	constexpr const char *PGN_FILE = "pgn/fenrir.pgn";
	constexpr const char *PGN_FILE_STORE = "pgn/fenrir.store.txt";
}

namespace logger
{
	constexpr const char *LOG_FILE = "logs/fenrir.log";
	constexpr const long MAX_LOG_SIZE = 5 * 1024 * 1024;

	enum class LEVEL : uint8_t
	{
		DEBUG,
		INFO,
		WARN,
		ERROR,
		CRITICAL
	};
}

namespace test
{
	inline const char *get_ci()
	{
#ifdef _MSC_VER
		char* val = nullptr;
		size_t len = 0;
		_dupenv_s(&val, &len, "CI");
		static const char* CI = val;
		return CI;
#else
		static const char *CI = getenv("CI");
		return CI;
#endif
	}
}

