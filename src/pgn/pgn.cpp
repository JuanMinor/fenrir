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

#include "include/chrono/chrono.h"
#include "include/logger/logger.h"
#include "include/pgn/pgn.h"

namespace io
{
    Pgn::Pgn() {}
    Pgn::~Pgn() {}

    void Pgn::__clear_stream_flags__(std::ostream &__os) const
    {
        __os.seekp(std::ios_base::beg);
        __os.clear();
    }

    void Pgn::__set_metadata__(std::ostream &__os) const
    {
        std::_Put_time<char> date = chrono::Chrono().get_time_with_format("%Y.%m.%d");
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

    void Pgn::record(const std::string &__move) const
    {
        std::fstream file;
        file.open(PGN_FILE_STORE, std::ios_base::app);
        if (!file)
        {
            std::stringstream ss;
            ss << "Cannot open file: '" << PGN_FILE_STORE << "'" << std::endl;
            logger::LOG_ERROR(ss.str());
            return;
        }
        file << __move << std::endl;
        file.close();
    }

    void Pgn::create(void) const
    {
        std::fstream pgnFile, storeFile;
        pgnFile.open(PGN_FILE, std::ios::out);
        storeFile.open(PGN_FILE_STORE, std::ios::in);
        if (!pgnFile)
        {
            std::stringstream ss;
            ss << "Cannot open file: '" << PGN_FILE << "'" << std::endl;
            logger::LOG_ERROR(ss.str());
            return;
        }
        if (!storeFile)
        {
            std::stringstream ss;
            ss << "Cannot open file: '" << PGN_FILE_STORE << "'" << std::endl;
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