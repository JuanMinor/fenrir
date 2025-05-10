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

#ifndef PGN_H
#define PGN_H

#include <fstream>
#include <iomanip>
#include <string>

namespace io
{
    class Pgn
    {
    private:
        // Helper methods
        void __clear_stream_flags__(std::ostream &__os) const;
        void __set_metadata__(std::ostream &__os) const;

    public:
        Pgn();
        ~Pgn();

        // Public methods
        void update_metadata(const std::string &__tag);
        void record(const std::string &__move) const;
        void create(void) const;
    };

    // Macros for PGN operations
#define PGN Pgn()
#define PGN_RECORD(MOVE) PGN.record(MOVE);
#define PGN_CREATE() PGN.create();
}

#endif // PGN_H