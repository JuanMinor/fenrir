/*
    log.cpp
    Author: M., Juan
    Date: 10/31/2023
*/

#include <fstream>
#include <unordered_map>

#include "include/logger/logger.h"

namespace logger
{

    std::unordered_map<uint8_t, const char *> level_types = {
        {DEBUG, "[DEBUG] - "},
        {INFO, "[INFO] - "},
        {WARN, "[WARN] - "},
        {ERROR, "[ERROR] - "},
        {CRITICAL, "[CRITICAL] - "}};

    Logger::Logger() {}
    Logger::~Logger() {}

    std::_Put_time<char> Logger::__get_timestamp__()
    {
        time_t time = std::time(nullptr);
        tm *localtime = std::localtime(&time);
        return std::put_time(localtime, "%a %b %d, %Y @ %H:%M:%S");
    }

    void Logger::log(const std::string &__message, const char *__file, const uint8_t &__lineno, const LEVEL &__level) noexcept
    {
        if (__level == DEBUG)
        {
            if (!DEBUG_ENABLED)
            {
                return;
            }
        }
        std::_Put_time<char> timestamp = this->__get_timestamp__();

        // @file
        std::fstream file;
        file.open(L_FILE, std::ios_base::app);
        if (!file)
        {
            return;
        }
        try
        {
            const char *type = level_types.at(__level);
            file << "[" << timestamp << "] [" << __file << " @ Line " << unsigned(__lineno) << "]::" << type << __message << std::endl;
            file.close();
        }
        catch (...)
        {
            // @close
            file.close();
        }
    }
}