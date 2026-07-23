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

    std::string captureModifierOutput(const modifier::Modifier &modifier)
    {
        std::ostringstream oss;
        oss << modifier;
        return oss.str();
    }

    bool isValidAnsiSequence(const std::string &output, modifier::Color expectedColor)
    {
        std::string expected = "\033[" + std::to_string(static_cast<int>(expectedColor)) + "m";
        return output == expected;
    }
};

TEST_F(ModifierTest, ConstructorAndDestructor)
{

    {
        modifier::Modifier resetModifier(modifier::Color::RESET);
        EXPECT_NO_THROW({

        });
    }

    {
        modifier::Modifier redModifier(modifier::Color::FG_RED);
        EXPECT_NO_THROW({

        });
    }

    SUCCEED();
}

TEST_F(ModifierTest, StreamOperatorReset)
{
    modifier::Modifier modifier(modifier::Color::RESET);
    std::string output = captureModifierOutput(modifier);

    EXPECT_TRUE(isValidAnsiSequence(output, modifier::Color::RESET));
    EXPECT_EQ(output, "\033[0m");
}

TEST_F(ModifierTest, StreamOperatorForegroundColors)
{

    std::vector<modifier::Color> fgColors = {
        modifier::Color::FG_BLACK, modifier::Color::FG_RED, modifier::Color::FG_GREEN, modifier::Color::FG_YELLOW,
        modifier::Color::FG_BLUE, modifier::Color::FG_MAGENTA, modifier::Color::FG_CYAN, modifier::Color::FG_WHITE};

    for (auto color_value : fgColors)
    {
        modifier::Modifier modifier(color_value);
        std::string output = captureModifierOutput(modifier);

        EXPECT_TRUE(isValidAnsiSequence(output, color_value));

        switch (color_value)
        {
        case modifier::Color::FG_BLACK:
            EXPECT_EQ(output, "\033[30m");
            break;
        case modifier::Color::FG_RED:
            EXPECT_EQ(output, "\033[31m");
            break;
        case modifier::Color::FG_GREEN:
            EXPECT_EQ(output, "\033[32m");
            break;
        case modifier::Color::FG_YELLOW:
            EXPECT_EQ(output, "\033[33m");
            break;
        case modifier::Color::FG_BLUE:
            EXPECT_EQ(output, "\033[34m");
            break;
        case modifier::Color::FG_MAGENTA:
            EXPECT_EQ(output, "\033[35m");
            break;
        case modifier::Color::FG_CYAN:
            EXPECT_EQ(output, "\033[36m");
            break;
        case modifier::Color::FG_WHITE:
            EXPECT_EQ(output, "\033[37m");
            break;
        default:
            break;
        }
    }
}

TEST_F(ModifierTest, StreamOperatorBackgroundColors)
{

    std::vector<modifier::Color> bgColors = {
        modifier::Color::BG_BLACK, modifier::Color::BG_RED, modifier::Color::BG_GREEN, modifier::Color::BG_YELLOW,
        modifier::Color::BG_BLUE, modifier::Color::BG_MAGENTA, modifier::Color::BG_CYAN, modifier::Color::BG_WHITE};

    for (auto color_value : bgColors)
    {
        modifier::Modifier modifier(color_value);
        std::string output = captureModifierOutput(modifier);

        EXPECT_TRUE(isValidAnsiSequence(output, color_value));

        switch (color_value)
        {
        case modifier::Color::BG_BLACK:
            EXPECT_EQ(output, "\033[40m");
            break;
        case modifier::Color::BG_RED:
            EXPECT_EQ(output, "\033[41m");
            break;
        case modifier::Color::BG_GREEN:
            EXPECT_EQ(output, "\033[42m");
            break;
        case modifier::Color::BG_YELLOW:
            EXPECT_EQ(output, "\033[43m");
            break;
        case modifier::Color::BG_BLUE:
            EXPECT_EQ(output, "\033[44m");
            break;
        case modifier::Color::BG_MAGENTA:
            EXPECT_EQ(output, "\033[45m");
            break;
        case modifier::Color::BG_CYAN:
            EXPECT_EQ(output, "\033[46m");
            break;
        case modifier::Color::BG_WHITE:
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

    modifier::Modifier red(modifier::Color::FG_RED);
    modifier::Modifier bgBlue(modifier::Color::BG_BLUE);
    modifier::Modifier reset(modifier::Color::RESET);

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
    modifier::Modifier original(modifier::Color::FG_GREEN);

    modifier::Modifier copy(original);

    std::string originalOutput = captureModifierOutput(original);
    std::string copyOutput = captureModifierOutput(copy);

    EXPECT_EQ(originalOutput, copyOutput);
    EXPECT_EQ(originalOutput, "\033[32m");
}

TEST_F(ModifierTest, DifferentStreamTypes)
{
    modifier::Modifier modifier(modifier::Color::FG_YELLOW);

    std::ostringstream oss;
    oss << modifier;
    EXPECT_EQ(oss.str(), "\033[33m");

    std::ostringstream oss2;
    oss2 << modifier << modifier << modifier;
    EXPECT_EQ(oss2.str(), "\033[33m\033[33m\033[33m");
}

TEST_F(ModifierTest, BoundaryValues)
{

    modifier::Modifier resetModifier(modifier::Color::RESET);
    std::string resetOutput = captureModifierOutput(resetModifier);
    EXPECT_EQ(resetOutput, "\033[0m");

    modifier::Modifier whiteModifier(modifier::Color::FG_WHITE);
    std::string whiteOutput = captureModifierOutput(whiteModifier);
    EXPECT_EQ(whiteOutput, "\033[37m");

    modifier::Modifier bgWhiteModifier(modifier::Color::BG_WHITE);
    std::string bgWhiteOutput = captureModifierOutput(bgWhiteModifier);
    EXPECT_EQ(bgWhiteOutput, "\033[47m");
}

TEST_F(ModifierTest, MemoryManagement)
{

    const int numModifiers = 1000;
    std::vector<std::unique_ptr<modifier::Modifier>> modifiers;

    EXPECT_NO_THROW({
        for (int i = 0; i < numModifiers; ++i)
        {
            modifier::Color color_value = static_cast<modifier::Color>(static_cast<int>(modifier::Color::FG_RED) + (i % 8));
            modifiers.push_back(std::make_unique<modifier::Modifier>(color_value));
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
    std::vector<modifier::Color> allColors = {
        modifier::Color::RESET, modifier::Color::FG_BLACK, modifier::Color::BG_BLACK, modifier::Color::FG_RED, modifier::Color::BG_RED,
        modifier::Color::FG_GREEN, modifier::Color::BG_GREEN, modifier::Color::FG_YELLOW, modifier::Color::BG_YELLOW,
        modifier::Color::FG_BLUE, modifier::Color::BG_BLUE, modifier::Color::FG_MAGENTA, modifier::Color::BG_MAGENTA,
        modifier::Color::FG_CYAN, modifier::Color::BG_CYAN, modifier::Color::FG_WHITE, modifier::Color::BG_WHITE};

    for (auto color_value : allColors)
    {
        modifier::Modifier modifier(color_value);
        std::string output = captureModifierOutput(modifier);

        EXPECT_GE(output.length(), 4);
        EXPECT_EQ(output.substr(0, 2), "\033[");
        EXPECT_EQ(output.back(), 'm');

        std::string numberPart = output.substr(2, output.length() - 3);
        int extractedNumber = std::stoi(numberPart);
        EXPECT_EQ(extractedNumber, static_cast<int>(color_value));
    }
}

TEST_F(ModifierTest, RealWorldUsagePattern)
{

    std::ostringstream console;

    modifier::Modifier green(modifier::Color::FG_GREEN);
    modifier::Modifier red(modifier::Color::FG_RED);
    modifier::Modifier yellow(modifier::Color::FG_YELLOW);
    modifier::Modifier reset(modifier::Color::RESET);

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
        modifier::Modifier modifier(modifier::Color::FG_RED);
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

    const int num_threads = 4;
    const int operationsPerThread = 10000;
    std::vector<std::thread> threads;

    auto workerFunction = [operationsPerThread]()
    {
        for (int i = 0; i < operationsPerThread; ++i)
        {
            modifier::Color randomColor = static_cast<modifier::Color>(
                static_cast<int>(modifier::Color::FG_RED) + (i % 8));
            modifier::Modifier modifier(randomColor);

            std::ostringstream oss;
            oss << modifier << "Test " << i;

            std::string output = oss.str();
            if (output.length() < 10)
            {
                throw std::runtime_error("Unexpected output in stress test");
            }
        }
    };

    for (int i = 0; i < num_threads; ++i)
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
