/*
    pgn.cpp
    Author: M., Juan
    Date: 10/31/2023
*/

#include "include/logger/logger.h"
#include "include/pgn/pgn.h"

namespace pgn
{
    Pgn::Pgn() {}
    Pgn::~Pgn() {}

    void Pgn::__clear_stream_flags__(std::ostream &__os) noexcept
    {
        __os.seekp(std::ios_base::beg);
        __os.clear();
    }

    std::_Put_time<char> Pgn::__get_pgn_date__(void) noexcept
    {
        time_t time = std::time(nullptr);
        tm *localtime = std::localtime(&time);
        return std::put_time(localtime, "%Y.%m.%d");
    }

    void Pgn::__set_metadata__(std::ostream &__os) noexcept
    {
        std::_Put_time<char> date = this->__get_pgn_date__();
        std::stringstream ss;
        ss << "[Event \"User vs. Loki\"]" << std::endl
           << "[Site \"Remote server - atom\"]" << std::endl
           << "[Date \"" << date << "\"]" << std::endl
           << "[Round \"1\"]" << std::endl
           << "[White \"User\"]" << std::endl
           << "[Black \"Loki\"]" << std::endl
           << "[Result \"-\"]" << std::endl;

        this->__clear_stream_flags__(__os);
        __os << ss.str();
        return;
    }

    void Pgn::record(const std::string &__move) noexcept
    {
        std::fstream file;
        file.open(P_FILE_STORE, std::ios_base::app);
        if (!file)
        {
            std::stringstream ss;
            ss << "Cannot open file: '" << P_FILE_STORE << "'" << std::endl;
            logger::LOG_ERROR(ss.str());
            return;
        }
        file << __move << std::endl;
        file.close();
    }

    void Pgn::create_pgn(void) noexcept
    {
        std::fstream pgnFile, storeFile;
        pgnFile.open(P_FILE, std::ios::out);
        storeFile.open(P_FILE_STORE, std::ios::in);
        if (!pgnFile)
        {
            std::stringstream ss;
            ss << "Cannot open file: '" << P_FILE << "'" << std::endl;
            logger::LOG_ERROR(ss.str());
            return;
        }
        if (!storeFile)
        {
            std::stringstream ss;
            ss << "Cannot open file: '" << P_FILE_STORE << "'" << std::endl;
            logger::LOG_ERROR(ss.str());
            return;
        }
        this->__set_metadata__(pgnFile);
        // write moves to file
        std::string move;
        unsigned long long moves = 0;
        while (std::getline(storeFile, move))
        {
            pgnFile << ++moves << ". " << move << " ";
        }
        pgnFile.close();
        storeFile.close();
    }

}