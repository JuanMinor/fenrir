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

	Pgn &Pgn::getInstance()
	{
		static Pgn instance;
		logger::DEBUG("PGN instance created and returned. Only one instance will be used throughout the application.");
		return instance;
	}

	void Pgn::clearStreamFlags(std::ostream &os) const
	{
		os.seekp(std::ios_base::beg);
		os.clear();
	}

	void Pgn::setMetadata(std::ostream &os) const
	{
		auto date = chrono::Chrono().getTimeWithFormat("%Y.%m.%d");
		std::stringstream ss;
		ss << "[Event \"User vs. Fenrir\"]\n"
		   << "[Site \"Remote server - atom\"]\n"
		   << "[Date \"" << date << "\"]\n"
		   << "[Round \"1\"]\n"
		   << "[White \"User\"]\n"
		   << "[Black \"Fenrir\"]\n"
		   << "[Result \"-\"]\n";

		this->clearStreamFlags(os);
		os << ss.str();
	}

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

		this->setMetadata(pgnFile);

		std::string move;
		unsigned long long moveCount = 0;
		while (std::getline(storeFile, move))
		{
			pgnFile << ++moveCount << ". " << move << " ";
		}
	}
}
