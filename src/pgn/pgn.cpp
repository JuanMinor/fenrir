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

#include "include/pgn/pgn.h"

namespace io
{
    Pgn::Pgn()
    {
        logger::INFO("Portable Game Notation (PGN) initialized");
    }

    Pgn::~Pgn() {}

    Pgn &Pgn::get_instance()
    {
        static Pgn instance;
        logger::DEBUG("PGN instance created and returned. Only one instance will be used throughout the application.");
        return instance;
    }

    void Pgn::__clear_stream_flags__(std::ostream &__os) const
    {
        __os.seekp(std::ios_base::beg);
        __os.clear();
    }

    void Pgn::__set_metadata__(std::ostream &__os) const
    {
        auto date = chrono::Chrono().get_time_with_format("%Y.%m.%d");
        std::stringstream ss;
        ss << "[Event \"User vs. Fenrir\"]\n"
           << "[Site \"Remote server - atom\"]\n"
           << "[Date \"" << date << "\"]\n"
           << "[Round \"1\"]\n"
           << "[White \"User\"]\n"
           << "[Black \"Fenrir\"]\n"
           << "[Result \"-\"]\n";

        this->__clear_stream_flags__(__os);
        __os << ss.str();
    }

    void Pgn::record(const std::string &__move) const
    {
        std::ofstream file(PGN_FILE_STORE, std::ios_base::app);
        if (!file)
        {
            logger::ERROR("Cannot open file: '" + std::string(PGN_FILE_STORE) + "'");
            return;
        }
        file << __move << '\n';
    }

    void Pgn::create(void) const
    {
        std::ifstream storeFile(PGN_FILE_STORE);
        std::ofstream pgnFile(PGN_FILE);

        if (!pgnFile)
        {
            logger::ERROR("Cannot open file: '" + std::string(PGN_FILE) + "'");
            return;
        }
        if (!storeFile)
        {
            logger::ERROR("Cannot open file: '" + std::string(PGN_FILE_STORE) + "'");
            return;
        }

        this->__set_metadata__(pgnFile);

        std::string move;
        unsigned long long moveCount = 0;
        while (std::getline(storeFile, move))
        {
            pgnFile << ++moveCount << ". " << move << " ";
        }
    }
}