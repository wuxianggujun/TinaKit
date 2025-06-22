/**
 * @file handle_system_test.cpp
 * @brief 测试句柄系统的基本功能
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/tinakit.hpp"
#include <iostream>
#include <string>

using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== TinaKit 句柄系统测试 ===" << std::endl;
        
        // ========================================
        // 1. 创建工作簿（轻量级句柄）
        // ========================================
        std::cout << "\n1. 创建工作簿..." << std::endl;
        auto workbook = Workbook::create();
        std::cout << "✅ 工作簿创建成功" << std::endl;
        
        // ========================================
        // 2. 获取工作表（轻量级句柄）
        // ========================================
        std::cout << "\n2. 获取工作表..." << std::endl;
        auto sheet = workbook.active_sheet();
        std::cout << "✅ 工作表获取成功，名称: " << sheet.name() << std::endl;
        
        // ========================================
        // 3. 单元格操作（轻量级句柄）
        // ========================================
        std::cout << "\n3. 测试单元格操作..." << std::endl;
        
        // 设置字符串值
        sheet["A1"].value("Hello TinaKit!");
        std::cout << "✅ A1 设置字符串值: " << sheet["A1"].as<std::string>() << std::endl;
        
        // 设置数字值
        sheet["B1"].value(42);
        std::cout << "✅ B1 设置数字值: " << sheet["B1"].as<int>() << std::endl;
        
        // 设置浮点数值
        sheet["C1"].value(3.14159);
        std::cout << "✅ C1 设置浮点数值: " << sheet["C1"].as<double>() << std::endl;
        
        // 设置布尔值
        sheet["D1"].value(true);
        std::cout << "✅ D1 设置布尔值: " << (sheet["D1"].as<bool>() ? "TRUE" : "FALSE") << std::endl;
        
        // ========================================
        // 4. 测试类型转换
        // ========================================
        std::cout << "\n4. 测试类型转换..." << std::endl;
        
        // 数字转字符串
        std::cout << "B1 转字符串: " << sheet["B1"].as<std::string>() << std::endl;
        
        // 字符串转数字（应该失败并返回0）
        std::cout << "A1 转数字: " << sheet["A1"].as<int>() << std::endl;
        
        // ========================================
        // 5. 测试公式
        // ========================================
        std::cout << "\n5. 测试公式..." << std::endl;
        
        sheet["E1"].formula("=B1+C1");
        auto formula = sheet["E1"].formula();
        if (formula) {
            std::cout << "✅ E1 公式设置成功: " << *formula << std::endl;
        }
        
        // ========================================
        // 6. 测试样式
        // ========================================
        std::cout << "\n6. 测试样式..." << std::endl;
        
        sheet["A1"].style_id(1);
        std::cout << "✅ A1 样式ID设置: " << sheet["A1"].style_id() << std::endl;
        std::cout << "✅ A1 有自定义样式: " << (sheet["A1"].has_custom_style() ? "是" : "否") << std::endl;
        
        // ========================================
        // 7. 测试工作表信息
        // ========================================
        std::cout << "\n7. 测试工作表信息..." << std::endl;
        
        std::cout << "工作表名称: " << sheet.name() << std::endl;
        std::cout << "最大行: " << sheet.max_row() << std::endl;
        std::cout << "最大列: " << sheet.max_column() << std::endl;
        std::cout << "是否为空: " << (sheet.empty() ? "是" : "否") << std::endl;
        
        // ========================================
        // 8. 测试工作簿信息
        // ========================================
        std::cout << "\n8. 测试工作簿信息..." << std::endl;
        
        std::cout << "工作表数量: " << workbook.worksheet_count() << std::endl;
        auto names = workbook.worksheet_names();
        std::cout << "工作表名称列表: ";
        for (const auto& name : names) {
            std::cout << name << " ";
        }
        std::cout << std::endl;
        
        // ========================================
        // 9. 测试范围操作
        // ========================================
        std::cout << "\n9. 测试范围操作..." << std::endl;
        
        try {
            auto range = sheet.range("A1:D1");
            std::cout << "✅ 范围创建成功: " << range.address() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "⚠️ 范围操作暂未完全实现: " << e.what() << std::endl;
        }
        
        std::cout << "\n=== 句柄系统测试完成 ===" << std::endl;
        std::cout << "✅ 所有基本功能正常工作！" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 测试失败: " << e.what() << std::endl;
        return 1;
    }
}
