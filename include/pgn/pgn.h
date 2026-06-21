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
		mutable std::mutex pgn_mutex;
		Pgn();

		void clearStreamFlags(std::ostream &os) const;
		void setMetadata(std::ostream &os) const;

	public:
		static Pgn &getInstance();

		Pgn(const Pgn &) = delete;
		Pgn &operator=(const Pgn &) = delete;

		~Pgn();

		void updateMetadata(const std::string &tag);
		void record(const std::string &move) const;
		void create(void) const;
	};

#define PGN Pgn::getInstance()
#define PGN_RECORD(MOVE) PGN.record(MOVE);
#define PGN_CREATE() PGN.create();
}
