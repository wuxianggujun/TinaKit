#include <iostream>
#include "tinakit/excel/workbook.hpp"

using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== 对齐功能测试 ===" << std::endl;
        
        // 创建工作簿
        auto workbook = Workbook::create();
        auto& sheet = workbook.create_sheet("对齐测试");
        
        // 测试水平对齐（明确设置垂直居中）
        sheet["A1"].value("左对齐+垂直居中");
        Alignment left_align;
        left_align.horizontal = Alignment::Horizontal::Left;
        left_align.vertical = Alignment::Vertical::Center;  // 明确设置垂直居中
        sheet["A1"].align(left_align);

        sheet["B1"].value("水平居中+垂直居中");
        Alignment center_align;
        center_align.horizontal = Alignment::Horizontal::Center;
        center_align.vertical = Alignment::Vertical::Center;  // 明确设置垂直居中
        sheet["B1"].align(center_align);

        sheet["C1"].value("右对齐+垂直居中");
        Alignment right_align;
        right_align.horizontal = Alignment::Horizontal::Right;
        right_align.vertical = Alignment::Vertical::Center;  // 明确设置垂直居中
        sheet["C1"].align(right_align);
        
        // 测试垂直对齐
        sheet["A3"].value("顶部对齐");
        Alignment top_align;
        top_align.vertical = Alignment::Vertical::Top;
        sheet["A3"].align(top_align);
        
        sheet["B3"].value("中间对齐");
        Alignment middle_align;
        middle_align.vertical = Alignment::Vertical::Center;
        sheet["B3"].align(middle_align);
        
        sheet["C3"].value("底部对齐");
        Alignment bottom_align;
        bottom_align.vertical = Alignment::Vertical::Bottom;
        sheet["C3"].align(bottom_align);
        
        // 保存文件
        const std::string filename = "alignment_test.xlsx";
        workbook.save(filename);
        
        std::cout << "\n文件已保存为: " << filename << std::endl;
        std::cout << "请用Excel打开查看对齐效果" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
