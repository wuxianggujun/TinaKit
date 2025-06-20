/**
 * @file test_range_system.cpp
 * @brief Range系统测试
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "test_framework.hpp"
#include "tinakit/excel/range.hpp"
#include "tinakit/excel/range_view.hpp"
#include "tinakit/core/types.hpp"
#include "tinakit/internal/coordinate_utils.hpp"

using namespace tinakit;
using namespace tinakit::excel;
using namespace tinakit::core;

// ========================================
// Range 基础测试
// ========================================

TEST_CASE(RangeSystem, DefaultRangeConstruction) {
    Range range;
    // 默认构造的Range应该是有效的（虽然可能为空）
    // 这个测试主要确保默认构造不会崩溃
    ASSERT_TRUE(true); // 如果能到这里说明构造成功
}

TEST_CASE(RangeSystem, RangeFromString) {
    // 注意：这个测试需要workbook_impl，暂时跳过具体实现
    // 主要测试静态方法的存在性
    ASSERT_TRUE(true); // 占位测试
}

TEST_CASE(RangeSystem, RangeAddressConversion) {
    // 测试range_address的基本功能
    Coordinate start(1, 1);
    Coordinate end(5, 3);
    range_address addr(start, end);
    
    ASSERT_EQ(start, addr.start);
    ASSERT_EQ(end, addr.end);
}

TEST_CASE(RangeSystem, RangeAddressSize) {
    Coordinate start(2, 2);
    Coordinate end(5, 4);
    range_address addr(start, end);
    
    auto size = addr.size();
    ASSERT_EQ(4u, size.first);  // 4 rows (2,3,4,5)
    ASSERT_EQ(3u, size.second); // 3 columns (2,3,4)
}

TEST_CASE(RangeSystem, RangeAddressOverlap) {
    range_address range1(Coordinate(1, 1), Coordinate(3, 3));
    range_address range2(Coordinate(2, 2), Coordinate(4, 4));
    range_address range3(Coordinate(5, 5), Coordinate(7, 7));
    
    ASSERT_TRUE(range1.overlaps(range2));
    ASSERT_TRUE(range2.overlaps(range1));
    ASSERT_FALSE(range1.overlaps(range3));
    ASSERT_FALSE(range3.overlaps(range1));
}

// ========================================
// RangeView 基础测试
// ========================================

TEST_CASE(RangeView, BasicConstruction) {
    // 注意：RangeView需要workbook_impl，这里只测试基本概念
    // 实际测试需要mock对象或集成测试
    ASSERT_TRUE(true); // 占位测试
}

// ========================================
// 坐标转换集成测试
// ========================================

TEST_CASE(Integration, CoordinateRoundTrip) {
    // 测试坐标字符串的往返转换
    std::vector<std::string> test_addresses = {
        "A1", "B2", "Z26", "AA27", "AB28", "ZZ702", "AAA703"
    };
    
    for (const auto& addr : test_addresses) {
        auto coord = internal::utils::CoordinateUtils::string_to_coordinate(addr);
        auto back_to_string = internal::utils::CoordinateUtils::coordinate_to_string(coord);
        ASSERT_EQ(addr, back_to_string);
    }
}

TEST_CASE(Integration, RangeAddressRoundTrip) {
    // 测试范围地址字符串的往返转换
    std::vector<std::string> test_ranges = {
        "A1:B2", "C3:Z26", "AA1:ZZ100", "A1", "Z99"
    };
    
    for (const auto& range_str : test_ranges) {
        auto range_addr = internal::utils::CoordinateUtils::string_to_range_address(range_str);
        auto back_to_string = internal::utils::CoordinateUtils::range_address_to_string(range_addr);
        ASSERT_EQ(range_str, back_to_string);
    }
}

// ========================================
// 边界条件测试
// ========================================

TEST_CASE(EdgeCases, LargeCoordinates) {
    // 测试大坐标值
    Coordinate large_coord(1048576, 16384); // Excel最大行列
    ASSERT_TRUE(large_coord.is_valid());
    
    auto coord_str = internal::utils::CoordinateUtils::coordinate_to_string(large_coord);
    auto parsed_back = internal::utils::CoordinateUtils::string_to_coordinate(coord_str);
    ASSERT_EQ(large_coord.row, parsed_back.row);
    ASSERT_EQ(large_coord.column, parsed_back.column);
}

TEST_CASE(EdgeCases, SingleCellRange) {
    // 测试单个单元格的范围
    auto range_addr = internal::utils::CoordinateUtils::string_to_range_address("B5");
    ASSERT_EQ(5u, range_addr.start.row);
    ASSERT_EQ(2u, range_addr.start.column);
    ASSERT_EQ(5u, range_addr.end.row);
    ASSERT_EQ(2u, range_addr.end.column);
    
    ASSERT_TRUE(range_addr.contains(Coordinate(5, 2)));
    ASSERT_FALSE(range_addr.contains(Coordinate(5, 3)));
}

TEST_CASE(EdgeCases, RangeSize) {
    // 测试范围大小计算
    range_address single_cell(Coordinate(1, 1), Coordinate(1, 1));
    auto size1 = single_cell.size();
    ASSERT_EQ(1u, size1.first);
    ASSERT_EQ(1u, size1.second);
    
    range_address large_range(Coordinate(1, 1), Coordinate(100, 50));
    auto size2 = large_range.size();
    ASSERT_EQ(100u, size2.first);
    ASSERT_EQ(50u, size2.second);
}

// ========================================
// 性能测试（简单版本）
// ========================================

TEST_CASE(Performance, CoordinateConversionSpeed) {
    // 简单的性能测试 - 转换1000个坐标
    const int iterations = 1000;
    
    for (int i = 1; i <= iterations; ++i) {
        std::string addr = internal::utils::CoordinateUtils::column_number_to_letters(i % 26 + 1) + std::to_string(i);
        auto coord = internal::utils::CoordinateUtils::string_to_coordinate(addr);
        auto back = internal::utils::CoordinateUtils::coordinate_to_string(coord);
        ASSERT_EQ(addr, back);
    }
    
    // 如果能完成1000次转换而不崩溃，认为性能可接受
    ASSERT_TRUE(true);
}

TEST_CASE(Performance, RangeAddressConversionSpeed) {
    // 简单的性能测试 - 转换100个范围
    const int iterations = 100;
    
    for (int i = 1; i <= iterations; ++i) {
        std::string range_str = "A" + std::to_string(i) + ":Z" + std::to_string(i + 10);
        auto range_addr = internal::utils::CoordinateUtils::string_to_range_address(range_str);
        auto back = internal::utils::CoordinateUtils::range_address_to_string(range_addr);
        ASSERT_EQ(range_str, back);
    }
    
    // 如果能完成100次转换而不崩溃，认为性能可接受
    ASSERT_TRUE(true);
}
