/**
 * @file workbook_test.cpp
 * @brief 测试 Workbook 类的读写功能
 * 
 * 展示如何使用 TinaKit 的 Workbook 类来：
 * 1. 创建新的 Excel 文件
 * 2. 添加和管理工作表
 * 3. 写入单元格数据
 * 4. 读取现有的 Excel 文件
 */

#include <iostream>
#include <filesystem>
#include <vector>
#include <tuple>

#include "tinakit/tinakit.hpp"

using namespace tinakit;
using namespace tinakit::excel;
using namespace tinakit::async;

// 测试创建新的 Excel 文件
Task<void> test_create_workbook() {
    std::cout << "\n=== 测试创建新的 Excel 文件 ===" << std::endl;
    
    try {
        // 创建新的工作簿
        auto workbook = Workbook::create();
        std::cout << "✓ 工作簿创建成功" << std::endl;
        
        // 获取默认工作表（应该有一个 Sheet1）
        auto& sheet1 = workbook.active_sheet();
        std::cout << "✓ 获取活动工作表: " << sheet1.name() << std::endl;
        
        // 写入数据到单元格
        sheet1["A1"].value("姓名");
        sheet1["B1"].value("年龄");
        sheet1["C1"].value("城市");
        
        sheet1["A2"].value("张三");
        sheet1["B2"].value(25);
        sheet1["C2"].value("北京");
        
        sheet1["A3"].value("李四");
        sheet1["B3"].value(30);
        sheet1["C3"].value("上海");
        
        std::cout << "✓ 数据写入成功" << std::endl;
        
        // 添加新的工作表
        auto& sheet2 = workbook.add_sheet("销售数据");
        std::cout << "✓ 添加新工作表: " << sheet2.name() << std::endl;
        
        // 在新工作表中写入数据
        sheet2["A1"].value("产品");
        sheet2["B1"].value("数量");
        sheet2["C1"].value("单价");
        sheet2["D1"].value("总价");
        
        // 批量写入数据
        std::vector<std::tuple<std::string, int, double>> sales_data = {
            {"iPhone 15", 10, 6999.0},
            {"iPad Pro", 5, 8999.0},
            {"MacBook Pro", 3, 15999.0}
        };
        
        int row = 2;
        for (const auto& [product, quantity, price] : sales_data) {
            sheet2.cell(row, 1).value(product);
            sheet2.cell(row, 2).value(quantity);
            sheet2.cell(row, 3).value(price);
            sheet2.cell(row, 4).value(quantity * price);  // 计算总价
            row++;
        }
        
        std::cout << "✓ 批量数据写入成功" << std::endl;
        
        // 保存文件
        workbook.save("test_workbook.xlsx");
        std::cout << "✓ 文件保存成功: test_workbook.xlsx" << std::endl;
        
        // 显示文件信息
        std::cout << "  - 工作表数量: " << workbook.sheet_count() << std::endl;
        
        // 获取文件大小
        if (std::filesystem::exists("test_workbook.xlsx")) {
            auto file_size = std::filesystem::file_size("test_workbook.xlsx");
            std::cout << "  - 文件大小: " << file_size << " 字节" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ 错误: " << e.what() << std::endl;
    }
    
    co_return;
}

// 测试读取 Excel 文件
Task<void> test_read_workbook() {
    std::cout << "\n=== 测试读取 Excel 文件 ===" << std::endl;
    
    try {
        // 确保文件路径是绝对路径
        auto file_path = std::filesystem::current_path() / "test_workbook.xlsx";
        
        // 检查文件是否存在
        if (!std::filesystem::exists(file_path)) {
            std::cout << "❌ 文件不存在，请先运行创建测试" << std::endl;
            co_return;
        }
        
        // 打开文件
        auto workbook = co_await Workbook::open_async(file_path);
        std::cout << "✓ 文件打开成功" << std::endl;
        
        // 显示工作表信息
        std::cout << "工作表数量: " << workbook.sheet_count() << std::endl;
        
        // 遍历所有工作表
        auto sheets = workbook.worksheets();
        for (const auto& sheet_ref : sheets) {
            const auto& sheet = sheet_ref.get();
            std::cout << "\n工作表: " << sheet.name() << std::endl;
            
            // 读取前几行数据
            std::cout << "内容预览:" << std::endl;
            for (std::size_t row = 1; row <= std::min(sheet.max_row(), std::size_t(5)); ++row) {
                std::cout << "  行 " << row << ": ";
                for (std::size_t col = 1; col <= std::min(sheet.max_column(), std::size_t(4)); ++col) {
                    try {
                        const auto& cell = sheet.cell(row, col);
                        std::cout << "[" << cell.to_string() << "] ";
                    } catch (...) {
                        std::cout << "[空] ";
                    }
                }
                std::cout << std::endl;
            }
        }
        
        // 访问特定工作表
        try {
            auto& sales_sheet = workbook["销售数据"];
            std::cout << "\n访问特定工作表: " << sales_sheet.name() << std::endl;
            
            // 计算总销售额
            double total_sales = 0;
            for (std::size_t row = 2; row <= sales_sheet.max_row(); ++row) {
                try {
                    auto value = sales_sheet.cell(row, 4).as<double>();
                    total_sales += value;
                } catch (...) {
                    // 忽略非数字单元格
                }
            }
            std::cout << "总销售额: " << total_sales << " 元" << std::endl;
            
        } catch (const WorksheetNotFoundException& e) {
            std::cout << "工作表未找到: " << e.what() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ 错误: " << e.what() << std::endl;
    }
    
    co_return;
}

// 测试修改现有文件
Task<void> test_modify_workbook() {
    std::cout << "\n=== 测试修改现有文件 ===" << std::endl;
    
    try {
        // 打开现有文件
        auto workbook = Workbook::open("test_workbook.xlsx");
        std::cout << "✓ 打开文件成功" << std::endl;
        
        // 添加新工作表
        auto& summary_sheet = workbook.add_sheet("汇总");
        std::cout << "✓ 添加汇总工作表" << std::endl;
        
        // 写入汇总信息
        summary_sheet["A1"].value("项目");
        summary_sheet["B1"].value("数值");
        
        summary_sheet["A2"].value("总工作表数");
        summary_sheet["B2"].value(static_cast<int>(workbook.sheet_count()));
        
        summary_sheet["A3"].value("创建时间");
        summary_sheet["B3"].value("2024-01-15");
        
        // 保存为新文件
        workbook.save_as("test_workbook_modified.xlsx");
        std::cout << "✓ 保存修改后的文件: test_workbook_modified.xlsx" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ 错误: " << e.what() << std::endl;
    }
    
    co_return;
}

// 测试错误处理
Task<void> test_error_handling() {
    std::cout << "\n=== 测试错误处理 ===" << std::endl;
    
    // 测试打开不存在的文件
    try {
        auto workbook = Workbook::open("nonexistent.xlsx");
    } catch (const FileNotFoundException& e) {
        std::cout << "✓ 正确捕获文件未找到异常: " << e.what() << std::endl;
    }
    
    // 测试访问不存在的工作表
    try {
        auto workbook = Workbook::create();
        auto& sheet = workbook["不存在的工作表"];
    } catch (const WorksheetNotFoundException& e) {
        std::cout << "✓ 正确捕获工作表未找到异常: " << e.what() << std::endl;
    }
    
    // 测试删除最后一个工作表
    try {
        auto workbook = Workbook::create();
        workbook.remove_sheet(0);  // 尝试删除唯一的工作表
    } catch (const CannotDeleteLastWorksheetException& e) {
        std::cout << "✓ 正确捕获不能删除最后工作表异常" << std::endl;
    }
    
    // 测试重复的工作表名称
    try {
        auto workbook = Workbook::create();
        workbook.add_sheet("重复名称");
        workbook.add_sheet("重复名称");  // 重复名称
    } catch (const DuplicateWorksheetNameException& e) {
        std::cout << "✓ 正确捕获重复工作表名称异常: " << e.what() << std::endl;
    }
    
    co_return;
}

// 主函数
Task<void> run_all_tests() {
    std::cout << "TinaKit Workbook 测试" << std::endl;
    std::cout << "====================" << std::endl;
    
    // 运行所有测试
    co_await test_create_workbook();
    co_await test_read_workbook();
    co_await test_modify_workbook();
    co_await test_error_handling();
    
    std::cout << "\n✅ 所有测试完成!" << std::endl;
    
    // 清理测试文件（可选）
    std::cout << "\n清理测试文件..." << std::endl;
    try {
        if (std::filesystem::exists("test_workbook.xlsx")) {
            std::filesystem::remove("test_workbook.xlsx");
            std::cout << "  - 删除 test_workbook.xlsx" << std::endl;
        }
        if (std::filesystem::exists("test_workbook_modified.xlsx")) {
            std::filesystem::remove("test_workbook_modified.xlsx");
            std::cout << "  - 删除 test_workbook_modified.xlsx" << std::endl;
        }
    } catch (...) {
        std::cout << "  - 清理文件时出错（可忽略）" << std::endl;
    }
}

int main() {
    try {
        auto task = run_all_tests();
        sync_wait(std::move(task));
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    }
}