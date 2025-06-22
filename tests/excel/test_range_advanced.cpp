/**
 * @file test_range_advanced.cpp
 * @brief Range类高级功能测试
 * @author TinaKit Team
 * @date 2025-6-21
 */

#include "test_framework.hpp"
#include "tinakit/excel/workbook.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/range.hpp"
#include "tinakit/excel/cell.hpp"
#include "tinakit/core/types.hpp"

using namespace tinakit::excel;
using namespace tinakit::test;
using namespace tinakit;

// ========================================
// Range 批量数据操作测试
// ========================================

TEST_CASE(RangeAdvanced, BatchSetValues) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 创建范围
    auto range = sheet.range("A1:B2");

    // 准备简单的二维数据
    std::vector<std::vector<Cell::CellValue>> values = {
        {std::string("A1"), std::string("B1")},
        {1, 2}
    };

    // 批量设置值
    range.set_values(values);

    // 验证设置的值
    ASSERT_EQ(sheet["A1"].as<std::string>(), "A1");
    ASSERT_EQ(sheet["B1"].as<std::string>(), "B1");
    ASSERT_EQ(sheet["A2"].as<int>(), 1);
    ASSERT_EQ(sheet["B2"].as<int>(), 2);
}

TEST_CASE(RangeAdvanced, BatchGetValues) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 设置一些值
    sheet["A1"].value("Text1");
    sheet["B1"].value("Text2");
    sheet["A2"].value(42);
    sheet["B2"].value(3.14);

    // 创建范围并获取值
    auto range = sheet.range("A1:B2");
    auto values = range.get_values();

    // 验证获取的值
    ASSERT_EQ(values.size(), 2);
    ASSERT_EQ(values[0].size(), 2);
    ASSERT_EQ(values[1].size(), 2);

    ASSERT_EQ(std::get<std::string>(values[0][0]), "Text1");
    ASSERT_EQ(std::get<std::string>(values[0][1]), "Text2");
    ASSERT_EQ(std::get<int>(values[1][0]), 42);
    ASSERT_EQ(std::get<double>(values[1][1]), 3.14);
}

TEST_CASE(RangeAdvanced, UniformValueSetting) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 创建范围
    auto range = sheet.range("A1:C3");

    // 设置统一值
    range.set_value(std::string("Uniform"));

    // 验证所有单元格都有相同值
    for (auto cell : range) {
        ASSERT_EQ(cell.as<std::string>(), "Uniform");
    }
}

TEST_CASE(RangeAdvanced, RangeClear) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 设置一些值
    sheet["A1"].value("Test1");
    sheet["A2"].value("Test2");
    sheet["B1"].value("Test3");
    sheet["B2"].value("Test4");

    // 创建范围并清除
    auto range = sheet.range("A1:B2");
    range.clear();

    // 验证所有单元格都被清除
    for (auto cell : range) {
        ASSERT_TRUE(cell.empty());
    }
}


// ========================================
// Range 合并操作测试
// ========================================

TEST_CASE(RangeAdvanced, MergeRange) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 创建范围
    auto range = sheet.range("A1:C3");
    
    // 合并范围
    range.merge();
    
    // 验证范围已合并
    ASSERT_TRUE(range.is_merged());
}

TEST_CASE(RangeAdvanced, UnmergeRange) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 创建范围并合并
    auto range = sheet.range("A1:C3");
    range.merge();
    ASSERT_TRUE(range.is_merged());
    
    // 取消合并
    range.unmerge();
    
    // 验证范围已取消合并
    ASSERT_FALSE(range.is_merged());
}

// ========================================
// Range 复制移动操作测试
// ========================================

TEST_CASE(RangeAdvanced, CopyRange) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置源数据
    sheet["A1"].value("Source1");
    sheet["A2"].value("Source2");
    sheet["B1"].value(100);
    sheet["B2"].value(200);
    
    // 创建源范围和目标范围
    auto source_range = sheet.range("A1:B2");
    auto dest_range = sheet.range("D1:E2");
    
    // 复制范围
    source_range.copy_to(dest_range);
    
    // 验证源数据仍然存在
    ASSERT_EQ(sheet["A1"].as<std::string>(), "Source1");
    ASSERT_EQ(sheet["A2"].as<std::string>(), "Source2");
    ASSERT_EQ(sheet["B1"].as<int>(), 100);
    ASSERT_EQ(sheet["B2"].as<int>(), 200);
    
    // 验证目标数据已复制
    ASSERT_EQ(sheet["D1"].as<std::string>(), "Source1");
    ASSERT_EQ(sheet["D2"].as<std::string>(), "Source2");
    ASSERT_EQ(sheet["E1"].as<int>(), 100);
    ASSERT_EQ(sheet["E2"].as<int>(), 200);
}

TEST_CASE(RangeAdvanced, MoveRange) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置源数据
    sheet["A1"].value("Move1");
    sheet["A2"].value("Move2");
    sheet["B1"].value(300);
    sheet["B2"].value(400);
    
    // 创建源范围和目标范围
    auto source_range = sheet.range("A1:B2");
    auto dest_range = sheet.range("D1:E2");
    
    // 移动范围
    source_range.move_to(dest_range);
    
    // 验证源数据已清除
    ASSERT_TRUE(sheet["A1"].empty());
    ASSERT_TRUE(sheet["A2"].empty());
    ASSERT_TRUE(sheet["B1"].empty());
    ASSERT_TRUE(sheet["B2"].empty());
    
    // 验证目标数据已移动
    ASSERT_EQ(sheet["D1"].as<std::string>(), "Move1");
    ASSERT_EQ(sheet["D2"].as<std::string>(), "Move2");
    ASSERT_EQ(sheet["E1"].as<int>(), 300);
    ASSERT_EQ(sheet["E2"].as<int>(), 400);
}

// ========================================
// Range 信息查询测试
// ========================================

TEST_CASE(RangeAdvanced, RangeInfo) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 创建范围
    auto range = sheet.range("B2:D5");
    
    // 验证范围信息
    ASSERT_EQ(range.address(), "B2:D5");
    ASSERT_EQ(range.start_position().row, 2);
    ASSERT_EQ(range.start_position().column, 2);
    ASSERT_EQ(range.end_position().row, 5);
    ASSERT_EQ(range.end_position().column, 4);
    
    auto size = range.size();
    ASSERT_EQ(size.first, 4);  // 4行
    ASSERT_EQ(size.second, 3); // 3列
}

TEST_CASE(RangeAdvanced, RangeContains) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 创建范围
    auto range = sheet.range("B2:D5");
    
    // 测试包含关系
    ASSERT_TRUE(range.contains(core::Coordinate(2, 2)));  // B2
    ASSERT_TRUE(range.contains(core::Coordinate(3, 3)));  // C3
    ASSERT_TRUE(range.contains(core::Coordinate(5, 4)));  // D5
    
    ASSERT_FALSE(range.contains(core::Coordinate(1, 1))); // A1
    ASSERT_FALSE(range.contains(core::Coordinate(6, 6))); // F6
    ASSERT_FALSE(range.contains(core::Coordinate(2, 1))); // A2
}

TEST_CASE(RangeAdvanced, RangeOverlaps) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 创建范围
    auto range1 = sheet.range("A1:C3");
    auto range2 = sheet.range("B2:D4");
    auto range3 = sheet.range("E1:F2");
    
    // 测试重叠关系
    ASSERT_TRUE(range1.overlaps(range2));   // 有重叠
    ASSERT_FALSE(range1.overlaps(range3));  // 无重叠
    ASSERT_FALSE(range2.overlaps(range3));  // 无重叠
}

// ========================================
// Range 链式调用测试
// ========================================

TEST_CASE(RangeAdvanced, ChainedOperations) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 链式调用测试
    auto range = sheet.range("A1:C3");
    range.set_value(std::string("Chain"))
         .set_style(1)
         .merge();
    
    // 验证操作结果
    ASSERT_TRUE(range.is_merged());
    for (auto cell : range) {
        if (!cell.empty()) {
            ASSERT_EQ(cell.as<std::string>(), "Chain");
        }
    }
}

// ========================================
// Range 性能和缓存测试
// ========================================

TEST_CASE(RangeAdvanced, CachePerformance) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 创建一个大范围
    auto range = sheet.range("A1:J10");

    // 填充数据（使用批量操作）
    std::vector<std::vector<Cell::CellValue>> values;
    for (std::size_t i = 0; i < 10; ++i) {
        std::vector<Cell::CellValue> row;
        for (std::size_t j = 0; j < 10; ++j) {
            row.push_back(static_cast<int>(i * 10 + j));
        }
        values.push_back(std::move(row));
    }
    range.set_values(values);

    // 验证缓存大小（通过迭代器访问触发缓存）
    int count = 0;
    for (auto cell : range) {
        ASSERT_EQ(cell.as<int>(), count);
        ++count;
    }

    // 验证缓存存在
    ASSERT_TRUE(range.cache_size() > 0);

    // 重复迭代，应该使用缓存
    auto initial_cache_size = range.cache_size();
    count = 0;
    for (auto cell : range) {
        ASSERT_EQ(cell.as<int>(), count);
        ++count;
        if (count >= 10) break; // 只检查前10个
    }

    // 清除缓存
    range.clear_cache();
    ASSERT_EQ(range.cache_size(), 0);
}

TEST_CASE(RangeAdvanced, LargeBatchOperations) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 创建大范围
    auto range = sheet.range("A1:Z100");

    // 准备大量数据
    std::vector<std::vector<Cell::CellValue>> values;
    for (std::size_t i = 0; i < 100; ++i) {
        std::vector<Cell::CellValue> row;
        for (std::size_t j = 0; j < 26; ++j) {
            row.push_back(static_cast<int>(i * 26 + j));
        }
        values.push_back(std::move(row));
    }

    // 批量设置值
    range.set_values(values);

    // 验证部分数据
    ASSERT_EQ(sheet["A1"].as<int>(), 0);
    ASSERT_EQ(sheet["Z1"].as<int>(), 25);
    ASSERT_EQ(sheet["A100"].as<int>(), 99 * 26);
    ASSERT_EQ(sheet["Z100"].as<int>(), 99 * 26 + 25);

    // 批量获取值
    auto retrieved_values = range.get_values();
    ASSERT_EQ(retrieved_values.size(), 100);
    ASSERT_EQ(retrieved_values[0].size(), 26);

    // 验证数据一致性
    ASSERT_EQ(std::get<int>(retrieved_values[0][0]), 0);
    ASSERT_EQ(std::get<int>(retrieved_values[0][25]), 25);
    ASSERT_EQ(std::get<int>(retrieved_values[99][0]), 99 * 26);
    ASSERT_EQ(std::get<int>(retrieved_values[99][25]), 99 * 26 + 25);
}
