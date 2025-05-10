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

#ifndef CORE_H
#define CORE_H

#include <cstdlib>

// Namespace for color-related constants
namespace color
{
    typedef enum
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
    } Color;
}

// Namespace for input/output-related constants
namespace io
{
    constexpr const char *PGN_FILE = "pgn/loki.pgn";
    constexpr const char *PGN_FILE_STORE = "pgn/loki.store.txt";
}

// Namespace for logger-related constants
namespace logger
{
    constexpr const char *LOG_FILE = "logs/loki.log";
    constexpr const long MAX_LOG_SIZE = 5 * 1024 * 1024; // 5 MB

    typedef enum
    {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL
    } LEVEL;
}

// Namespace for Loki engine constants
namespace loki
{
    constexpr bool DEBUG_ENABLED = true;

    constexpr int BOARD_SIZE = 8;
    constexpr int BOARD_MAX_LEFT = 0;
    constexpr int BOARD_MAX_RIGHT = 7;

    constexpr int WHITE = 0;
    constexpr int BLACK = 1;
}

namespace test
{
    static const char *CI = getenv("CI");
}

#endif // CORE_H