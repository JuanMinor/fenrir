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

#pragma once

#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <mutex>
#include "include/chrono/chrono.h"
#include "include/logger/logger.h"

namespace io
{
    class Pgn
    {
    private:
        /**
         * @brief Private constructor to enforce the Singleton pattern.
         */
        Pgn();

        /**
         * @brief Resets stream flags and seeks the output stream to the beginning of the file.
         * @param os The output stream to modify.
         */
        void clear_stream_flags(std::ostream &os) const;

        mutable std::mutex pgn_mutex;

        /**
         * @brief Writes standard PGN metadata tags to the output stream.
         * @param os The output stream to write metadata to.
         */
        void set_metadata(std::ostream &os) const;

    public:
        /**
         * @brief Destructor for the Pgn class.
         */
        ~Pgn();

        Pgn(const Pgn &) = delete;
        Pgn(Pgn &&) = delete;
        Pgn &operator=(const Pgn &) = delete;
        Pgn &operator=(Pgn &&) = delete;

        /**
         * @brief Reads all recorded moves from the store and generates a final formatted PGN file with metadata tags.
         */
        void create(void) const;

        /**
         * @brief Retrieves the single global instance of the PGN recorder.
         * @returns A reference to the Singleton Pgn instance.
         */
        static Pgn &get_instance();

        /**
         * @brief Appends a single move to the temporary PGN move store file.
         * @param move The move string to record.
         */
        void record(const std::string &move) const;
    };

#define PGN Pgn::get_instance()
#define PGN_CREATE() PGN.create();
#define PGN_RECORD(MOVE) PGN.record(MOVE);
}
