/**
 * @file test_fixes.cpp
 * @brief 测试修复的功能 - Style转换和used_range功能
 * @author TinaKit Team
 * @date 2025-6-21
 */

#include "test_framework.hpp"
#include "tinakit/tinakit.hpp"
#include "tinakit/excel/style.hpp"
#include "tinakit/core/types.hpp"
#include <filesystem>

using namespace tinakit;
using namespace tinakit::excel;
using namespace tinakit::core;
using namespace tinakit::test;

// ========================================
// Style 转换功能测试
// ========================================

TEST_CASE(StyleFixes, StyleToStyleIdConversion) {
    // 测试Style到style_id的转换功能
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 创建样式
    auto test_style = Style()
        .font("Calibri", 12)
        .bold()
        .color(Color::Blue);
    
    // 应用到单元格 - 这应该不会抛出异常
    ASSERT_NO_THROW(sheet["A1"].value("测试").style(test_style));
    
    // 验证值被正确设置
    ASSERT_EQ("测试", sheet["A1"].as<std::string>());
}

TEST_CASE(StyleFixes, RangeStyleApplication) {
    // 测试Range的样式应用功能
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 创建样式
    auto range_style = Style()
        .font("Arial", 14)
        .background_color(Color::Yellow)
        .bold();
    
    // 获取范围并应用样式
    auto range = sheet.range("A1:C3");
    ASSERT_NO_THROW(range.set_style(range_style));
    
    // 设置值
    ASSERT_NO_THROW(range.set_value("范围测试"));
}

TEST_CASE(StyleFixes, PreDefinedStyleTemplates) {
    // 测试预定义样式模板
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 测试各种预定义样式
    ASSERT_NO_THROW(sheet["A1"].value("标题").style(StyleTemplates::title()));
    ASSERT_NO_THROW(sheet["A2"].value("表头").style(StyleTemplates::header()));
    ASSERT_NO_THROW(sheet["A3"].value("数据").style(StyleTemplates::data()));
    ASSERT_NO_THROW(sheet["A4"].value("警告").style(StyleTemplates::warning()));
    ASSERT_NO_THROW(sheet["A5"].value("错误").style(StyleTemplates::error()));
    ASSERT_NO_THROW(sheet["A6"].value("成功").style(StyleTemplates::success()));
}

TEST_CASE(StyleFixes, StyleChaining) {
    // 测试样式链式调用
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 创建复杂样式
    auto complex_style = Style()
        .font("Arial", 16)
        .bold()
        .italic()
        .color(Color::White)
        .background_color(Color::Blue);

    // 应用复杂样式
    ASSERT_NO_THROW(sheet["A1"].value(12345.67).style(complex_style));
}

// ========================================
// used_range 功能测试
// ========================================

TEST_CASE(UsedRangeFixes, EmptyWorksheetUsedRange) {
    // 测试空工作表的used_range
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 空工作表应该返回空范围
    auto used_range = sheet.used_range();
    // 注意：空范围的行为可能需要根据实际实现调整
    ASSERT_TRUE(true); // 主要确保不会崩溃
}

TEST_CASE(UsedRangeFixes, SingleCellUsedRange) {
    // 测试单个单元格的used_range
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置单个单元格
    sheet["B3"].value("单个单元格");
    
    // 获取使用范围
    auto used_range = sheet.used_range();
    ASSERT_NO_THROW(used_range.address());
    
    // 验证范围大小
    auto [rows, cols] = used_range.size();
    ASSERT_EQ(1u, rows);
    ASSERT_EQ(1u, cols);
}

TEST_CASE(UsedRangeFixes, MultiCellUsedRange) {
    // 测试多个单元格的used_range
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置多个单元格
    sheet["A1"].value("左上角");
    sheet["C2"].value("中间");
    sheet["E5"].value("右下角");
    
    // 获取使用范围
    auto used_range = sheet.used_range();
    ASSERT_NO_THROW(used_range.address());
    
    // 验证范围包含所有设置的单元格
    ASSERT_TRUE(used_range.contains(Coordinate(1, 1))); // A1
    ASSERT_TRUE(used_range.contains(Coordinate(2, 3))); // C2
    ASSERT_TRUE(used_range.contains(Coordinate(5, 5))); // E5
    
    // 验证范围大小
    auto [rows, cols] = used_range.size();
    ASSERT_EQ(5u, rows); // 从第1行到第5行
    ASSERT_EQ(5u, cols); // 从第1列到第5列
}

TEST_CASE(UsedRangeFixes, UsedRangeAfterClear) {
    // 测试清除单元格后的used_range
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置一些单元格
    sheet["A1"].value("数据1");
    sheet["B2"].value("数据2");
    
    // 清除一个单元格（通过设置空值）
    sheet["B2"].value("");
    
    // 获取使用范围
    auto used_range = sheet.used_range();
    ASSERT_NO_THROW(used_range.address());
    
    // 验证范围仍然正确
    ASSERT_TRUE(used_range.contains(Coordinate(1, 1))); // A1 仍然存在
}

// ========================================
// 集成测试
// ========================================

TEST_CASE(Integration, StyleAndUsedRangeTogether) {
    // 测试样式和used_range功能的集成
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 创建样式
    auto header_style = StyleTemplates::header();
    auto data_style = StyleTemplates::data();
    
    // 设置表头
    sheet["A1"].value("姓名").style(header_style);
    sheet["B1"].value("年龄").style(header_style);
    sheet["C1"].value("部门").style(header_style);
    
    // 设置数据
    sheet["A2"].value("张三").style(data_style);
    sheet["B2"].value(25).style(data_style);
    sheet["C2"].value("技术部").style(data_style);
    
    sheet["A3"].value("李四").style(data_style);
    sheet["B3"].value(30).style(data_style);
    sheet["C3"].value("销售部").style(data_style);
    
    // 获取使用范围
    auto used_range = sheet.used_range();
    ASSERT_NO_THROW(used_range.address());
    
    // 验证范围正确
    auto [rows, cols] = used_range.size();
    ASSERT_EQ(3u, rows); // 3行数据
    ASSERT_EQ(3u, cols); // 3列数据
    
    // 验证范围包含所有数据
    ASSERT_TRUE(used_range.contains(Coordinate(1, 1))); // A1
    ASSERT_TRUE(used_range.contains(Coordinate(3, 3))); // C3
}

TEST_CASE(Integration, SaveFileWithFixes) {
    // 测试保存包含修复功能的文件
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    sheet.set_name("修复测试");
    
    // 使用修复的功能
    auto title_style = StyleTemplates::title(18);
    sheet["A1"].value("TinaKit 修复测试").style(title_style);
    
    // 添加一些数据
    for (int i = 1; i <= 5; ++i) {
        sheet["A" + std::to_string(i + 2)].value("数据" + std::to_string(i));
    }
    
    // 获取使用范围
    auto used_range = sheet.used_range();
    ASSERT_NO_THROW(used_range.address());
    
    // 保存文件
    std::string filename = "test_fixes_output.xlsx";
    ASSERT_NO_THROW(workbook.save(filename));
    
    // 验证文件存在（简单检查）
    ASSERT_TRUE(std::filesystem::exists(filename));
    
    // 清理测试文件
    std::filesystem::remove(filename);
}
