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
#include <ctime>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include "include/core/core.h"
#include "include/chrono/chrono.h"

class ChronoTest : public ::testing::Test
{
protected:
	void SetUp() override
	{

		chrono_obj = std::make_unique<chrono::Chrono>();
	}

	void TearDown() override
	{

		chrono_obj.reset();
	}

	std::unique_ptr<chrono::Chrono> chrono_obj;

	bool isTimeWithinRange(time_t time1, time_t time2, int tolerance_seconds = 2)
	{
		return std::abs(static_cast<long>(time1 - time2)) <= tolerance_seconds;
	}

	bool isValidTm(const tm *time_struct)
	{
		if (!time_struct)
			return false;

		return (time_struct->tm_sec >= 0 && time_struct->tm_sec <= 60) &&
			   (time_struct->tm_min >= 0 && time_struct->tm_min <= 59) &&
			   (time_struct->tm_hour >= 0 && time_struct->tm_hour <= 23) &&
			   (time_struct->tm_mday >= 1 && time_struct->tm_mday <= 31) &&
			   (time_struct->tm_mon >= 0 && time_struct->tm_mon <= 11) &&
			   (time_struct->tm_year >= 0) &&
			   (time_struct->tm_wday >= 0 && time_struct->tm_wday <= 6) &&
			   (time_struct->tm_yday >= 0 && time_struct->tm_yday <= 365);
	}
};

/* Basic functionality tests */
TEST_F(ChronoTest, ConstructorAndDestructor)
{
	chrono::Chrono *chrono_ptr = new chrono::Chrono();
	EXPECT_NE(chrono_ptr, nullptr);
	delete chrono_ptr;
}

TEST_F(ChronoTest, GetRawTime)
{
	time_t raw_time = chrono_obj->getRawTime();
	time_t system_time = std::time(nullptr);

	EXPECT_TRUE(isTimeWithinRange(raw_time, system_time));
	EXPECT_GT(raw_time, 0);
}

TEST_F(ChronoTest, GetLocalTimeWithNullTimer)
{
	tm *local_time = chrono_obj->getLocalTime(nullptr);

	EXPECT_NE(local_time, nullptr);
	EXPECT_TRUE(isValidTm(local_time));

	time_t current_time = std::time(nullptr);
	tm *expected_time = std::localtime(&current_time);

	EXPECT_EQ(local_time->tm_year, expected_time->tm_year);
	EXPECT_EQ(local_time->tm_mon, expected_time->tm_mon);
	EXPECT_EQ(local_time->tm_mday, expected_time->tm_mday);
	EXPECT_EQ(local_time->tm_hour, expected_time->tm_hour);
	EXPECT_EQ(local_time->tm_min, expected_time->tm_min);

	EXPECT_TRUE(std::abs(local_time->tm_sec - expected_time->tm_sec) <= 2);
}

TEST_F(ChronoTest, GetLocalTimeWithSpecificTimer)
{

	time_t epoch_time = 0;
	tm *local_time = chrono_obj->getLocalTime(&epoch_time);

	EXPECT_NE(local_time, nullptr);
	EXPECT_TRUE(isValidTm(local_time));

	EXPECT_EQ(local_time->tm_year, 70);
}

TEST_F(ChronoTest, GetLocalTimeWithFutureTime)
{

	time_t future_time = 1893456000;
	tm *local_time = chrono_obj->getLocalTime(&future_time);

	EXPECT_NE(local_time, nullptr);
	EXPECT_TRUE(isValidTm(local_time));
	EXPECT_EQ(local_time->tm_year, 130);
}

TEST_F(ChronoTest, GetTimeWithFormatBasic)
{
	const char *format = "%Y-%m-%d %H:%M:%S";
	auto formatted_time = chrono_obj->getTimeWithFormat(format);

	std::ostringstream oss;
	oss << formatted_time;
	std::string time_str = oss.str();

	EXPECT_FALSE(time_str.empty());

	EXPECT_TRUE(time_str.find("-") != std::string::npos);
	EXPECT_TRUE(time_str.find(":") != std::string::npos);
	EXPECT_TRUE(time_str.find(" ") != std::string::npos);

	EXPECT_EQ(time_str.length(), 19);
}

TEST_F(ChronoTest, GetTimeWithFormatISO8601)
{
	const char *format = "%Y-%m-%dT%H:%M:%S";
	auto formatted_time = chrono_obj->getTimeWithFormat(format);

	std::ostringstream oss;
	oss << formatted_time;
	std::string time_str = oss.str();

	EXPECT_FALSE(time_str.empty());
	EXPECT_TRUE(time_str.find("T") != std::string::npos);
	EXPECT_EQ(time_str.length(), 19);
}

TEST_F(ChronoTest, GetTimeWithFormatShort)
{
	const char *format = "%H:%M";
	auto formatted_time = chrono_obj->getTimeWithFormat(format);

	std::ostringstream oss;
	oss << formatted_time;
	std::string time_str = oss.str();

	EXPECT_FALSE(time_str.empty());
	EXPECT_TRUE(time_str.find(":") != std::string::npos);
	EXPECT_EQ(time_str.length(), 5);
}

TEST_F(ChronoTest, GetTimeWithFormatDate)
{
	const char *format = "%Y-%m-%d";
	auto formatted_time = chrono_obj->getTimeWithFormat(format);

	std::ostringstream oss;
	oss << formatted_time;
	std::string time_str = oss.str();

	EXPECT_FALSE(time_str.empty());
	EXPECT_TRUE(time_str.find("-") != std::string::npos);
	EXPECT_EQ(time_str.length(), 10);
}

TEST_F(ChronoTest, GetTimeWithFormatCustom)
{
	const char *format = "Today is %A, %B %d, %Y";
	auto formatted_time = chrono_obj->getTimeWithFormat(format);

	std::ostringstream oss;
	oss << formatted_time;
	std::string time_str = oss.str();

	EXPECT_FALSE(time_str.empty());
	EXPECT_TRUE(time_str.find("Today is") != std::string::npos);
	EXPECT_TRUE(time_str.find(",") != std::string::npos);
}

/* Edge cases */
TEST_F(ChronoTest, EmptyFormat)
{
	const char *format = "";
	auto formatted_time = chrono_obj->getTimeWithFormat(format);

	std::ostringstream oss;
	oss << formatted_time;
	std::string time_str = oss.str();

	EXPECT_TRUE(time_str.empty());
}

TEST_F(ChronoTest, InvalidFormat)
{
	const char *format = "%Z";
	auto formatted_time = chrono_obj->getTimeWithFormat(format);

	std::ostringstream oss;
	oss << formatted_time;
	std::string time_str = oss.str();

	EXPECT_NO_THROW(oss << formatted_time);
}

/* Consistency tests */
TEST_F(ChronoTest, ConsistencyBetweenRawTimeAndLocalTime)
{
	time_t raw_time = chrono_obj->getRawTime();
	tm *local_time = chrono_obj->getLocalTime(&raw_time);

	EXPECT_NE(local_time, nullptr);
	EXPECT_TRUE(isValidTm(local_time));

	time_t converted_time = std::mktime(local_time);
	EXPECT_TRUE(isTimeWithinRange(raw_time, converted_time));
}

TEST_F(ChronoTest, MultipleCallsConsistency)
{
	time_t time1 = chrono_obj->getRawTime();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	time_t time2 = chrono_obj->getRawTime();

	EXPECT_GE(time2, time1);
	EXPECT_TRUE(isTimeWithinRange(time1, time2, 1));
}

/* Thread safety tests */
TEST_F(ChronoTest, ThreadSafetyMultipleThreads)
{
	const int num_threads = 10;
	const int calls_per_thread = 100;
	std::vector<std::thread> threads;
	std::vector<std::vector<time_t>> results(num_threads);

	for (int i = 0; i < num_threads; ++i)
	{
		threads.emplace_back([this, i, calls_per_thread, &results]()
							 {
			for (int j = 0; j < calls_per_thread; ++j)
			{
				time_t raw_time = chrono_obj->getRawTime();
				tm* local_time = chrono_obj->getLocalTime(nullptr);
				
				EXPECT_GT(raw_time, 0);
				EXPECT_NE(local_time, nullptr);
				EXPECT_TRUE(isValidTm(local_time));
				
				results[i].push_back(raw_time);
			} });
	}

	for (auto &thread : threads)
	{
		thread.join();
	}

	for (int i = 0; i < num_threads; ++i)
	{
		EXPECT_EQ(results[i].size(), static_cast<size_t>(calls_per_thread));

		time_t current_time = std::time(nullptr);
		for (time_t recorded_time : results[i])
		{
			EXPECT_TRUE(isTimeWithinRange(recorded_time, current_time, 60));
		}
	}
}

/* Performance tests */
TEST_F(ChronoTest, PerformanceGetRawTime)
{
	const int num_calls = 10000;
	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < num_calls; ++i)
	{
		time_t raw_time = chrono_obj->getRawTime();
		EXPECT_GT(raw_time, 0);
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	EXPECT_LT(duration.count(), 1000);
}

TEST_F(ChronoTest, PerformanceGetLocalTime)
{
	const int num_calls = 1000;
	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < num_calls; ++i)
	{
		tm *local_time = chrono_obj->getLocalTime(nullptr);
		EXPECT_NE(local_time, nullptr);
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	EXPECT_LT(duration.count(), 1000);
}

/* Memory management */
TEST_F(ChronoTest, MemoryManagement)
{

	for (int i = 0; i < 1000; ++i)
	{
		chrono::Chrono temp_chrono;
		time_t raw_time = temp_chrono.getRawTime();
		tm *local_time = temp_chrono.getLocalTime(nullptr);

		EXPECT_GT(raw_time, 0);
		EXPECT_NE(local_time, nullptr);
	}
}

/* Boundary value tests */
TEST_F(ChronoTest, BoundaryValuesMinTime)
{

	time_t min_time = 0;
	tm *local_time = chrono_obj->getLocalTime(&min_time);

	EXPECT_NE(local_time, nullptr);
	EXPECT_TRUE(isValidTm(local_time));
	EXPECT_EQ(local_time->tm_year, 70);
}

TEST_F(ChronoTest, BoundaryValuesNegativeTime)
{

	time_t negative_time = -86400;
	tm *local_time = chrono_obj->getLocalTime(&negative_time);

	EXPECT_NO_THROW(chrono_obj->getLocalTime(&negative_time));
}

/* Format string validation */
TEST_F(ChronoTest, FormatStringValidation)
{
	std::vector<const char *> valid_formats = {
		"%Y-%m-%d",
		"%H:%M:%S",
		"%Y-%m-%d %H:%M:%S",
		"%a %b %d %H:%M:%S %Y",
		"%c",
		"%x %X"};

	for (const char *format : valid_formats)
	{
		EXPECT_NO_THROW({
			auto formatted_time = chrono_obj->getTimeWithFormat(format);
			std::ostringstream oss;
			oss << formatted_time;
			std::string result = oss.str();
			EXPECT_FALSE(result.empty());
		});
	}
}

/* Stress test */
TEST_F(ChronoTest, StressTest)
{
	if (!test::CI || std::string(test::CI) != "true")
	{
		GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
	}

	const int num_iterations = 100000;
	const char *format = "%Y-%m-%d %H:%M:%S";

	for (int i = 0; i < num_iterations; ++i)
	{
		time_t raw_time = chrono_obj->getRawTime();
		tm *local_time = chrono_obj->getLocalTime(nullptr);
		auto formatted_time = chrono_obj->getTimeWithFormat(format);

		EXPECT_GT(raw_time, 0);
		EXPECT_NE(local_time, nullptr);
		EXPECT_TRUE(isValidTm(local_time));

		std::ostringstream oss;
		oss << formatted_time;
		EXPECT_FALSE(oss.str().empty());
	}
}

/* Real-world usage patterns */
TEST_F(ChronoTest, RealWorldUsagePattern)
{

	time_t start_time = chrono_obj->getRawTime();

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	time_t end_time = chrono_obj->getRawTime();

	tm *start_tm = chrono_obj->getLocalTime(&start_time);
	tm *end_tm = chrono_obj->getLocalTime(&end_time);

	EXPECT_NE(start_tm, nullptr);
	EXPECT_NE(end_tm, nullptr);
	EXPECT_TRUE(isValidTm(start_tm));
	EXPECT_TRUE(isValidTm(end_tm));

	EXPECT_GE(end_time, start_time);

	const char *log_format = "%Y-%m-%d %H:%M:%S";
	auto start_formatted = chrono_obj->getTimeWithFormat(log_format);
	auto end_formatted = chrono_obj->getTimeWithFormat(log_format);

	std::ostringstream start_oss, end_oss;
	start_oss << start_formatted;
	end_oss << end_formatted;

	EXPECT_FALSE(start_oss.str().empty());
	EXPECT_FALSE(end_oss.str().empty());
}
