/*
 *   Copyright (c) 2026 Juan Minor
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

#include <gtest/gtest.h>
#include "include/uci/uci.h"
#include <sstream>

using namespace chess;

TEST(UCITest, LoopCommands) {
    std::istringstream in("uci\nisready\nucinewgame\nposition startpos\nposition fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\nposition startpos moves e2e4\nposition startpos moves e2e4 e7e5\ngo nodes 10\nquit\n");
    std::ostringstream out;
    
    std::streambuf* cinbuf = std::cin.rdbuf();
    std::streambuf* coutbuf = std::cout.rdbuf();
    
    std::cin.rdbuf(in.rdbuf());
    std::cout.rdbuf(out.rdbuf());
    
    UCI uci;
    uci.loop();
    
    std::cin.rdbuf(cinbuf);
    std::cout.rdbuf(coutbuf);
    
    std::string output = out.str();
    EXPECT_TRUE(output.find("uciok") != std::string::npos);
    EXPECT_TRUE(output.find("readyok") != std::string::npos);
    EXPECT_TRUE(output.find("bestmove") != std::string::npos);
}

TEST(UCITest, LoopCommandsUnknownGo) {
    std::istringstream in("go unknwn 10\nquit\n");
    std::ostringstream out;
    std::streambuf* cinbuf = std::cin.rdbuf();
    std::streambuf* coutbuf = std::cout.rdbuf();
    std::cin.rdbuf(in.rdbuf());
    std::cout.rdbuf(out.rdbuf());
    
    UCI uci;
    uci.loop();
    
    std::cin.rdbuf(cinbuf);
    std::cout.rdbuf(coutbuf);
}

TEST(UCITest, LoopCommandsTimeManagement) {
    std::istringstream in("position startpos\ngo movetime 10\ngo wtime 30000 btime 30000\ngo wtime 100 btime 100\nquit\n");
    std::ostringstream out;
    std::streambuf* cinbuf = std::cin.rdbuf();
    std::streambuf* coutbuf = std::cout.rdbuf();
    std::cin.rdbuf(in.rdbuf());
    std::cout.rdbuf(out.rdbuf());
    
    UCI uci;
    uci.loop();
    
    std::cin.rdbuf(cinbuf);
    std::cout.rdbuf(coutbuf);
    
    std::string output = out.str();
    EXPECT_TRUE(output.find("bestmove") != std::string::npos);
}

