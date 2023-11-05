/*
    core.h
    Author: M., Juan
    Date: 10/31/2023
*/

#ifndef CORE_H
#define CORE_H
namespace Logger
{
#define FILE "logs/loki.log"

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

namespace Loki
{
#define DEBUG_ENABLED 1

#define BOARD_SIZE 8
#define WHITE 0
#define BLACK 1
}

#endif