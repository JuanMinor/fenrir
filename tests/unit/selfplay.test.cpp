#include <gtest/gtest.h>
#include "include/selfplay/selfplay.h"
#include <filesystem>

using namespace fenrir;

TEST(SelfPlayTest, RunGameDefault) {
    SelfPlay sp(0, 1, 1);
    sp.run();
    EXPECT_TRUE(true);
}

TEST(SelfPlayTest, RunGameWhiteMated) {
    // Black mates White
    SelfPlay sp(0, 1, 1, "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
    sp.run();
    EXPECT_TRUE(true);
}

TEST(SelfPlayTest, RunGameBlackMated) {
    // White mates Black
    SelfPlay sp(0, 1, 1, "rnbqkbnr/ppppp2p/5p2/6pQ/4P3/8/PPPP1PPP/RNB1KBNR b KQkq - 1 3");
    sp.run();
    EXPECT_TRUE(true);
}
