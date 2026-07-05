#include <gtest/gtest.h>
#include "include/abi/c_api.h"
#include <cstring>
#include <string>

class CApiTest : public ::testing::Test {
protected:
    void* engine = nullptr;

    void SetUp() override {
        engine = fenrir_create();
    }

    void TearDown() override {
        if (engine) {
            fenrir_destroy(engine);
        }
    }
};

TEST_F(CApiTest, MakeMoveValidAndInvalid) {
    EXPECT_TRUE(fenrir_make_move(engine, "e2", "e4"));
    /* Invalid move - it's black's turn now */
    EXPECT_FALSE(fenrir_make_move(engine, "d2", "d4")); 
    /* Invalid move format or pseudo-illegal */
    EXPECT_FALSE(fenrir_make_move(engine, "e8", "e9"));
}

TEST_F(CApiTest, GenerateAllMoves) {
    char buffer[1024];
    int count = fenrir_generate_all_moves(engine, buffer, sizeof(buffer));
    EXPECT_EQ(count, 20); /* 20 initial moves for white */
    EXPECT_TRUE(strstr(buffer, "e2e4") != nullptr);
}

TEST_F(CApiTest, GetFen) {
    char buffer[256];
    fenrir_get_fen(engine, buffer, sizeof(buffer));
    EXPECT_STREQ(buffer, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

TEST_F(CApiTest, PrintBoard) {
    testing::internal::CaptureStdout();
    fenrir_print_board(engine);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_FALSE(output.empty());
}
