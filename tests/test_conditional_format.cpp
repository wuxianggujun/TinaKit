/**
 * @file test_conditional_format.cpp
 * @brief 条件格式测试
 * @author TinaKit Team
 * @date 2025-6-21
 */

#include "test_framework.hpp"
#include "tinakit/tinakit.hpp"
#include "tinakit/excel/conditional_format.hpp"
#include <algorithm>

using namespace tinakit;
using namespace tinakit::excel;
using namespace tinakit::test;

TEST_CASE(ConditionalFormat, BasicValueConditions) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置测试数据
    sheet["A1"].value(95);  // 高分
    sheet["A2"].value(75);  // 中等分数
    sheet["A3"].value(45);  // 低分
    sheet["A4"].value(88);  // 高分
    sheet["A5"].value(62);  // 中等分数
    
    // 添加条件格式：分数大于90显示绿色背景
    sheet.conditional_format("A1:A5")
        .when_greater_than(90)
        .background_color(Color::Green)
        .apply();
    
    // 添加条件格式：分数小于60显示红色背景
    sheet.conditional_format("A1:A5")
        .when_less_than(60)
        .background_color(Color::Red)
        .apply();
    
    // 验证条件格式已添加
    auto formats = sheet.get_conditional_formats();
    ASSERT_EQ(2u, formats.size());
    
    // 验证第一个条件格式
    ASSERT_EQ("A1:A5", formats[0].range);
    ASSERT_EQ(1u, formats[0].rules.size());
    ASSERT_TRUE(formats[0].rules[0].type == ConditionalFormatType::CellValue);
    ASSERT_TRUE(formats[0].rules[0].operator_type == ConditionalFormatOperator::GreaterThan);
    ASSERT_EQ("90", formats[0].rules[0].formulas[0]);
    
    // 验证第二个条件格式
    ASSERT_EQ("A1:A5", formats[1].range);
    ASSERT_EQ(1u, formats[1].rules.size());
    ASSERT_TRUE(formats[1].rules[0].type == ConditionalFormatType::CellValue);
    ASSERT_TRUE(formats[1].rules[0].operator_type == ConditionalFormatOperator::LessThan);
    ASSERT_EQ("60", formats[1].rules[0].formulas[0]);
}

TEST_CASE(ConditionalFormat, TextConditions) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置测试数据
    sheet["B1"].value("Excellent");
    sheet["B2"].value("Good");
    sheet["B3"].value("Poor");
    sheet["B4"].value("Excellent");
    sheet["B5"].value("Average");
    
    // 添加条件格式：包含"Excellent"的单元格显示金色背景
    sheet.conditional_format("B1:B5")
        .when_contains("Excellent")
        .background_color(Color::Yellow)
        .font_color(Color::Black)
        .bold()
        .apply();
    
    // 验证条件格式已添加
    auto formats = sheet.get_conditional_formats();
    ASSERT_EQ(1u, formats.size());
    
    // 验证条件格式设置
    ASSERT_EQ("B1:B5", formats[0].range);
    ASSERT_EQ(1u, formats[0].rules.size());
    ASSERT_TRUE(formats[0].rules[0].type == ConditionalFormatType::ContainsText);
    ASSERT_EQ("Excellent", formats[0].rules[0].text);
}

TEST_CASE(ConditionalFormat, BetweenCondition) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置测试数据
    sheet["C1"].value(15);
    sheet["C2"].value(25);
    sheet["C3"].value(35);
    sheet["C4"].value(45);
    sheet["C5"].value(55);
    
    // 添加条件格式：值在20-40之间显示蓝色背景
    sheet.conditional_format("C1:C5")
        .when_between(20, 40)
        .background_color(Color::Blue)
        .font_color(Color::White)
        .apply();
    
    // 验证条件格式已添加
    auto formats = sheet.get_conditional_formats();
    ASSERT_EQ(1u, formats.size());
    
    // 验证条件格式设置
    ASSERT_EQ("C1:C5", formats[0].range);
    ASSERT_EQ(1u, formats[0].rules.size());
    ASSERT_TRUE(formats[0].rules[0].type == ConditionalFormatType::CellValue);
    ASSERT_TRUE(formats[0].rules[0].operator_type == ConditionalFormatOperator::Between);
    ASSERT_EQ(2u, formats[0].rules[0].formulas.size());
    ASSERT_EQ("20", formats[0].rules[0].formulas[0]);
    ASSERT_EQ("40", formats[0].rules[0].formulas[1]);
}

TEST_CASE(ConditionalFormat, MultipleConditionsWithPriority) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置测试数据
    sheet["D1"].value(95);
    sheet["D2"].value(85);
    sheet["D3"].value(75);
    sheet["D4"].value(65);
    sheet["D5"].value(55);
    
    // 添加多个条件格式，测试优先级
    // 优先级1：分数大于90显示绿色（最高优先级）
    sheet.conditional_format("D1:D5")
        .when_greater_than(90)
        .background_color(Color::Green)
        .apply();
    
    // 优先级2：分数大于80显示黄色
    sheet.conditional_format("D1:D5")
        .when_greater_than(80)
        .background_color(Color::Yellow)
        .apply();
    
    // 优先级3：分数大于70显示橙色
    sheet.conditional_format("D1:D5")
        .when_greater_than(70)
        .background_color(Color::Red)  // 使用红色代替橙色
        .apply();
    
    // 验证条件格式已添加
    auto formats = sheet.get_conditional_formats();
    ASSERT_EQ(3u, formats.size());
    
    // 验证所有条件格式都应用到同一范围
    for (const auto& format : formats) {
        ASSERT_EQ("D1:D5", format.range);
        ASSERT_EQ(1u, format.rules.size());
        ASSERT_TRUE(format.rules[0].type == ConditionalFormatType::CellValue);
        ASSERT_TRUE(format.rules[0].operator_type == ConditionalFormatOperator::GreaterThan);
    }
}

TEST_CASE(ConditionalFormat, FontStyling) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置测试数据
    sheet["E1"].value("Important");
    sheet["E2"].value("Normal");
    sheet["E3"].value("Important");
    
    // 添加条件格式：包含"Important"的单元格使用特殊字体样式
    sheet.conditional_format("E1:E3")
        .when_contains("Important")
        .font("Arial", 12)
        .bold()
        .italic()
        .font_color(Color::Red)
        .background_color(Color::LightGray)
        .apply();
    
    // 验证条件格式已添加
    auto formats = sheet.get_conditional_formats();
    ASSERT_EQ(1u, formats.size());
    
    // 验证条件格式设置
    const auto& rule = formats[0].rules[0];
    ASSERT_TRUE(rule.type == ConditionalFormatType::ContainsText);
    ASSERT_EQ("Important", rule.text);
    
    // 注意：字体和填充信息在apply()后会转换为dxf_id，
    // 所以这里主要验证条件设置是否正确
}

TEST_CASE(ConditionalFormat, DuplicateAndUniqueValues) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置测试数据（包含重复值）
    sheet["F1"].value("Apple");
    sheet["F2"].value("Banana");
    sheet["F3"].value("Apple");   // 重复
    sheet["F4"].value("Cherry");
    sheet["F5"].value("Banana");  // 重复
    
    // 添加条件格式：重复值显示红色背景
    sheet.conditional_format("F1:F5")
        .when_duplicate_values()
        .background_color(Color::Red)
        .apply();
    
    // 添加条件格式：唯一值显示绿色背景
    sheet.conditional_format("F1:F5")
        .when_unique_values()
        .background_color(Color::Green)
        .apply();
    
    // 验证条件格式已添加
    auto formats = sheet.get_conditional_formats();
    ASSERT_EQ(2u, formats.size());
    
    // 验证重复值条件格式
    ASSERT_TRUE(formats[0].rules[0].type == ConditionalFormatType::DuplicateValues);

    // 验证唯一值条件格式
    ASSERT_TRUE(formats[1].rules[0].type == ConditionalFormatType::UniqueValues);
}

TEST_CASE(ConditionalFormat, SaveAndLoad) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置测试数据
    sheet["G1"].value(100);
    sheet["G2"].value(80);
    sheet["G3"].value(60);
    
    // 添加条件格式
    sheet.conditional_format("G1:G3")
        .when_greater_than(90)
        .background_color(Color::Green)
        .font_color(Color::White)
        .bold()
        .apply();
    
    // 保存文件
    std::string filename = "test_conditional_format.xlsx";
    workbook.save(filename);
    
    // 重新加载文件
    auto loaded_workbook = Workbook::load(filename);
    auto loaded_sheet = loaded_workbook.active_sheet();
    
    // 验证条件格式是否正确保存和加载
    auto loaded_formats = loaded_sheet.get_conditional_formats();
    ASSERT_EQ(1u, loaded_formats.size());
    
    const auto& loaded_format = loaded_formats[0];
    ASSERT_EQ("G1:G3", loaded_format.range);
    ASSERT_EQ(1u, loaded_format.rules.size());
    
    const auto& loaded_rule = loaded_format.rules[0];
    ASSERT_TRUE(loaded_rule.type == ConditionalFormatType::CellValue);
    ASSERT_TRUE(loaded_rule.operator_type == ConditionalFormatOperator::GreaterThan);
    ASSERT_EQ("90", loaded_rule.formulas[0]);
    
    // 清理测试文件
    std::remove(filename.c_str());
}
