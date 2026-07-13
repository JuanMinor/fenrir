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

#include "include/pgn/pgn.h"

namespace io
{
    /**
     * @brief Private constructor to enforce the Singleton pattern.
     */
    Pgn::Pgn()
    {
        logger::INFO("Portable Game Notation (PGN) initialized");
    }

    /**
     * @brief Resets stream flags and seeks the output stream to the beginning of the file.
     * @param os The output stream to modify.
     */
    void Pgn::clear_stream_flags(std::ostream &os) const
    {
        os.seekp(std::ios_base::beg);
        os.clear();
    }

    /**
     * @brief Writes standard PGN metadata tags to the output stream.
     * @param os The output stream to write metadata to.
     */
    void Pgn::set_metadata(std::ostream &os) const
    {
        auto date = chrono::Chrono().get_time_with_format("%Y.%m.%d");
        this->clear_stream_flags(os);
        os << "[Event \"User vs. Fenrir\"]\n"
           << "[Site \"Remote server - atom\"]\n"
           << "[Date \"" << date << "\"]\n"
           << "[Round \"1\"]\n"
           << "[White \"User\"]\n"
           << "[Black \"Fenrir\"]\n"
           << "[Result \"-\"]\n\n";
    }

    /**
     * @brief Destructor for the Pgn class.
     */
    Pgn::~Pgn() {}

    /**
     * @brief Reads all recorded moves from the store and generates a final formatted PGN file with metadata tags.
     */
    void Pgn::create(void) const
    {
        std::lock_guard<std::mutex> lock(pgn_mutex);
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

        this->set_metadata(pgnFile);

        std::string whiteMove, blackMove;
        unsigned long long move_count = 1;
        while (std::getline(storeFile, whiteMove))
        {
            pgnFile << move_count << ". " << whiteMove;
            if (std::getline(storeFile, blackMove))
            {
                pgnFile << " " << blackMove << " ";
            }
            else
            {
                pgnFile << " ";
            }
            move_count++;
        }
    }

    /**
     * @brief Retrieves the single global instance of the PGN recorder.
     * @returns A reference to the Singleton Pgn instance.
     */
    Pgn &Pgn::get_instance()
    {
        static Pgn instance;
        return instance;
    }

    /**
     * @brief Appends a single move to the temporary PGN move store file.
     * @param move The move string to record.
     */
    void Pgn::record(const std::string &move) const
    {
        std::lock_guard<std::mutex> lock(pgn_mutex);
        std::ofstream file(PGN_FILE_STORE, std::ios_base::app);
        if (!file)
        {
            logger::ERROR("Cannot open file: '" + std::string(PGN_FILE_STORE) + "'");
            return;
        }
        file << move << '\n';
    }
}
