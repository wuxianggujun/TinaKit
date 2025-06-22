/**
 * @file test_formula_engine.cpp
 * @brief 公式引擎测试
 * @author TinaKit Team
 * @date 2025-6-21
 */

#include "test_framework.hpp"
#include "tinakit/tinakit.hpp"
#include "tinakit/excel/formula_engine.hpp"
#include <algorithm>

using namespace tinakit;
using namespace tinakit::excel;
using namespace tinakit::test;

TEST_CASE(FormulaEngine, BasicArithmetic) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置一些基础数据
    sheet["A1"].value(10);
    sheet["A2"].value(20);
    sheet["A3"].value(5);
    
    // 测试基本算术运算
    sheet["B1"].formula("A1 + A2");  // 10 + 20 = 30
    sheet["B2"].formula("A1 - A3");  // 10 - 5 = 5
    sheet["B3"].formula("A1 * A3");  // 10 * 5 = 50
    sheet["B4"].formula("A2 / A3");  // 20 / 5 = 4
    sheet["B5"].formula("A1 ^ 2");   // 10 ^ 2 = 100
    
    // 重新计算公式
    workbook.recalculate_formulas();
    
    // 验证结果
    ASSERT_EQ(30.0, sheet["B1"].as<double>());
    ASSERT_EQ(5.0, sheet["B2"].as<double>());
    ASSERT_EQ(50.0, sheet["B3"].as<double>());
    ASSERT_EQ(4.0, sheet["B4"].as<double>());
    ASSERT_EQ(100.0, sheet["B5"].as<double>());
}

TEST_CASE(FormulaEngine, SumFunction) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置数据
    sheet["A1"].value(1);
    sheet["A2"].value(2);
    sheet["A3"].value(3);
    sheet["A4"].value(4);
    sheet["A5"].value(5);
    
    // 测试SUM函数
    sheet["B1"].formula("SUM(A1:A5)");  // 1+2+3+4+5 = 15
    sheet["B2"].formula("SUM(A1, A3, A5)");  // 1+3+5 = 9
    
    // 重新计算公式
    workbook.recalculate_formulas();
    
    // 验证结果
    ASSERT_EQ(15.0, sheet["B1"].as<double>());
    ASSERT_EQ(9.0, sheet["B2"].as<double>());
}

TEST_CASE(FormulaEngine, AverageFunction) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置数据
    sheet["A1"].value(10);
    sheet["A2"].value(20);
    sheet["A3"].value(30);
    
    // 测试AVERAGE函数
    sheet["B1"].formula("AVERAGE(A1:A3)");  // (10+20+30)/3 = 20
    
    // 重新计算公式
    workbook.recalculate_formulas();
    
    // 验证结果
    ASSERT_EQ(20.0, sheet["B1"].as<double>());
}

TEST_CASE(FormulaEngine, CountFunction) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置数据（包括空单元格）
    sheet["A1"].value(1);
    sheet["A2"].value(2);
    // A3 为空
    sheet["A4"].value(4);
    sheet["A5"].value("text");
    
    // 测试COUNT函数
    sheet["B1"].formula("COUNT(A1:A5)");  // 应该计算所有非空单元格
    
    // 重新计算公式
    workbook.recalculate_formulas();
    
    // 验证结果（应该是4个非空单元格）
    ASSERT_EQ(4.0, sheet["B1"].as<double>());
}

TEST_CASE(FormulaEngine, MaxMinFunctions) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置数据
    sheet["A1"].value(5);
    sheet["A2"].value(15);
    sheet["A3"].value(3);
    sheet["A4"].value(12);
    
    // 测试MAX和MIN函数
    sheet["B1"].formula("MAX(A1:A4)");  // 最大值 = 15
    sheet["B2"].formula("MIN(A1:A4)");  // 最小值 = 3
    
    // 重新计算公式
    workbook.recalculate_formulas();
    
    // 验证结果
    ASSERT_EQ(15.0, sheet["B1"].as<double>());
    ASSERT_EQ(3.0, sheet["B2"].as<double>());
}

TEST_CASE(FormulaEngine, IfFunction) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置数据
    sheet["A1"].value(10);
    sheet["A2"].value(5);
    
    // 测试IF函数
    sheet["B1"].formula("IF(A1 > A2, \"Greater\", \"Less or Equal\")");
    sheet["B2"].formula("IF(A2 > A1, \"Greater\", \"Less or Equal\")");
    
    // 重新计算公式
    workbook.recalculate_formulas();
    
    // 验证结果
    ASSERT_EQ("Greater", sheet["B1"].as<std::string>());
    ASSERT_EQ("Less or Equal", sheet["B2"].as<std::string>());
}

TEST_CASE(FormulaEngine, ComplexFormulas) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置数据
    sheet["A1"].value(10);
    sheet["A2"].value(20);
    sheet["A3"].value(30);
    
    // 测试复杂公式
    sheet["B1"].formula("(A1 + A2) * A3");  // (10 + 20) * 30 = 900
    sheet["B2"].formula("SUM(A1:A3) / 3");  // (10+20+30) / 3 = 20
    sheet["B3"].formula("MAX(A1, A2) + MIN(A2, A3)");  // 20 + 20 = 40
    
    // 重新计算公式
    workbook.recalculate_formulas();
    
    // 验证结果
    ASSERT_EQ(900.0, sheet["B1"].as<double>());
    ASSERT_EQ(20.0, sheet["B2"].as<double>());
    ASSERT_EQ(40.0, sheet["B3"].as<double>());
}

TEST_CASE(FormulaEngine, StringConcatenation) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置数据
    sheet["A1"].value("Hello");
    sheet["A2"].value(" ");
    sheet["A3"].value("World");
    
    // 测试字符串连接
    sheet["B1"].formula("A1 & A2 & A3");  // "Hello World"
    
    // 重新计算公式
    workbook.recalculate_formulas();
    
    // 验证结果
    ASSERT_EQ("Hello World", sheet["B1"].as<std::string>());
}

TEST_CASE(FormulaEngine, ErrorHandling) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置数据
    sheet["A1"].value(10);
    sheet["A2"].value(0);
    
    // 测试除零错误
    sheet["B1"].formula("A1 / A2");
    
    // 重新计算公式
    workbook.recalculate_formulas();
    
    // 验证结果（应该包含错误信息）
    auto result = sheet["B1"].as<std::string>();
    ASSERT_TRUE(result.find("#ERROR") != std::string::npos);
}

TEST_CASE(FormulaEngine, InvalidFormulas) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 测试无效公式
    sheet["A1"].formula("INVALID_FUNCTION(1, 2)");
    sheet["A2"].formula("1 + * 2");  // 语法错误：连续操作符
    sheet["A3"].formula("SUM(");     // 不完整的公式
    
    // 重新计算公式
    workbook.recalculate_formulas();
    
    // 验证结果（都应该包含错误信息）
    auto result1 = sheet["A1"].as<std::string>();
    auto result2 = sheet["A2"].as<std::string>();
    auto result3 = sheet["A3"].as<std::string>();
    
    ASSERT_TRUE(result1.find("#ERROR") != std::string::npos);
    ASSERT_TRUE(result2.find("#ERROR") != std::string::npos);
    ASSERT_TRUE(result3.find("#ERROR") != std::string::npos);
}

TEST_CASE(FormulaEngine, CellReferences) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 设置数据
    sheet["A1"].value(100);
    sheet["B1"].value(200);
    
    // 测试单元格引用
    sheet["C1"].formula("A1");      // 直接引用
    sheet["C2"].formula("A1 + B1"); // 引用计算
    
    // 重新计算公式
    workbook.recalculate_formulas();
    
    // 验证结果
    ASSERT_EQ(100.0, sheet["C1"].as<double>());
    ASSERT_EQ(300.0, sheet["C2"].as<double>());
    
    // 修改原始值，测试依赖更新
    sheet["A1"].value(150);
    workbook.recalculate_formulas();
    
    ASSERT_EQ(150.0, sheet["C1"].as<double>());
    ASSERT_EQ(350.0, sheet["C2"].as<double>());
}
