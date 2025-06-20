/**
 * @file test_coordinate_system.cpp
 * @brief 坐标系统测试
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "test_framework.hpp"
#include "tinakit/core/types.hpp"
#include "tinakit/internal/coordinate_utils.hpp"
#include "tinakit/core/exceptions.hpp"

using namespace tinakit;
using namespace tinakit::core;
using namespace tinakit::internal::utils;

// ========================================
// Coordinate 基础测试
// ========================================

TEST_CASE(CoordinateSystem, BasicCoordinateCreation) {
    Coordinate coord(1, 1);
    ASSERT_EQ(1u, coord.row);
    ASSERT_EQ(1u, coord.column);
    ASSERT_TRUE(coord.is_valid());
}

TEST_CASE(CoordinateSystem, InvalidCoordinateDetection) {
    Coordinate invalid_coord(0, 1);
    ASSERT_FALSE(invalid_coord.is_valid());
    
    Coordinate another_invalid(1, 0);
    ASSERT_FALSE(another_invalid.is_valid());
}

TEST_CASE(CoordinateSystem, CoordinateComparison) {
    Coordinate coord1(1, 1);
    Coordinate coord2(1, 1);
    Coordinate coord3(2, 1);
    
    ASSERT_TRUE(coord1 == coord2);
    ASSERT_FALSE(coord1 == coord3);
    ASSERT_TRUE(coord1 != coord3);
}

TEST_CASE(CoordinateSystem, CoordinateOrdering) {
    Coordinate coord1(1, 1);
    Coordinate coord2(1, 2);
    Coordinate coord3(2, 1);
    
    ASSERT_TRUE(coord1 < coord2);
    ASSERT_TRUE(coord1 < coord3);
    ASSERT_TRUE(coord2 < coord3);
}

// ========================================
// CoordinateUtils 测试
// ========================================

TEST_CASE(CoordinateUtils, StringToCoordinate) {
    auto coord = CoordinateUtils::string_to_coordinate("A1");
    ASSERT_EQ(1u, coord.row);
    ASSERT_EQ(1u, coord.column);
    
    auto coord2 = CoordinateUtils::string_to_coordinate("Z99");
    ASSERT_EQ(99u, coord2.row);
    ASSERT_EQ(26u, coord2.column);
    
    auto coord3 = CoordinateUtils::string_to_coordinate("AA100");
    ASSERT_EQ(100u, coord3.row);
    ASSERT_EQ(27u, coord3.column);
}

TEST_CASE(CoordinateUtils, CoordinateToString) {
    Coordinate coord1(1, 1);
    ASSERT_EQ("A1", CoordinateUtils::coordinate_to_string(coord1));
    
    Coordinate coord2(99, 26);
    ASSERT_EQ("Z99", CoordinateUtils::coordinate_to_string(coord2));
    
    Coordinate coord3(100, 27);
    ASSERT_EQ("AA100", CoordinateUtils::coordinate_to_string(coord3));
}

TEST_CASE(CoordinateUtils, InvalidStringThrowsException) {
    ASSERT_THROWS(CoordinateUtils::string_to_coordinate(""), InvalidCellAddressException);
    ASSERT_THROWS(CoordinateUtils::string_to_coordinate("A"), InvalidCellAddressException);
    ASSERT_THROWS(CoordinateUtils::string_to_coordinate("1"), InvalidCellAddressException);
    ASSERT_THROWS(CoordinateUtils::string_to_coordinate("A0"), InvalidCellAddressException);
}

TEST_CASE(CoordinateUtils, InvalidCoordinateThrowsException) {
    Coordinate invalid_coord(0, 1);
    ASSERT_THROWS(CoordinateUtils::coordinate_to_string(invalid_coord), InvalidCellAddressException);
}

// ========================================
// 列号转换测试
// ========================================

TEST_CASE(ColumnConversion, LettersToNumber) {
    ASSERT_EQ(1u, CoordinateUtils::column_letters_to_number("A"));
    ASSERT_EQ(26u, CoordinateUtils::column_letters_to_number("Z"));
    ASSERT_EQ(27u, CoordinateUtils::column_letters_to_number("AA"));
    ASSERT_EQ(52u, CoordinateUtils::column_letters_to_number("AZ"));
    ASSERT_EQ(53u, CoordinateUtils::column_letters_to_number("BA"));
}

TEST_CASE(ColumnConversion, NumberToLetters) {
    ASSERT_EQ("A", CoordinateUtils::column_number_to_letters(1));
    ASSERT_EQ("Z", CoordinateUtils::column_number_to_letters(26));
    ASSERT_EQ("AA", CoordinateUtils::column_number_to_letters(27));
    ASSERT_EQ("AZ", CoordinateUtils::column_number_to_letters(52));
    ASSERT_EQ("BA", CoordinateUtils::column_number_to_letters(53));
}

TEST_CASE(ColumnConversion, InvalidInputThrowsException) {
    ASSERT_THROWS(CoordinateUtils::column_letters_to_number(""), InvalidCellAddressException);
    ASSERT_THROWS(CoordinateUtils::column_letters_to_number("1"), InvalidCellAddressException);
    ASSERT_THROWS(CoordinateUtils::column_number_to_letters(0), InvalidCellAddressException);
}

// ========================================
// 范围地址测试
// ========================================

TEST_CASE(RangeAddress, BasicRangeCreation) {
    Coordinate start(1, 1);
    Coordinate end(5, 3);
    range_address range(start, end);
    
    ASSERT_EQ(start, range.start);
    ASSERT_EQ(end, range.end);
}

TEST_CASE(RangeAddress, StringToRangeAddress) {
    auto range = CoordinateUtils::string_to_range_address("A1:C5");
    ASSERT_EQ(1u, range.start.row);
    ASSERT_EQ(1u, range.start.column);
    ASSERT_EQ(5u, range.end.row);
    ASSERT_EQ(3u, range.end.column);
}

TEST_CASE(RangeAddress, SingleCellRange) {
    auto range = CoordinateUtils::string_to_range_address("B2");
    ASSERT_EQ(2u, range.start.row);
    ASSERT_EQ(2u, range.start.column);
    ASSERT_EQ(2u, range.end.row);
    ASSERT_EQ(2u, range.end.column);
}

TEST_CASE(RangeAddress, RangeAddressToString) {
    Coordinate start(1, 1);
    Coordinate end(5, 3);
    range_address range(start, end);
    
    ASSERT_EQ("A1:C5", CoordinateUtils::range_address_to_string(range));
}

TEST_CASE(RangeAddress, SingleCellRangeToString) {
    Coordinate pos(2, 2);
    range_address range(pos, pos);
    
    ASSERT_EQ("B2", CoordinateUtils::range_address_to_string(range));
}

TEST_CASE(RangeAddress, ContainsTest) {
    Coordinate start(2, 2);
    Coordinate end(5, 5);
    range_address range(start, end);
    
    ASSERT_TRUE(range.contains(Coordinate(3, 3)));
    ASSERT_TRUE(range.contains(Coordinate(2, 2)));
    ASSERT_TRUE(range.contains(Coordinate(5, 5)));
    ASSERT_FALSE(range.contains(Coordinate(1, 1)));
    ASSERT_FALSE(range.contains(Coordinate(6, 6)));
}

// ========================================
// 验证函数测试
// ========================================

TEST_CASE(Validation, ValidCoordinateStrings) {
    ASSERT_TRUE(CoordinateUtils::is_valid_coordinate_string("A1"));
    ASSERT_TRUE(CoordinateUtils::is_valid_coordinate_string("Z99"));
    ASSERT_TRUE(CoordinateUtils::is_valid_coordinate_string("AA100"));
}

TEST_CASE(Validation, InvalidCoordinateStrings) {
    ASSERT_FALSE(CoordinateUtils::is_valid_coordinate_string(""));
    ASSERT_FALSE(CoordinateUtils::is_valid_coordinate_string("A"));
    ASSERT_FALSE(CoordinateUtils::is_valid_coordinate_string("1"));
    ASSERT_FALSE(CoordinateUtils::is_valid_coordinate_string("A0"));
}

TEST_CASE(Validation, ValidRangeStrings) {
    ASSERT_TRUE(CoordinateUtils::is_valid_range_string("A1:C5"));
    ASSERT_TRUE(CoordinateUtils::is_valid_range_string("B2"));
    ASSERT_TRUE(CoordinateUtils::is_valid_range_string("AA1:ZZ100"));
}

TEST_CASE(Validation, InvalidRangeStrings) {
    ASSERT_FALSE(CoordinateUtils::is_valid_range_string(""));
    ASSERT_FALSE(CoordinateUtils::is_valid_range_string("A:"));
    ASSERT_FALSE(CoordinateUtils::is_valid_range_string(":B2"));
}
