/*
    log.h
    Author: M., Juan
    Date: 10/31/2023
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <iomanip>
#include <string>

#include "include/core/core.h"

namespace Logger
{

    class Log
    {
        // @methods
        std::_Put_time<char> __get_timestamp__();

    public:
        Log();
        ~Log();

        // @methods
        void log(std::string __message, const char *__file, uint8_t __lineno, LEVEL __level);
    };

    // @macros
#define LOG Log()
#define LOG_DEBUG(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, Logger::DEBUG);
#define LOG_INFO(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, Logger::INFO);
#define LOG_WARN(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, Logger::WARN);
#define LOG_ERROR(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, Logger::ERROR);
#define LOG_CRITICAL(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, Logger::CRITICAL);
}

#endif