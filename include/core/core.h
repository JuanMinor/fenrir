/*
 *   Copyright (c) 2025 Juan Minor

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

#ifndef CORE_H
#define CORE_H

namespace color
{
    // @typedefs
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

namespace io
{
#define PGN_FILE "pgn/loki.pgn"
#define PGN_FILE_STORE "pgn/loki.store.txt"
}

namespace logger
{
#define LOG_FILE "logs/loki.log"

    // @typedefs
    typedef enum
    {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL
    } LEVEL;
}

namespace loki
{
#define DEBUG_ENABLED 1
#define BOARD_SIZE 8
#define WHITE 0
#define BLACK 1
}

#endif