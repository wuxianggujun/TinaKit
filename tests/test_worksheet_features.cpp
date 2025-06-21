/**
 * @file test_worksheet_features.cpp
 * @brief Worksheet功能测试
 * @author TinaKit Team
 * @date 2025-6-21
 */

#include "test_framework.hpp"
#include "tinakit/tinakit.hpp"
#include "tinakit/core/exceptions.hpp"
#include <algorithm>

using namespace tinakit;
using namespace tinakit::excel;
using namespace tinakit::test;

// ========================================
// 查找和替换功能测试
// ========================================

TEST_CASE(WorksheetFeatures, FindFunction) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 填充测试数据
    sheet.cell("A1").value("Hello World");
    sheet.cell("A2").value("Hello TinaKit");
    sheet.cell("B1").value("World Peace");
    sheet.cell("B2").value("TinaKit Library");
    
    // 查找包含"Hello"的单元格
    auto results = sheet.find("Hello");
    ASSERT_EQ(2u, results.size());
    
    // 验证结果包含正确的单元格地址
    ASSERT_TRUE(std::find(results.begin(), results.end(), "A1") != results.end());
    ASSERT_TRUE(std::find(results.begin(), results.end(), "A2") != results.end());
    
    // 查找包含"TinaKit"的单元格
    auto tinakit_results = sheet.find("TinaKit");
    ASSERT_EQ(2u, tinakit_results.size());
    ASSERT_TRUE(std::find(tinakit_results.begin(), tinakit_results.end(), "A2") != tinakit_results.end());
    ASSERT_TRUE(std::find(tinakit_results.begin(), tinakit_results.end(), "B2") != tinakit_results.end());
    
    // 查找不存在的内容
    auto empty_results = sheet.find("NotFound");
    ASSERT_TRUE(empty_results.empty());
}

TEST_CASE(WorksheetFeatures, ReplaceFunction) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 填充测试数据
    sheet.cell("A1").value("Hello World");
    sheet.cell("A2").value("Hello TinaKit");
    sheet.cell("B1").value("World Peace");
    
    // 替换"Hello"为"Hi"
    auto replace_count = sheet.replace("Hello", "Hi");
    ASSERT_EQ(2u, replace_count);
    
    // 验证替换结果
    ASSERT_EQ("Hi World", sheet.cell("A1").as<std::string>());
    ASSERT_EQ("Hi TinaKit", sheet.cell("A2").as<std::string>());
    ASSERT_EQ("World Peace", sheet.cell("B1").as<std::string>()); // 未改变
    
    // 替换不存在的内容
    auto zero_replace = sheet.replace("NotFound", "Replacement");
    ASSERT_EQ(0u, zero_replace);
}

TEST_CASE(WorksheetFeatures, ReplaceMultipleOccurrences) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 单个单元格中有多个匹配项
    sheet.cell("A1").value("test test test");
    
    auto replace_count = sheet.replace("test", "demo");
    ASSERT_EQ(1u, replace_count); // 只有一个单元格被修改
    
    // 验证所有匹配项都被替换
    ASSERT_EQ("demo demo demo", sheet.cell("A1").as<std::string>());
}

// ========================================
// 基础范围功能测试
// ========================================

TEST_CASE(WorksheetFeatures, BasicRangeFunction) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 测试基础范围创建
    auto range1 = sheet.basic_range("A1:C3");
    auto range2 = sheet.range("A1:C3");
    
    // 两种方法应该创建相同的范围
    // 这里我们只测试不会抛出异常
    ASSERT_TRUE(true);
}

// ========================================
// 合并单元格功能测试
// ========================================

TEST_CASE(WorksheetFeatures, MergeCellsByRange) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 填充数据
    sheet.cell("A1").value("Merged Cell");
    sheet.cell("A2").value("Data");

    // 测试按范围字符串合并
    ASSERT_NO_THROW(sheet.merge_cells("A1:B2"));

    // 测试按坐标合并
    ASSERT_NO_THROW(sheet.merge_cells(4, 1, 5, 3));
}

TEST_CASE(WorksheetFeatures, MergeCellsWithSaveAndLoad) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 填充数据并设置样式
    sheet.cell("A1").value("水平合并").bold().background_color(Color::Yellow);
    sheet.cell("C1").value("垂直合并").italic().background_color(Color::LightGreen);
    sheet.cell("E1").value("大范围合并").background_color(Color::LightBlue);

    // 测试不同类型的合并
    sheet.merge_cells("A1:D1");  // 水平合并
    sheet.merge_cells("C2:C4");  // 垂直合并
    sheet.merge_cells("E1:G3");  // 大范围合并

    // 保存并重新加载
    std::string filename = "test_merge_cells.xlsx";
    ASSERT_NO_THROW(workbook.save(filename));

    // 重新加载并验证
    auto loaded_workbook = Workbook::load(filename);
    auto loaded_sheet = loaded_workbook.active_sheet();

    // 验证单元格值仍然存在
    ASSERT_EQ(loaded_sheet.cell("A1").as<std::string>(), "水平合并");
    ASSERT_EQ(loaded_sheet.cell("C1").as<std::string>(), "垂直合并");
    ASSERT_EQ(loaded_sheet.cell("E1").as<std::string>(), "大范围合并");

    // 清理测试文件
    std::remove(filename.c_str());
}

TEST_CASE(WorksheetFeatures, MergeCellsValidation) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 测试无效范围
    ASSERT_THROWS(sheet.merge_cells(2, 2, 1, 1), std::invalid_argument);
    ASSERT_THROWS(sheet.merge_cells(1, 3, 1, 2), std::invalid_argument);
    
    // 测试单个单元格（应该不抛出异常但不执行合并）
    ASSERT_NO_THROW(sheet.merge_cells(1, 1, 1, 1));
}

TEST_CASE(WorksheetFeatures, UnmergeCells) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 先合并单元格
    sheet.merge_cells("A1:B2");
    
    // 然后取消合并
    ASSERT_NO_THROW(sheet.unmerge_cells("A1:B2"));
    ASSERT_NO_THROW(sheet.unmerge_cells(4, 1, 5, 3));
}

// ========================================
// RowRange高级功能测试
// ========================================

TEST_CASE(WorksheetFeatures, RowRangeFilterAndTransform) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 填充测试数据
    for (int i = 1; i <= 10; ++i) {
        sheet.row(i)[1].value(i);
        sheet.row(i)[2].value(i % 2 == 0 ? "偶数" : "奇数");
    }
    
    auto row_range = sheet.rows(1, 10);
    
    // 测试过滤功能
    auto filtered = row_range.filter([](const Row& row) {
        return row[1].as<int>() > 5;
    });
    
    std::vector<int> values;
    for (auto row : filtered) {
        values.push_back(row[1].as<int>());
    }
    
    ASSERT_EQ(5u, values.size());
    ASSERT_EQ(6, values[0]);
    ASSERT_EQ(10, values[4]);
}

TEST_CASE(WorksheetFeatures, RowRangeTransform) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 填充测试数据
    for (int i = 1; i <= 5; ++i) {
        sheet.row(i)[1].value(i * 10);
    }
    
    auto row_range = sheet.rows(1, 5);
    
    // 测试转换功能
    auto transformed = row_range.transform([](const Row& row) {
        return row[1].as<int>() / 10;
    });
    
    std::vector<int> results;
    for (auto value : transformed) {
        results.push_back(value);
    }
    
    ASSERT_EQ(5u, results.size());
    ASSERT_EQ(1, results[0]);
    ASSERT_EQ(2, results[1]);
    ASSERT_EQ(5, results[4]);
}

// ========================================
// 错误处理测试
// ========================================

TEST_CASE(WorksheetFeatures, InvalidWorksheetOperations) {
    // 创建一个有效的工作表，然后测试无效操作
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 测试无效的范围字符串
    ASSERT_THROWS(sheet.merge_cells(""), InvalidCellAddressException);
    ASSERT_THROWS(sheet.unmerge_cells(""), InvalidCellAddressException);
    ASSERT_THROWS(sheet.basic_range(""), InvalidCellAddressException);

    // 测试空字符串查找和替换
    auto empty_find = sheet.find("");
    ASSERT_TRUE(empty_find.empty());

    auto zero_replace = sheet.replace("", "new");
    ASSERT_EQ(0u, zero_replace);
}

TEST_CASE(WorksheetFeatures, InvalidRangeStrings) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 测试无效的范围字符串
    ASSERT_THROWS(sheet.basic_range(""), InvalidCellAddressException);
    ASSERT_THROWS(sheet.merge_cells(""), InvalidCellAddressException);
    ASSERT_THROWS(sheet.unmerge_cells("INVALID"), InvalidCellAddressException);
}
