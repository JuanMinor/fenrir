#include <gtest/gtest.h>
#include "include/uci/uci.h"
#include <sstream>

using namespace fenrir;

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

