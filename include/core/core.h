/*
    core.h
    Author: M., Juan
    Date: 10/31/2023
*/

#ifndef CORE_H
#define CORE_H

namespace logger
{
#define L_FILE "logs/loki.log"

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

namespace pgn
{
#define P_FILE "pgn/loki.pgn"
#define P_FILE_STORE "pgn/loki.store.txt"
}

namespace loki
{
#define DEBUG_ENABLED 1
#define BOARD_SIZE 8
#define WHITE 0
#define BLACK 1
}

#endif