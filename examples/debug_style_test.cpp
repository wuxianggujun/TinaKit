#include <iostream>
#include <tinakit/tinakit.hpp>

using namespace tinakit;

int main() {
    try {
        std::cout << "=== 样式解析测试 ===" << std::endl;
        
        // 1. 创建一个简单的Excel文件
        auto workbook = excel::create();
        auto sheet = workbook.active_sheet();
        sheet.set_name("样式测试");
        
        // 添加一些带样式的数据
        sheet["A1"].value("标题").bold();
        sheet["A2"].value(0.85).number_format("0.00%");
        
        workbook.save("style_test.xlsx");
        std::cout << "✅ 文件创建完成" << std::endl;
        
        // 2. 立即读取这个文件
        auto read_workbook = excel::open("style_test.xlsx");
        auto read_sheet = read_workbook.active_sheet();
        
        std::cout << "\n=== 读取结果 ===" << std::endl;
        
        // 检查A1单元格
        auto& a1 = read_sheet["A1"];
        std::cout << "A1 值: " << a1.to_string() << std::endl;
        std::cout << "A1 样式ID: " << a1.style_id() << std::endl;
        
        // 检查A2单元格
        auto& a2 = read_sheet["A2"];
        std::cout << "A2 值: " << a2.to_string() << std::endl;
        std::cout << "A2 样式ID: " << a2.style_id() << std::endl;
        
        // 简单检查样式是否生效
        std::cout << "\n=== 样式检查 ===" << std::endl;

        // 检查A1是否有样式
        if (a1.style_id() > 0) {
            std::cout << "✅ A1有样式ID: " << a1.style_id() << std::endl;
        } else {
            std::cout << "❌ A1没有样式" << std::endl;
        }

        // 检查A2是否有样式
        if (a2.style_id() > 0) {
            std::cout << "✅ A2有样式ID: " << a2.style_id() << std::endl;
        } else {
            std::cout << "❌ A2没有样式" << std::endl;
        }

        // 尝试再次保存，看看样式是否保持
        read_workbook.save("style_test_resaved.xlsx");
        std::cout << "✅ 重新保存完成" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
