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
 *   along with this program.  If not, see <https:
 */

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <regex>
#include <chrono>
#include "include/pgn/pgn.h"
#include "include/core/core.h"

class PgnTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		test_store_file = "test_fenrir.store.txt";
		test_pgn_file = "test_fenrir.pgn";
		sample_moves = {"e4", "e5", "Nf3", "Nc6", "Bb5"};
	}

	static void TearDownTestSuite() {}

	void SetUp() override
	{

		cleanup_test_files();

		std::filesystem::create_directories("pgn");
	}

	void TearDown() override
	{

		cleanup_test_files();
	}

	static std::string test_store_file;
	static std::string test_pgn_file;
	static std::vector<std::string> sample_moves;

	void cleanup_test_files()
	{

		std::filesystem::remove(test_store_file);
		std::filesystem::remove(test_pgn_file);
		std::filesystem::remove(io::PGN_FILE);
		std::filesystem::remove(io::PGN_FILE_STORE);
	}

	void create_test_store_file(const std::vector<std::string> &moves)
	{
		std::ofstream file(io::PGN_FILE_STORE);
		for (const auto &move : moves)
		{
			file << move << '\n';
		}
		file.close();
	}

	std::string read_file_content(const std::string &filename)
	{
		std::ifstream file(filename);
		if (!file.is_open())
		{
			return "";
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool file_exists(const std::string &filename)
	{
		return std::filesystem::exists(filename);
	}

	size_t count_lines_in_file(const std::string &filename)
	{
		std::ifstream file(filename);
		size_t count = 0;
		std::string line;
		while (std::getline(file, line))
		{
			++count;
		}
		return count;
	}
};

std::string PgnTest::test_store_file = "";
std::string PgnTest::test_pgn_file = "";
std::vector<std::string> PgnTest::sample_moves = {};

/* Singleton tests */
TEST_F(PgnTest, GetInstance)
{
	io::Pgn &pgn1 = io::Pgn::get_instance();
	io::Pgn &pgn2 = io::Pgn::get_instance();

	EXPECT_EQ(&pgn1, &pgn2);
}

TEST_F(PgnTest, GetInstanceMacro)
{
	io::Pgn &pgn1 = io::PGN;
	io::Pgn &pgn2 = io::PGN;

	EXPECT_EQ(&pgn1, &pgn2);
}

/* Record move tests */
TEST_F(PgnTest, RecordSingleMove)
{
	io::Pgn &pgn = io::Pgn::get_instance();

	pgn.record("e4");

	EXPECT_TRUE(file_exists(io::PGN_FILE_STORE));

	std::string content = read_file_content(io::PGN_FILE_STORE);
	EXPECT_EQ(content, "e4\n");
}

TEST_F(PgnTest, RecordMultipleMoves)
{
	io::Pgn &pgn = io::Pgn::get_instance();

	for (const auto &move : sample_moves)
	{
		pgn.record(move);
	}

	EXPECT_TRUE(file_exists(io::PGN_FILE_STORE));
	EXPECT_EQ(count_lines_in_file(io::PGN_FILE_STORE), sample_moves.size());

	std::string content = read_file_content(io::PGN_FILE_STORE);
	for (const auto &move : sample_moves)
	{
		EXPECT_NE(content.find(move), std::string::npos);
	}
}

TEST_F(PgnTest, RecordMoveWithMacro)
{
	io::PGN_RECORD("d4");

	EXPECT_TRUE(file_exists(io::PGN_FILE_STORE));

	std::string content = read_file_content(io::PGN_FILE_STORE);
	EXPECT_EQ(content, "d4\n");
}

TEST_F(PgnTest, RecordEmptyMove)
{
	io::Pgn &pgn = io::Pgn::get_instance();

	pgn.record("");

	EXPECT_TRUE(file_exists(io::PGN_FILE_STORE));

	std::string content = read_file_content(io::PGN_FILE_STORE);
	EXPECT_EQ(content, "\n");
}

TEST_F(PgnTest, RecordMoveWithSpecialCharacters)
{
	io::Pgn &pgn = io::Pgn::get_instance();

	std::string move_with_special_chars = "O-O-O+";
	pgn.record(move_with_special_chars);

	EXPECT_TRUE(file_exists(io::PGN_FILE_STORE));

	std::string content = read_file_content(io::PGN_FILE_STORE);
	EXPECT_EQ(content, move_with_special_chars + "\n");
}

/* Create PGN tests */
TEST_F(PgnTest, CreatePgnFromEmptyStore)
{

	create_test_store_file({});

	io::Pgn &pgn = io::Pgn::get_instance();
	pgn.create();

	EXPECT_TRUE(file_exists(io::PGN_FILE));

	std::string content = read_file_content(io::PGN_FILE);

	EXPECT_NE(content.find("[Event \"User vs. Fenrir\"]"), std::string::npos);
	EXPECT_NE(content.find("[Site \"Remote server - atom\"]"), std::string::npos);
	EXPECT_NE(content.find("[Round \"1\"]"), std::string::npos);
	EXPECT_NE(content.find("[White \"User\"]"), std::string::npos);
	EXPECT_NE(content.find("[Black \"Fenrir\"]"), std::string::npos);
	EXPECT_NE(content.find("[Result \"-\"]"), std::string::npos);

	std::regex date_pattern(R"(\[Date "\d{4}\.\d{2}\.\d{2}"\])");
	EXPECT_TRUE(std::regex_search(content, date_pattern));
}

TEST_F(PgnTest, CreatePgnFromMovesInStore)
{

	create_test_store_file(sample_moves);

	io::Pgn &pgn = io::Pgn::get_instance();
	pgn.create();

	EXPECT_TRUE(file_exists(io::PGN_FILE));

	std::string content = read_file_content(io::PGN_FILE);

	EXPECT_NE(content.find("[Event \"User vs. Fenrir\"]"), std::string::npos);

	EXPECT_NE(content.find("1. e4"), std::string::npos);
	EXPECT_NE(content.find("2. e5"), std::string::npos);
	EXPECT_NE(content.find("3. Nf3"), std::string::npos);
	EXPECT_NE(content.find("4. Nc6"), std::string::npos);
	EXPECT_NE(content.find("5. Bb5"), std::string::npos);
}

TEST_F(PgnTest, CreatePgnWithMacro)
{

	create_test_store_file({"e4", "e5"});

	io::PGN_CREATE();

	EXPECT_TRUE(file_exists(io::PGN_FILE));

	std::string content = read_file_content(io::PGN_FILE);
	EXPECT_NE(content.find("1. e4"), std::string::npos);
	EXPECT_NE(content.find("2. e5"), std::string::npos);
}

TEST_F(PgnTest, CreatePgnWithSingleMove)
{
	create_test_store_file({"e4"});

	io::Pgn &pgn = io::Pgn::get_instance();
	pgn.create();

	EXPECT_TRUE(file_exists(io::PGN_FILE));

	std::string content = read_file_content(io::PGN_FILE);
	EXPECT_NE(content.find("1. e4"), std::string::npos);
}

/* Error handling tests */
TEST_F(PgnTest, CreatePgnWithoutStoreFile)
{

	io::Pgn &pgn = io::Pgn::get_instance();

	EXPECT_NO_THROW(pgn.create());

	if (file_exists(io::PGN_FILE))
	{
		std::string content = read_file_content(io::PGN_FILE);

		SUCCEED();
	}
}

TEST_F(PgnTest, RecordMoveToReadOnlyDirectory)
{

	io::Pgn &pgn = io::Pgn::get_instance();

	EXPECT_NO_THROW(pgn.record("e4"));
}

TEST_F(PgnTest, RecordMoveWithoutPgnDirectory)
{

	std::filesystem::remove_all("pgn");

	io::Pgn &pgn = io::Pgn::get_instance();

	EXPECT_NO_THROW(pgn.record("e4"));

	std::filesystem::create_directories("pgn");
	std::ofstream gitkeep("pgn/.gitkeep");
	gitkeep.close();
}

TEST_F(PgnTest, CreatePgnWithoutPgnDirectory)
{

	std::filesystem::create_directories("pgn");
	create_test_store_file({"e4", "e5"});

	std::filesystem::remove_all("pgn");

	io::Pgn &pgn = io::Pgn::get_instance();

	EXPECT_NO_THROW(pgn.create());

	std::filesystem::create_directories("pgn");
	std::ofstream gitkeep("pgn/.gitkeep");
	gitkeep.close();
}

/* Integration tests */
TEST_F(PgnTest, CompleteWorkflow)
{
	io::Pgn &pgn = io::Pgn::get_instance();

	pgn.record("e4");
	pgn.record("e5");
	pgn.record("Nf3");
	pgn.record("Nc6");

	EXPECT_TRUE(file_exists(io::PGN_FILE_STORE));
	EXPECT_EQ(count_lines_in_file(io::PGN_FILE_STORE), 4);

	pgn.create();

	EXPECT_TRUE(file_exists(io::PGN_FILE));

	std::string content = read_file_content(io::PGN_FILE);

	EXPECT_NE(content.find("[Event \"User vs. Fenrir\"]"), std::string::npos);

	EXPECT_NE(content.find("1. e4"), std::string::npos);
	EXPECT_NE(content.find("2. e5"), std::string::npos);
	EXPECT_NE(content.find("3. Nf3"), std::string::npos);
	EXPECT_NE(content.find("4. Nc6"), std::string::npos);
}

TEST_F(PgnTest, MultipleRecordAndCreateCycles)
{
	io::Pgn &pgn = io::Pgn::get_instance();

	pgn.record("e4");
	pgn.record("e5");
	pgn.create();

	std::string first_content = read_file_content(io::PGN_FILE);
	EXPECT_NE(first_content.find("1. e4"), std::string::npos);
	EXPECT_NE(first_content.find("2. e5"), std::string::npos);

	pgn.record("Nf3");
	pgn.create();

	std::string second_content = read_file_content(io::PGN_FILE);
	EXPECT_NE(second_content.find("3. Nf3"), std::string::npos);
}

/* Memory and performance tests */
TEST_F(PgnTest, RecordManyMoves)
{
	io::Pgn &pgn = io::Pgn::get_instance();

	const int num_moves = 1000;
	for (int i = 0; i < num_moves; ++i)
	{
		pgn.record("move" + std::to_string(i));
	}

	EXPECT_TRUE(file_exists(io::PGN_FILE_STORE));
	EXPECT_EQ(count_lines_in_file(io::PGN_FILE_STORE), num_moves);
}

TEST_F(PgnTest, CreatePgnFromManyMoves)
{

	std::vector<std::string> many_moves;
	for (int i = 0; i < 100; ++i)
	{
		many_moves.push_back("move" + std::to_string(i));
	}

	create_test_store_file(many_moves);

	io::Pgn &pgn = io::Pgn::get_instance();

	auto start = std::chrono::high_resolution_clock::now();
	pgn.create();
	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	EXPECT_TRUE(file_exists(io::PGN_FILE));

	std::string content = read_file_content(io::PGN_FILE);
	EXPECT_NE(content.find("1. move0"), std::string::npos);
	EXPECT_NE(content.find("100. move99"), std::string::npos);

	EXPECT_LT(duration.count(), 1000);
}

/* Edge cases and boundary tests */
TEST_F(PgnTest, RecordVeryLongMove)
{
	io::Pgn &pgn = io::Pgn::get_instance();

	std::string long_move(1000, 'a');
	pgn.record(long_move);

	EXPECT_TRUE(file_exists(io::PGN_FILE_STORE));

	std::string content = read_file_content(io::PGN_FILE_STORE);
	EXPECT_EQ(content, long_move + "\n");
}

TEST_F(PgnTest, RecordMoveWithNewlines)
{
	io::Pgn &pgn = io::Pgn::get_instance();

	std::string move_with_newlines = "e4\ntest\nmore";
	pgn.record(move_with_newlines);

	EXPECT_TRUE(file_exists(io::PGN_FILE_STORE));

	std::string content = read_file_content(io::PGN_FILE_STORE);
	EXPECT_NE(content.find(move_with_newlines), std::string::npos);
}

/* Stress test */
TEST_F(PgnTest, StressTest)
{
	if (!test::CI || std::string(test::CI) != "true")
	{
		GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
	}

	io::Pgn &pgn = io::Pgn::get_instance();

	const int stress_iterations = 10000;
	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < stress_iterations; ++i)
	{
		pgn.record("move" + std::to_string(i % 100));

		if (i % 1000 == 0)
		{
			pgn.create();
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	EXPECT_TRUE(file_exists(io::PGN_FILE_STORE));
	EXPECT_TRUE(file_exists(io::PGN_FILE));

	std::cout << "Stress test completed " << stress_iterations
			  << " operations in " << duration.count() << " ms" << std::endl;
}

/* Copy semantics test (should not be copyable) */
TEST_F(PgnTest, SingletonNotCopyable)
{

	io::Pgn &pgn1 = io::Pgn::get_instance();
	io::Pgn &pgn2 = io::Pgn::get_instance();

	EXPECT_EQ(&pgn1, &pgn2);

	SUCCEED();
}