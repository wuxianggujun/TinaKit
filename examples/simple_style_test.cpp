#include <iostream>
#include "tinakit/excel/workbook.hpp"

using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== 简单样式测试 ===" << std::endl;
        
        // 创建工作簿
        auto workbook = Workbook::create();
        auto& sheet = workbook.create_sheet("样式测试");
        
        // 测试1：只设置粗体
        std::cout << "设置A1为粗体..." << std::endl;
        auto& cell_a1 = sheet.cell("A1");
        cell_a1.value("粗体文本").bold();
        std::cout << "A1样式ID: " << cell_a1.style_id() << std::endl;

        // 测试2：普通文本
        std::cout << "设置A2为普通文本..." << std::endl;
        auto& cell_a2 = sheet.cell("A2");
        cell_a2.value("普通文本");
        std::cout << "A2样式ID: " << cell_a2.style_id() << std::endl;

        // 测试3：斜体
        std::cout << "设置A3为斜体..." << std::endl;
        auto& cell_a3 = sheet.cell("A3");
        cell_a3.value("斜体文本").italic();
        std::cout << "A3样式ID: " << cell_a3.style_id() << std::endl;
        
        // 保存文件
        const std::string filename = "simple_style_test.xlsx";
        workbook.save(filename);
        
        std::cout << "\n文件已保存为: " << filename << std::endl;
        std::cout << "请用Excel打开查看样式是否生效" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
