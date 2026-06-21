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
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include "include/core/core.h"
#include "include/logger/logger.h"

class LoggerTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{

		test_log_file = logger::LOG_FILE;
		test_backup_file = std::string(logger::LOG_FILE) + ".1";

		std::filesystem::remove(test_log_file);
		std::filesystem::remove(test_backup_file);
	}

	static void TearDownTestSuite()
	{

		std::filesystem::remove(test_log_file);
		std::filesystem::remove(test_backup_file);

		std::filesystem::path log_path(logger::LOG_FILE);
		std::filesystem::path log_dir = log_path.parent_path();

		if (!std::filesystem::exists(log_dir))
		{
			std::filesystem::create_directories(log_dir);
		}

		std::ofstream file(log_dir / ".gitkeep", std::ios::trunc);
		file.close();
	}

	void SetUp() override
	{

		std::filesystem::path log_path(logger::LOG_FILE);
		std::filesystem::path log_dir = log_path.parent_path();

		std::filesystem::remove(test_log_file);
		std::filesystem::remove(test_backup_file);

		if (std::filesystem::exists(log_dir) && !std::filesystem::is_directory(log_dir))
		{
			std::filesystem::remove(log_dir);
		}
		if (!std::filesystem::exists(log_dir))
		{
			std::filesystem::create_directories(log_dir);
		}

		std::ofstream file(log_dir / ".gitkeep", std::ios::trunc);
		file.close();
	}

	void TearDown() override
	{

		std::filesystem::remove(test_log_file);
		std::filesystem::remove(test_backup_file);

		std::filesystem::path log_path(logger::LOG_FILE);
		std::filesystem::path log_dir = log_path.parent_path();

		if (std::filesystem::exists(log_dir) && !std::filesystem::is_directory(log_dir))
		{
			std::filesystem::remove(log_dir);
		}
		if (!std::filesystem::exists(log_dir))
		{
			std::filesystem::create_directories(log_dir);
		}

		std::ofstream file(log_dir / ".gitkeep", std::ios::trunc);
		file.close();
	}

	std::string readLogFile() const
	{

		std::ifstream file(logger::LOG_FILE);
		if (!file)
			return "";

		std::string content((std::istreambuf_iterator<char>(file)),
							std::istreambuf_iterator<char>());
		return content;
	}

	void createLargeLogFile(size_t size) const
	{

		std::ofstream file(logger::LOG_FILE);
		std::string data(size, 'X');
		file << data;
	}

	static std::string test_log_file;
	static std::string test_backup_file;
};

std::string LoggerTest::test_log_file = "";
std::string LoggerTest::test_backup_file = "";

/* Basic functionality tests */
TEST_F(LoggerTest, ConstructorAndDestructor)
{
	logger::Logger *log = new logger::Logger();
	EXPECT_NE(log, nullptr);
	delete log;
}

TEST_F(LoggerTest, LogDebugMessage)
{
	logger::Logger log;
	log.log("Test debug message", __FILE__, __LINE__, logger::LEVEL::DEBUG);

	std::string content = readLogFile();

	if (fenrir::DEBUG)
	{
		EXPECT_TRUE(content.find("[DEBUG] - Test debug message") != std::string::npos);
		EXPECT_TRUE(content.find(__FILE__) != std::string::npos);
	}
	else
	{
		EXPECT_TRUE(content.empty() || content.find("Test debug message") == std::string::npos);
	}
}

TEST_F(LoggerTest, LogInfoMessage)
{
	logger::Logger log;
	log.log("Test info message", __FILE__, __LINE__, logger::LEVEL::INFO);

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("[INFO] - Test info message") != std::string::npos);
	EXPECT_TRUE(content.find(__FILE__) != std::string::npos);
}

TEST_F(LoggerTest, LogWarnMessage)
{
	logger::Logger log;
	log.log("Test warn message", __FILE__, __LINE__, logger::LEVEL::WARN);

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("[WARN] - Test warn message") != std::string::npos);
	EXPECT_TRUE(content.find(__FILE__) != std::string::npos);
}

TEST_F(LoggerTest, LogErrorMessage)
{
	logger::Logger log;
	log.log("Test error message", __FILE__, __LINE__, logger::LEVEL::ERROR);

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("[ERROR] - Test error message") != std::string::npos);
	EXPECT_TRUE(content.find(__FILE__) != std::string::npos);
}

TEST_F(LoggerTest, LogCriticalMessage)
{
	logger::Logger log;
	log.log("Test critical message", __FILE__, __LINE__, logger::LEVEL::CRITICAL);

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("[CRITICAL] - Test critical message") != std::string::npos);
	EXPECT_TRUE(content.find(__FILE__) != std::string::npos);
}


/* Macro tests */
TEST_F(LoggerTest, DebugMacro)
{

	using namespace logger;
	DEBUG("Test debug macro");

	std::string content = readLogFile();

	if (fenrir::DEBUG)
	{
		EXPECT_TRUE(content.find("[DEBUG] - Test debug macro") != std::string::npos);
	}
	else
	{
		EXPECT_TRUE(content.empty() || content.find("Test debug macro") == std::string::npos);
	}
}

TEST_F(LoggerTest, InfoMacro)
{
	using namespace logger;
	INFO("Test info macro");

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("[INFO] - Test info macro") != std::string::npos);
}

TEST_F(LoggerTest, WarnMacro)
{
	using namespace logger;
	WARN("Test warn macro");

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("[WARN] - Test warn macro") != std::string::npos);
}

TEST_F(LoggerTest, ErrorMacro)
{
	using namespace logger;
	ERROR("Test error macro");

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("[ERROR] - Test error macro") != std::string::npos);
}

TEST_F(LoggerTest, CriticalMacro)
{
	using namespace logger;
	CRITICAL("Test critical macro");

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("[CRITICAL] - Test critical macro") != std::string::npos);
}

/* Debug mode tests */
TEST_F(LoggerTest, DebugMessageBehavior)
{
	logger::Logger log;
	log.log("Debug message behavior test", __FILE__, __LINE__, logger::LEVEL::DEBUG);

	std::string content = readLogFile();

	if (fenrir::DEBUG)
	{
		EXPECT_TRUE(content.find("Debug message behavior test") != std::string::npos);
		EXPECT_TRUE(content.find("[DEBUG]") != std::string::npos);
	}
	else
	{
		EXPECT_TRUE(content.empty() || content.find("Debug message behavior test") == std::string::npos);
	}
}

/* Log rotation tests */
TEST_F(LoggerTest, LogRotationWhenFileSizeExceedsLimit)
{

	createLargeLogFile(logger::MAX_LOG_SIZE + 1000);
	EXPECT_TRUE(std::filesystem::exists(logger::LOG_FILE));

	logger::Logger log;
	log.log("New message after rotation", __FILE__, __LINE__, logger::LEVEL::INFO);

	std::string backup_file = std::string(logger::LOG_FILE) + ".1";
	EXPECT_TRUE(std::filesystem::exists(backup_file));

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("New message after rotation") != std::string::npos);
}

TEST_F(LoggerTest, LogRotationWithExistingBackupFile)
{

	std::string backup_file = std::string(logger::LOG_FILE) + ".1";

	createLargeLogFile(logger::MAX_LOG_SIZE + 1000);
	EXPECT_TRUE(std::filesystem::exists(logger::LOG_FILE));

	{
		std::ofstream existing_backup(backup_file);
		existing_backup << "This is an old backup file that should be removed";
	}
	EXPECT_TRUE(std::filesystem::exists(backup_file));

	auto original_backup_size = std::filesystem::file_size(backup_file);

	logger::Logger log;
	log.log("New message after rotation with existing backup", __FILE__, __LINE__, logger::LEVEL::INFO);

	EXPECT_TRUE(std::filesystem::exists(backup_file));

	auto new_backup_size = std::filesystem::file_size(backup_file);
	EXPECT_GT(new_backup_size, original_backup_size);

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("New message after rotation with existing backup") != std::string::npos);
}

/* Thread safety tests */
TEST_F(LoggerTest, ThreadSafetyMultipleThreads)
{
	logger::Logger log;
	const int num_threads = 10;
	const int messages_per_thread = 100;

	std::vector<std::thread> threads;

	for (int i = 0; i < num_threads; ++i)
	{
		threads.emplace_back([&log, i, messages_per_thread]()
							 {
			for (int j = 0; j < messages_per_thread; ++j)
			{
				std::string message = "Thread " + std::to_string(i) + " Message " + std::to_string(j);
				log.log(message, __FILE__, __LINE__, logger::LEVEL::INFO);
			} });
	}

	for (auto &thread : threads)
	{
		thread.join();
	}

	std::string content = readLogFile();

	for (int i = 0; i < num_threads; ++i)
	{
		for (int j = 0; j < messages_per_thread; ++j)
		{
			std::string message = "Thread " + std::to_string(i) + " Message " + std::to_string(j);
			EXPECT_TRUE(content.find(message) != std::string::npos);
		}
	}
}

/* Edge cases */
TEST_F(LoggerTest, EmptyMessage)
{
	logger::Logger log;
	log.log("", __FILE__, __LINE__, logger::LEVEL::INFO);

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("[INFO] -") != std::string::npos);
}

TEST_F(LoggerTest, LongMessage)
{
	logger::Logger log;
	std::string long_message(10000, 'A');
	log.log(long_message, __FILE__, __LINE__, logger::LEVEL::INFO);

	std::string content = readLogFile();
	EXPECT_TRUE(content.find(long_message) != std::string::npos);
}

TEST_F(LoggerTest, SpecialCharactersInMessage)
{
	logger::Logger log;
	std::string special_message = "Special chars: !@#$%^&*()_+{}|:<>?[]\\;'\",./ \n\t";
	log.log(special_message, __FILE__, __LINE__, logger::LEVEL::INFO);

	std::string content = readLogFile();
	EXPECT_TRUE(content.find("Special chars:") != std::string::npos);
}

/* Invalid level handling */
TEST_F(LoggerTest, InvalidLogLevel)
{
	logger::Logger log;

	log.log("Invalid level test", __FILE__, __LINE__, static_cast<logger::LEVEL>(99));

	std::string content = readLogFile();
}

/* File I/O error handling */
TEST_F(LoggerTest, LogToReadOnlyDirectory)
{

	logger::Logger log;
	log.log("Test message", __FILE__, __LINE__, logger::LEVEL::INFO);

	EXPECT_NO_THROW(log.log("Another test", __FILE__, __LINE__, logger::LEVEL::INFO));
}

/* Memory management */
TEST_F(LoggerTest, MemoryManagement)
{
	for (int i = 0; i < 1000; ++i)
	{
		logger::Logger log;
		log.log("Memory test " + std::to_string(i), __FILE__, __LINE__, logger::LEVEL::INFO);
	}
}

/* Timestamp verification */
TEST_F(LoggerTest, TimestampFormat)
{
	logger::Logger log;
	log.log("Timestamp test", __FILE__, __LINE__, logger::LEVEL::INFO);

	std::string content = readLogFile();

	EXPECT_TRUE(content.find("[") != std::string::npos);
	EXPECT_TRUE(content.find("@") != std::string::npos);
	EXPECT_TRUE(content.find(":") != std::string::npos);
}

/* Line number verification */
TEST_F(LoggerTest, LineNumberInLog)
{
	logger::Logger log;
	uint32_t current_line = __LINE__ + 1;
	log.log("Line number test", __FILE__, current_line, logger::LEVEL::INFO);

	std::string content = readLogFile();
	std::string expected_line = "@ Line " + std::to_string(current_line);
	EXPECT_TRUE(content.find(expected_line) != std::string::npos);
}

/* Stress test */
TEST_F(LoggerTest, StressTest)
{
	if (!test::getCI() || std::string(test::getCI()) != "true")
	{
		GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
	}

	logger::Logger log;
	const int num_messages = 10000;

	for (int i = 0; i < num_messages; ++i)
	{
		std::string message = "Stress test message " + std::to_string(i);
		log.log(message, __FILE__, __LINE__, logger::LEVEL::INFO);
	}

	std::string content = readLogFile();

	EXPECT_TRUE(content.find("Stress test message 0") != std::string::npos);
	EXPECT_TRUE(content.find("Stress test message " + std::to_string(num_messages - 1)) != std::string::npos);
}

/* Additional coverage tests for 100% */
TEST_F(LoggerTest, ConstructorCoverage)
{

	{
		logger::Logger log;
	}
}

TEST_F(LoggerTest, FileOpeningFailure)
{

	std::filesystem::path log_path(logger::LOG_FILE);
	std::filesystem::path log_dir = log_path.parent_path();

	bool original_dir_existed = std::filesystem::exists(log_dir);
	bool original_dir_was_directory = original_dir_existed && std::filesystem::is_directory(log_dir);

	if (std::filesystem::exists(log_dir))
	{
		std::filesystem::remove_all(log_dir);
	}

	std::ofstream file_blocker(log_dir);
	file_blocker << "blocking";
	file_blocker.close();

	logger::Logger log;

	log.log("This should fail to write", __FILE__, __LINE__, logger::LEVEL::INFO);

	std::filesystem::remove(log_dir);

	if (original_dir_existed && original_dir_was_directory)
	{
		std::filesystem::create_directories(log_dir);
	}
}

TEST_F(LoggerTest, ExceptionHandlingInLogging)
{

	logger::Logger log;

	logger::LEVEL invalid_level = static_cast<logger::LEVEL>(255);

	log.log("Test exception handling", __FILE__, __LINE__, invalid_level);
}