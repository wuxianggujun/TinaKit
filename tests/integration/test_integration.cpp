/**
 * @file test_integration.cpp
 * @brief TinaKit 集成测试 - 完整工作流程验证
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "test_framework.hpp"
#include "tinakit/tinakit.hpp"
#include "tinakit/excel/workbook.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/cell.hpp"
#include "tinakit/excel/range.hpp"
#include "tinakit/core/exceptions.hpp"
#include <filesystem>
#include <fstream>

using namespace tinakit;
using namespace tinakit::excel;

// ========================================
// 完整工作流程集成测试
// ========================================

TEST_CASE(Integration, CompleteWorkflow) {
    const std::string test_file = "test_complete_workflow.xlsx";
    
    // 清理可能存在的测试文件
    if (std::filesystem::exists(test_file)) {
        std::filesystem::remove(test_file);
    }
    
    try {
        // 步骤1：创建新工作簿
        auto workbook = Workbook::create();
        // 新工作簿开始时没有工作表，这是正确的行为
        ASSERT_EQ(0u, workbook.worksheet_count());
        
        // 步骤2：获取默认工作表（这会自动创建Sheet1）
        auto sheet = workbook.active_sheet();
        // 现在应该有1个工作表了
        ASSERT_EQ(1u, workbook.worksheet_count());
        
        // 步骤3：添加基础数据
        sheet.cell("A1").value("姓名");
        sheet.cell("B1").value("年龄");
        sheet.cell("C1").value("薪资");
        
        sheet.cell("A2").value("张三");
        sheet.cell("B2").value(25);
        sheet.cell("C2").value(5000.0);
        
        sheet.cell("A3").value("李四");
        sheet.cell("B3").value(30);
        sheet.cell("C3").value(6000.0);
        
        // 步骤4：保存文件
        workbook.save(test_file);
        
        // 验证文件是否创建成功
        ASSERT_TRUE(std::filesystem::exists(test_file));
        ASSERT_TRUE(std::filesystem::file_size(test_file) > 0);
        
    } catch (const std::exception& e) {
        // 清理测试文件
        if (std::filesystem::exists(test_file)) {
            std::filesystem::remove(test_file);
        }
        throw; // 重新抛出异常
    }
    
    // 清理测试文件
    if (std::filesystem::exists(test_file)) {
        std::filesystem::remove(test_file);
    }
}

TEST_CASE(Integration, CreateAndReadWorkflow) {
    const std::string test_file = "test_create_read.xlsx";
    
    // 清理可能存在的测试文件
    if (std::filesystem::exists(test_file)) {
        std::filesystem::remove(test_file);
    }
    
    try {
        // 步骤1：创建并保存工作簿
        {
            auto workbook = Workbook::create();
            auto sheet = workbook.active_sheet();
            
            // 添加测试数据
            sheet.cell("A1").value("测试标题");
            sheet.cell("A2").value("数据1");
            sheet.cell("B2").value(123);
            sheet.cell("C2").value(45.67);
            
            workbook.save(test_file);
        }
        
        // 验证文件创建
        ASSERT_TRUE(std::filesystem::exists(test_file));
        
        // 步骤2：暂时跳过文件读取验证
        // TODO: 等文件读取API完善后再添加验证
        // 目前只验证文件保存成功
        ASSERT_TRUE(std::filesystem::exists(test_file));
        
    } catch (const std::exception& e) {
        // 清理测试文件
        if (std::filesystem::exists(test_file)) {
            std::filesystem::remove(test_file);
        }
        throw; // 重新抛出异常
    }
    
    // 清理测试文件
    if (std::filesystem::exists(test_file)) {
        std::filesystem::remove(test_file);
    }
}

TEST_CASE(Integration, MultipleWorksheetsWorkflow) {
    const std::string test_file = "test_multiple_sheets.xlsx";
    
    // 清理可能存在的测试文件
    if (std::filesystem::exists(test_file)) {
        std::filesystem::remove(test_file);
    }
    
    try {
        // 创建包含多个工作表的工作簿
        auto workbook = Workbook::create();

        // 创建工作表
        workbook.create_worksheet("Sheet1");  // 手动创建第一个工作表
        workbook.create_worksheet("数据表");
        workbook.create_worksheet("统计表");

        ASSERT_EQ(3u, workbook.worksheet_count()); // Sheet1 + 数据表 + 统计表

        // 在不同工作表中添加数据
        auto sheet1 = workbook.get_worksheet("Sheet1");
        sheet1.cell("A1").value("第一个工作表");

        auto sheet2 = workbook.get_worksheet("数据表");
        sheet2.cell("A1").value("数据工作表");
        sheet2.cell("A2").value("数据1");
        sheet2.cell("A3").value("数据2");

        auto sheet3 = workbook.get_worksheet("统计表");
        sheet3.cell("A1").value("统计工作表");
        sheet3.cell("A2").value("总计");
        
        // 保存文件
        workbook.save(test_file);
        ASSERT_TRUE(std::filesystem::exists(test_file));
        
    } catch (const std::exception& e) {
        // 清理测试文件
        if (std::filesystem::exists(test_file)) {
            std::filesystem::remove(test_file);
        }
        throw;
    }
    
    // 清理测试文件
    if (std::filesystem::exists(test_file)) {
        std::filesystem::remove(test_file);
    }
}

// ========================================
// Range 批量操作测试
// ========================================

TEST_CASE(RangeBatchOperations, BasicRangeOperations) {
    const std::string test_file = "test_range_operations.xlsx";
    
    // 清理可能存在的测试文件
    if (std::filesystem::exists(test_file)) {
        std::filesystem::remove(test_file);
    }
    
    try {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        
        // 测试Range的基本操作
        auto range = sheet.range("A1:C3");
        
        // 批量设置相同值
        range.set_value(std::string("测试数据"));
        
        // 验证设置是否成功（通过检查单个单元格）
        auto cell_value = sheet.cell("A1").as<std::string>();
        ASSERT_EQ("测试数据", cell_value);

        auto cell_value2 = sheet.cell("C3").as<std::string>();
        ASSERT_EQ("测试数据", cell_value2);

        // 测试清除操作
        range.clear();

        // 验证清除是否成功
        auto cleared_value = sheet.cell("A1").as<std::string>();
        ASSERT_EQ("", cleared_value);
        
        workbook.save(test_file);
        ASSERT_TRUE(std::filesystem::exists(test_file));
        
    } catch (const std::exception& e) {
        // 清理测试文件
        if (std::filesystem::exists(test_file)) {
            std::filesystem::remove(test_file);
        }
        throw;
    }
    
    // 清理测试文件
    if (std::filesystem::exists(test_file)) {
        std::filesystem::remove(test_file);
    }
}

TEST_CASE(RangeBatchOperations, RangeWithDifferentDataTypes) {
    const std::string test_file = "test_range_datatypes.xlsx";
    
    // 清理可能存在的测试文件
    if (std::filesystem::exists(test_file)) {
        std::filesystem::remove(test_file);
    }
    
    try {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        
        // 测试不同数据类型的Range操作
        auto text_range = sheet.range("A1:A3");
        text_range.set_value(std::string("文本数据"));

        auto number_range = sheet.range("B1:B3");
        number_range.set_value(42);

        auto decimal_range = sheet.range("C1:C3");
        decimal_range.set_value(3.14);
        
        // 验证不同类型的数据
        ASSERT_EQ("文本数据", sheet.cell("A2").as<std::string>());
        ASSERT_EQ(42, sheet.cell("B2").as<int>());
        ASSERT_EQ(3.14, sheet.cell("C2").as<double>());
        
        workbook.save(test_file);
        ASSERT_TRUE(std::filesystem::exists(test_file));
        
    } catch (const std::exception& e) {
        // 清理测试文件
        if (std::filesystem::exists(test_file)) {
            std::filesystem::remove(test_file);
        }
        throw;
    }
    
    // 清理测试文件
    if (std::filesystem::exists(test_file)) {
        std::filesystem::remove(test_file);
    }
}
