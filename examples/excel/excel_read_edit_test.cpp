/**
 * @file excel_read_edit_test.cpp
 * @brief Excel文件读取、编辑和保存的完整测试
 * 
 * 这个程序演示如何：
 * 1. 读取现有的Excel文件
 * 2. 获取单元格内容和样式信息
 * 3. 编辑单元格数据
 * 4. 保存修改后的文件
 */

#include <tinakit/tinakit.hpp>
#include <iostream>
#include <filesystem>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace tinakit::excel;
using namespace tinakit;

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << " " << title << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

void print_cell_info(const excel::Cell& cell) {
    std::cout << "  地址: " << cell.address() 
              << " | 行: " << cell.row() 
              << " | 列: " << cell.column() << std::endl;
    
    if (!cell.empty()) {
        const auto& raw_value = cell.raw_value();
        std::cout << "  原始值: ";
        
        if (std::holds_alternative<std::string>(raw_value)) {
            std::cout << "\"" << std::get<std::string>(raw_value) << "\" (字符串)";
        } else if (std::holds_alternative<double>(raw_value)) {
            std::cout << std::get<double>(raw_value) << " (浮点数)";
        } else if (std::holds_alternative<int>(raw_value)) {
            std::cout << std::get<int>(raw_value) << " (整数)";
        } else if (std::holds_alternative<bool>(raw_value)) {
            std::cout << (std::get<bool>(raw_value) ? "true" : "false") << " (布尔值)";
        }
        
        std::cout << std::endl;
        std::cout << "  字符串表示: \"" << cell.to_string() << "\"" << std::endl;
        std::cout << "  样式ID: " << cell.style_id() << std::endl;
    } else {
        std::cout << "  (空单元格)" << std::endl;
    }
}

void test_read_existing_file() {
    print_separator("测试读取现有Excel文件");
    
    // 首先创建一个测试文件
    std::cout << "1. 创建测试文件..." << std::endl;
    try {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        sheet.set_name("测试数据");
        
        // 添加一些测试数据
        sheet["A1"].value("姓名").bold();
        sheet["B1"].value("年龄").bold();
        sheet["C1"].value("工资").bold();
        sheet["D1"].value("完成率").bold();
        
        sheet["A2"].value("张三");
        sheet["B2"].value(25);
        sheet["C2"].value(5000.50);
        sheet["D2"].value(0.85).number_format("0.00%");
        
        sheet["A3"].value("李四");
        sheet["B3"].value(30);
        sheet["C3"].value(7500.75);
        sheet["D3"].value(0.92).number_format("0.00%");
        
        workbook.save("test_data.xlsx");
        std::cout << "   ✅ 测试文件创建成功: test_data.xlsx" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   ❌ 创建测试文件失败: " << e.what() << std::endl;
        return;
    }
    
    // 现在读取文件
    std::cout << "\n2. 读取测试文件..." << std::endl;
    try {
        auto workbook = Workbook::load("test_data.xlsx");
        std::cout << "   ✅ 文件读取成功" << std::endl;
        
        std::cout << "   工作表数量: " << workbook.worksheet_count() << std::endl;

        auto sheet = workbook.active_sheet();
        std::cout << "   活动工作表: \"" << sheet.name() << "\"" << std::endl;
        std::cout << "   最大行数: " << sheet.max_row() << std::endl;
        std::cout << "   最大列数: " << sheet.max_column() << std::endl;
        
        // 读取单元格数据
        std::cout << "\n3. 读取单元格数据:" << std::endl;
        
        // 读取标题行
        std::cout << "\n标题行 (第1行):" << std::endl;
        for (std::size_t col = 1; col <= 4; ++col) {
            auto cell = sheet.cell(1, col);
            std::cout << "  " << cell.address() << ": ";
            print_cell_info(cell);
        }
        
        // 读取数据行
        for (std::size_t row = 2; row <= 3; ++row) {
            std::cout << "\n数据行 (第" << row << "行):" << std::endl;
            for (std::size_t col = 1; col <= 4; ++col) {
                auto cell = sheet.cell(row, col);
                std::cout << "  " << cell.address() << ": ";
                print_cell_info(cell);
            }
        }
        
    } catch (const std::exception& e) {
        std::cout << "   ❌ 读取文件失败: " << e.what() << std::endl;
    }
}

void test_edit_existing_file() {
    print_separator("测试编辑现有Excel文件");
    
    try {
        std::cout << "1. 打开现有文件..." << std::endl;
        auto workbook = Workbook::load("test_data.xlsx");
        auto sheet = workbook.active_sheet();
        std::cout << "   ✅ 文件打开成功" << std::endl;
        
        std::cout << "\n2. 编辑数据..." << std::endl;
        
        // 修改现有数据
        sheet["B2"].value(26);  // 修改张三的年龄
        sheet["C3"].value(8000.00);  // 修改李四的工资
        
        // 添加新数据
        sheet["A4"].value("王五");
        sheet["B4"].value(28);
        sheet["C4"].value(6500.25);
        sheet["D4"].value(0.78).number_format("0.00%");
        
        // 添加一些样式
        sheet["A4"].bold().color(Color::Blue);
        sheet["C4"].number_format("¥#,##0.00");
        
        // 添加总计行
        sheet["A5"].value("总计").bold();
        sheet["B5"].background_color(Color::LightGray);   // ✅ 优化的API：直接设置背景色
        sheet["C5"].value(19500.75).bold().number_format("¥#,##0.00");
        sheet["D5"].background_color(Color::LightGray);   // ✅ 优化的API：直接设置背景色

        // 测试空单元格背景色问题
        std::cout << "   测试空单元格背景色..." << std::endl;

        // 第7行：测试9个连续单元格的绿色背景（模拟您的场景）
        for (int col = 1; col <= 9; ++col) {
            std::string cell_addr = std::string(1, static_cast<char>('A' + col - 1)) + "7";
            if (col % 3 == 1) {
                // 每3个单元格中的第1个有内容
                sheet[cell_addr].value("数据" + std::to_string(col)).background_color(Color::Green);
            } else {
                // 其他单元格为空但设置背景色
                sheet[cell_addr].value("").background_color(Color::Green);
            }
        }

        // 第8行：同样的测试，但所有单元格都有空格
        for (int col = 1; col <= 9; ++col) {
            std::string cell_addr = std::string(1, static_cast<char>('A' + col - 1)) + "8";
            if (col % 3 == 1) {
                sheet[cell_addr].value("数据" + std::to_string(col)).background_color(Color::Blue);
            } else {
                // 空单元格用空格填充
                sheet[cell_addr].value(" ").background_color(Color::Blue);
            }
        }

        // 添加更多颜色测试
        sheet["A6"].value("颜色测试").background_color(Color::Green);
        sheet["B6"].value("蓝色背景").background_color(Color::Blue);
        sheet["C6"].value("紫色背景").background_color(Color(128, 0, 128)); // 紫色
        
        std::cout << "   ✅ 数据编辑完成" << std::endl;
        
        std::cout << "\n3. 保存修改后的文件..." << std::endl;
        workbook.save("test_data_modified.xlsx");
        std::cout << "   ✅ 文件保存成功: test_data_modified.xlsx" << std::endl;
        
        // 验证修改
        std::cout << "\n4. 验证修改结果..." << std::endl;
        auto verify_workbook = Workbook::load("test_data_modified.xlsx");
        auto verify_sheet = verify_workbook.active_sheet();
        
        std::cout << "   修改后的数据:" << std::endl;
        std::cout << "   B2 (张三年龄): " << verify_sheet["B2"].to_string() << std::endl;
        std::cout << "   C3 (李四工资): " << verify_sheet["C3"].to_string() << std::endl;
        std::cout << "   A4 (新员工): " << verify_sheet["A4"].to_string() << std::endl;
        std::cout << "   C5 (总计): " << verify_sheet["C5"].to_string() << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "   ❌ 编辑文件失败: " << e.what() << std::endl;
    }
}

void test_cell_operations() {
    print_separator("测试单元格操作");
    
    try {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        sheet.set_name("单元格操作测试");
        
        std::cout << "1. 测试不同类型的值..." << std::endl;
        
        // 字符串
        sheet["A1"].value("文本内容");
        std::cout << "   A1 字符串: " << sheet["A1"].as<std::string>() << std::endl;
        
        // 整数
        sheet["A2"].value(42);
        std::cout << "   A2 整数: " << sheet["A2"].as<int>() << std::endl;
        
        // 浮点数
        sheet["A3"].value(3.14159);
        std::cout << "   A3 浮点数: " << sheet["A3"].as<double>() << std::endl;
        
        // 布尔值
        sheet["A4"].value(true);
        std::cout << "   A4 布尔值: " << (sheet["A4"].as<bool>() ? "true" : "false") << std::endl;
        
        std::cout << "\n2. 测试类型转换..." << std::endl;
        
        // 尝试安全转换
        auto int_result = sheet["A2"].try_as<int>();
        if (int_result) {
            std::cout << "   A2 安全转换为int: " << *int_result << std::endl;
        }
        
        auto str_result = sheet["A3"].try_as<std::string>();
        if (str_result) {
            std::cout << "   A3 转换为string: " << *str_result << std::endl;
        }
        
        workbook.save("cell_operations_test.xlsx");
        std::cout << "\n   ✅ 单元格操作测试完成" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "   ❌ 单元格操作测试失败: " << e.what() << std::endl;
    }
}

int main() {
    // 设置控制台UTF-8编码（Windows）
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    #endif

    std::cout << "TinaKit Excel 读取编辑测试程序" << std::endl;
    std::cout << "================================" << std::endl;
    
    try {
        // 测试1: 读取现有文件
        test_read_existing_file();
        
        // 测试2: 编辑现有文件
        test_edit_existing_file();
        
        // 测试3: 单元格操作
        test_cell_operations();
        
        print_separator("测试总结");
        std::cout << "✅ 所有测试完成！" << std::endl;
        std::cout << "\n生成的文件:" << std::endl;
        std::cout << "  - test_data.xlsx (原始测试数据)" << std::endl;
        std::cout << "  - test_data_modified.xlsx (修改后的数据)" << std::endl;
        std::cout << "  - cell_operations_test.xlsx (单元格操作测试)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 程序异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
