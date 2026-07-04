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
#include "include/utils/utils.h"

class UtilsTest : public ::testing::Test
{
};

TEST(UtilsTest, GetAlgebraicNotation_ValidInput)
{
	EXPECT_EQ(utils::get_algebraic_notation(0, 0), "a1");
	EXPECT_EQ(utils::get_algebraic_notation(7, 7), "h8");
	EXPECT_EQ(utils::get_algebraic_notation(3, 4), "e4");
}

TEST(UtilsTest, GetAlgebraicNotation_InvalidInput)
{
	EXPECT_THROW(utils::get_algebraic_notation(8, 8), std::runtime_error);
}

TEST(UtilsTest, LogThrowError_NoThrow)
{
	EXPECT_NO_THROW(LOG_THROW_ERROR("This is a test error", false));
}

TEST(UtilsTest, LogThrowError_WithThrow)
{
	EXPECT_THROW(LOG_THROW_ERROR("This is a test error", true), std::runtime_error);
}

TEST(UtilsTest, LogThrowError_NullError)
{
	EXPECT_THROW(LOG_THROW_ERROR("", true), std::runtime_error);
}

TEST(UtilsTest, LogThrowError_EmptyError)
{
	EXPECT_THROW(LOG_THROW_ERROR("", true), std::runtime_error);
}

TEST(UtilsTest, LogThrowError_EmptyErrorNoThrow)
{
	EXPECT_NO_THROW(LOG_THROW_ERROR("", false));
}
TEST(UtilsTest, ParseAlgebraicNotation_ValidInput)
{
	uint8_t rank, file;
	utils::parse_algebraic_notation("e4", rank, file);
	EXPECT_EQ(rank, 3);
	EXPECT_EQ(file, 4);
}

TEST(UtilsTest, ParseAlgebraicNotation_InvalidInput)
{
	uint8_t rank, file;
	EXPECT_THROW(utils::parse_algebraic_notation("e9", rank, file), std::runtime_error);
	EXPECT_THROW(utils::parse_algebraic_notation("i4", rank, file), std::runtime_error);
	EXPECT_THROW(utils::parse_algebraic_notation("e", rank, file), std::runtime_error);
	EXPECT_THROW(utils::parse_algebraic_notation("4", rank, file), std::runtime_error);
	EXPECT_THROW(utils::parse_algebraic_notation("", rank, file), std::runtime_error);
	EXPECT_THROW(utils::parse_algebraic_notation("e4e4", rank, file), std::runtime_error);
	EXPECT_THROW(utils::parse_algebraic_notation("e4e", rank, file), std::runtime_error);
	EXPECT_THROW(utils::parse_algebraic_notation("e-1", rank, file), std::runtime_error);
	EXPECT_THROW(utils::parse_algebraic_notation("e+4", rank, file), std::runtime_error);
}

TEST(UtilsTest, ChessPieceCountRules_TooManyQueens)
{
	std::unordered_map<char, uint8_t> piece_counts = {
		{'K', 1}, {'Q', 10}, {'R', 2}, {'B', 2}, {'N', 2}, {'P', 8}, {'k', 1}, {'q', 1}, {'r', 2}, {'b', 2}, {'n', 2}, {'p', 8}};
	EXPECT_FALSE(utils::are_chess_piece_count_rules_valid(piece_counts));
}

TEST(UtilsTest, ChessPieceCountRules_TooManyRooks)
{
	std::unordered_map<char, uint8_t> piece_counts = {
		{'K', 1}, {'Q', 1}, {'R', 11}, {'B', 2}, {'N', 2}, {'P', 8}, {'k', 1}, {'q', 1}, {'r', 2}, {'b', 2}, {'n', 2}, {'p', 8}};
	EXPECT_FALSE(utils::are_chess_piece_count_rules_valid(piece_counts));
}

TEST(UtilsTest, ChessPieceCountRules_TooManyBishops)
{
	std::unordered_map<char, uint8_t> piece_counts = {
		{'K', 1}, {'Q', 1}, {'R', 2}, {'B', 11}, {'N', 2}, {'P', 8}, {'k', 1}, {'q', 1}, {'r', 2}, {'b', 2}, {'n', 2}, {'p', 8}};
	EXPECT_FALSE(utils::are_chess_piece_count_rules_valid(piece_counts));
}

TEST(UtilsTest, ChessPieceCountRules_TooManyKnights)
{
	std::unordered_map<char, uint8_t> piece_counts = {
		{'K', 1}, {'Q', 1}, {'R', 2}, {'B', 2}, {'N', 11}, {'P', 8}, {'k', 1}, {'q', 1}, {'r', 2}, {'b', 2}, {'n', 2}, {'p', 8}};
	EXPECT_FALSE(utils::are_chess_piece_count_rules_valid(piece_counts));
}

TEST(UtilsTest, ChessPieceCountRules_WhitePromotionExceedsLostPawns)
{
	std::unordered_map<char, uint8_t> piece_counts = {
		{'K', 1}, {'Q', 9}, {'R', 2}, {'B', 2}, {'N', 2}, {'P', 1}, {'k', 1}, {'q', 1}, {'r', 2}, {'b', 2}, {'n', 2}, {'p', 8}};
	EXPECT_FALSE(utils::are_chess_piece_count_rules_valid(piece_counts));
}

TEST(UtilsTest, ChessPieceCountRules_BlackPromotionExceedsLostPawns)
{
	std::unordered_map<char, uint8_t> piece_counts = {
		{'K', 1}, {'Q', 1}, {'R', 2}, {'B', 2}, {'N', 2}, {'P', 8}, {'k', 1}, {'q', 9}, {'r', 2}, {'b', 2}, {'n', 2}, {'p', 1}};
	EXPECT_FALSE(utils::are_chess_piece_count_rules_valid(piece_counts));
}

TEST(UtilsTest, ChessPieceCountRules_ValidCounts)
{
	std::unordered_map<char, uint8_t> piece_counts = {
		{'K', 1}, {'Q', 1}, {'R', 2}, {'B', 2}, {'N', 2}, {'P', 8}, {'k', 1}, {'q', 1}, {'r', 2}, {'b', 2}, {'n', 2}, {'p', 8}};
	EXPECT_TRUE(utils::are_chess_piece_count_rules_valid(piece_counts));
}
