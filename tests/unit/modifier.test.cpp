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
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "include/core/core.h"
#include "include/modifier/modifier.h"

class ModifierTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
	}

	void TearDown() override
	{
	}

	std::string captureModifierOutput(const color::Modifier &modifier)
	{
		std::ostringstream oss;
		oss << modifier;
		return oss.str();
	}

	bool isValidAnsiSequence(const std::string &output, color::Color expectedColor)
	{
		std::string expected = "\033[" + std::to_string(static_cast<int>(expectedColor)) + "m";
		return output == expected;
	}
};

TEST_F(ModifierTest, ConstructorAndDestructor)
{

	{
		color::Modifier resetModifier(color::Color::RESET);
		EXPECT_NO_THROW({

		});
	}

	{
		color::Modifier redModifier(color::Color::FG_RED);
		EXPECT_NO_THROW({

		});
	}

	SUCCEED();
}

TEST_F(ModifierTest, StreamOperatorReset)
{
	color::Modifier modifier(color::Color::RESET);
	std::string output = captureModifierOutput(modifier);

	EXPECT_TRUE(isValidAnsiSequence(output, color::Color::RESET));
	EXPECT_EQ(output, "\033[0m");
}

TEST_F(ModifierTest, StreamOperatorForegroundColors)
{

	std::vector<color::Color> fgColors = {
		color::Color::FG_BLACK, color::Color::FG_RED, color::Color::FG_GREEN, color::Color::FG_YELLOW,
		color::Color::FG_BLUE, color::Color::FG_MAGENTA, color::Color::FG_CYAN, color::Color::FG_WHITE};

	for (auto colorValue : fgColors)
	{
		color::Modifier modifier(colorValue);
		std::string output = captureModifierOutput(modifier);

		EXPECT_TRUE(isValidAnsiSequence(output, colorValue));

		switch (colorValue)
		{
		case color::Color::FG_BLACK:
			EXPECT_EQ(output, "\033[30m");
			break;
		case color::Color::FG_RED:
			EXPECT_EQ(output, "\033[31m");
			break;
		case color::Color::FG_GREEN:
			EXPECT_EQ(output, "\033[32m");
			break;
		case color::Color::FG_YELLOW:
			EXPECT_EQ(output, "\033[33m");
			break;
		case color::Color::FG_BLUE:
			EXPECT_EQ(output, "\033[34m");
			break;
		case color::Color::FG_MAGENTA:
			EXPECT_EQ(output, "\033[35m");
			break;
		case color::Color::FG_CYAN:
			EXPECT_EQ(output, "\033[36m");
			break;
		case color::Color::FG_WHITE:
			EXPECT_EQ(output, "\033[37m");
			break;
		default:
			break;
		}
	}
}

TEST_F(ModifierTest, StreamOperatorBackgroundColors)
{

	std::vector<color::Color> bgColors = {
		color::Color::BG_BLACK, color::Color::BG_RED, color::Color::BG_GREEN, color::Color::BG_YELLOW,
		color::Color::BG_BLUE, color::Color::BG_MAGENTA, color::Color::BG_CYAN, color::Color::BG_WHITE};

	for (auto colorValue : bgColors)
	{
		color::Modifier modifier(colorValue);
		std::string output = captureModifierOutput(modifier);

		EXPECT_TRUE(isValidAnsiSequence(output, colorValue));

		switch (colorValue)
		{
		case color::Color::BG_BLACK:
			EXPECT_EQ(output, "\033[40m");
			break;
		case color::Color::BG_RED:
			EXPECT_EQ(output, "\033[41m");
			break;
		case color::Color::BG_GREEN:
			EXPECT_EQ(output, "\033[42m");
			break;
		case color::Color::BG_YELLOW:
			EXPECT_EQ(output, "\033[43m");
			break;
		case color::Color::BG_BLUE:
			EXPECT_EQ(output, "\033[44m");
			break;
		case color::Color::BG_MAGENTA:
			EXPECT_EQ(output, "\033[45m");
			break;
		case color::Color::BG_CYAN:
			EXPECT_EQ(output, "\033[46m");
			break;
		case color::Color::BG_WHITE:
			EXPECT_EQ(output, "\033[47m");
			break;
		default:
			break;
		}
	}
}

TEST_F(ModifierTest, MultipleModifiersInSequence)
{
	std::ostringstream oss;

	color::Modifier red(color::Color::FG_RED);
	color::Modifier bgBlue(color::Color::BG_BLUE);
	color::Modifier reset(color::Color::RESET);

	oss << red << "Red text" << bgBlue << " with blue background" << reset << " normal text";

	std::string output = oss.str();

	EXPECT_NE(output.find("\033[31m"), std::string::npos);
	EXPECT_NE(output.find("\033[44m"), std::string::npos);
	EXPECT_NE(output.find("\033[0m"), std::string::npos);
	EXPECT_NE(output.find("Red text"), std::string::npos);
	EXPECT_NE(output.find(" with blue background"), std::string::npos);
	EXPECT_NE(output.find(" normal text"), std::string::npos);
}

TEST_F(ModifierTest, CopySemantics)
{
	color::Modifier original(color::Color::FG_GREEN);

	color::Modifier copy(original);

	std::string originalOutput = captureModifierOutput(original);
	std::string copyOutput = captureModifierOutput(copy);

	EXPECT_EQ(originalOutput, copyOutput);
	EXPECT_EQ(originalOutput, "\033[32m");
}

TEST_F(ModifierTest, DifferentStreamTypes)
{
	color::Modifier modifier(color::Color::FG_YELLOW);

	std::ostringstream oss;
	oss << modifier;
	EXPECT_EQ(oss.str(), "\033[33m");

	std::ostringstream oss2;
	oss2 << modifier << modifier << modifier;
	EXPECT_EQ(oss2.str(), "\033[33m\033[33m\033[33m");
}

TEST_F(ModifierTest, BoundaryValues)
{

	color::Modifier resetModifier(color::Color::RESET);
	std::string resetOutput = captureModifierOutput(resetModifier);
	EXPECT_EQ(resetOutput, "\033[0m");

	color::Modifier whiteModifier(color::Color::FG_WHITE);
	std::string whiteOutput = captureModifierOutput(whiteModifier);
	EXPECT_EQ(whiteOutput, "\033[37m");

	color::Modifier bgWhiteModifier(color::Color::BG_WHITE);
	std::string bgWhiteOutput = captureModifierOutput(bgWhiteModifier);
	EXPECT_EQ(bgWhiteOutput, "\033[47m");
}

TEST_F(ModifierTest, MemoryManagement)
{

	const int numModifiers = 1000;
	std::vector<std::unique_ptr<color::Modifier>> modifiers;

	EXPECT_NO_THROW({
		for (int i = 0; i < numModifiers; ++i)
		{
			color::Color colorValue = static_cast<color::Color>(static_cast<int>(color::Color::FG_RED) + (i % 8));
			modifiers.push_back(std::make_unique<color::Modifier>(colorValue));
		}
	});

	for (const auto &modifier : modifiers)
	{
		std::string output = captureModifierOutput(*modifier);
		EXPECT_FALSE(output.empty());
		EXPECT_EQ(output.substr(0, 2), "\033[");
		EXPECT_EQ(output.back(), 'm');
	}

	modifiers.clear();
	SUCCEED();
}

TEST_F(ModifierTest, OutputFormatConsistency)
{
	std::vector<color::Color> allColors = {
		color::Color::RESET, color::Color::FG_BLACK, color::Color::BG_BLACK, color::Color::FG_RED, color::Color::BG_RED,
		color::Color::FG_GREEN, color::Color::BG_GREEN, color::Color::FG_YELLOW, color::Color::BG_YELLOW,
		color::Color::FG_BLUE, color::Color::BG_BLUE, color::Color::FG_MAGENTA, color::Color::BG_MAGENTA,
		color::Color::FG_CYAN, color::Color::BG_CYAN, color::Color::FG_WHITE, color::Color::BG_WHITE};

	for (auto colorValue : allColors)
	{
		color::Modifier modifier(colorValue);
		std::string output = captureModifierOutput(modifier);

		EXPECT_GE(output.length(), 4);
		EXPECT_EQ(output.substr(0, 2), "\033[");
		EXPECT_EQ(output.back(), 'm');

		std::string numberPart = output.substr(2, output.length() - 3);
		int extractedNumber = std::stoi(numberPart);
		EXPECT_EQ(extractedNumber, static_cast<int>(colorValue));
	}
}

TEST_F(ModifierTest, RealWorldUsagePattern)
{

	std::ostringstream console;

	color::Modifier green(color::Color::FG_GREEN);
	color::Modifier red(color::Color::FG_RED);
	color::Modifier yellow(color::Color::FG_YELLOW);
	color::Modifier reset(color::Color::RESET);

	console << green << "[INFO] " << reset << "Application started successfully" << std::endl;
	console << yellow << "[WARN] " << reset << "Configuration file not found, using defaults" << std::endl;
	console << red << "[ERROR] " << reset << "Failed to connect to database" << std::endl;

	std::string output = console.str();

	EXPECT_NE(output.find("\033[32m[INFO] \033[0m"), std::string::npos);
	EXPECT_NE(output.find("\033[33m[WARN] \033[0m"), std::string::npos);
	EXPECT_NE(output.find("\033[31m[ERROR] \033[0m"), std::string::npos);
	EXPECT_NE(output.find("Application started successfully"), std::string::npos);
	EXPECT_NE(output.find("Configuration file not found"), std::string::npos);
	EXPECT_NE(output.find("Failed to connect to database"), std::string::npos);
}

TEST_F(ModifierTest, PerformanceModifierCreation)
{
	const int numIterations = 10000;
	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < numIterations; ++i)
	{
		color::Modifier modifier(color::Color::FG_RED);
		std::ostringstream oss;
		oss << modifier;
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	EXPECT_LT(duration.count(), 1000);

	std::cout << "Created and used " << numIterations << " modifiers in "
			  << duration.count() << " ms" << std::endl;
}

#ifdef STRESS_TEST_ENABLED
TEST_F(ModifierTest, StressTest)
{
	GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";

	const int numThreads = 4;
	const int operationsPerThread = 10000;
	std::vector<std::thread> threads;

	auto workerFunction = [operationsPerThread]()
	{
		for (int i = 0; i < operationsPerThread; ++i)
		{
			color::Color randomColor = static_cast<color::Color>(
				static_cast<int>(color::Color::FG_RED) + (i % 8));
			color::Modifier modifier(randomColor);

			std::ostringstream oss;
			oss << modifier << "Test " << i;

			std::string output = oss.str();
			if (output.length() < 10)
			{
				throw std::runtime_error("Unexpected output in stress test");
			}
		}
	};

	for (int i = 0; i < numThreads; ++i)
	{
		threads.emplace_back(workerFunction);
	}

	for (auto &thread : threads)
	{
		thread.join();
	}

	SUCCEED();
}
#else
TEST_F(ModifierTest, StressTest)
{
	GTEST_SKIP() << "🚀 Skipping stress test due to environment configuration 🌟";
}
#endif
