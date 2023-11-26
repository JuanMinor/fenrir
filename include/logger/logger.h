/*
    log.h
    Author: M., Juan
    Date: 10/31/2023
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <iomanip>
#include <string>
#include <stdint.h>

#include "include/core/core.h"

namespace logger
{
    class Logger
    {
        // @methods
        std::_Put_time<char> __get_timestamp__();

    public:
        Logger();
        ~Logger();

        // @methods
        void log(const std::string &__message, const char *__file, const uint8_t &__lineno, const LEVEL &__level) noexcept;
    };

    // @macros
#define LOG Logger()

#define LOG_DEBUG(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::DEBUG);
#define LOG_INFO(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::INFO);
#define LOG_WARN(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::WARN);
#define LOG_ERROR(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::ERROR);
#define LOG_CRITICAL(MESSAGE) LOG.log(MESSAGE, __FILE__, __LINE__, logger::CRITICAL);
}

#endif