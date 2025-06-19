#include <iostream>
#include "tinakit/excel/workbook.hpp"

using namespace tinakit;
using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== TinaKit 综合样式功能测试 ===" << std::endl;
        
        // 创建工作簿
        auto workbook = Workbook::create();
        
        // ========================================
        // 1. 字体和颜色测试
        // ========================================
        std::cout << "创建字体和颜色测试工作表..." << std::endl;
        auto& font_sheet = workbook.create_sheet("字体和颜色");
        
        // 标题
        font_sheet["A1"]
            .value("字体和颜色测试")
            .font("微软雅黑", 16)
            .bold()
            .color(Color::Blue)
            .background_color(Color::LightGray);
        
        // 不同字体测试
        font_sheet["A3"].value("Arial 字体").font("Arial", 12);
        font_sheet["A4"].value("Times New Roman 字体").font("Times New Roman", 12);
        font_sheet["A5"].value("Calibri 字体").font("Calibri", 12);
        font_sheet["A6"].value("微软雅黑字体").font("微软雅黑", 12);
        
        // 字体样式测试
        font_sheet["C3"].value("普通文本");
        font_sheet["C4"].value("粗体文本").bold();
        font_sheet["C5"].value("斜体文本").italic();
        font_sheet["C6"].value("粗体+斜体").bold().italic();
        
        // 颜色测试
        font_sheet["E3"].value("红色文字").color(Color::Red);
        font_sheet["E4"].value("绿色文字").color(Color::Green);
        font_sheet["E5"].value("蓝色文字").color(Color::Blue);
        font_sheet["E6"].value("自定义颜色").color(Color::from_hex("#FF6600"));
        
        // 背景色测试
        font_sheet["G3"].value("黄色背景").background_color(Color::Yellow);
        font_sheet["G4"].value("浅蓝背景").background_color(Color::LightBlue);
        font_sheet["G5"].value("浅绿背景").background_color(Color::LightGreen);
        font_sheet["G6"].value("浅红背景").background_color(Color::LightRed);
        
        // ========================================
        // 2. 对齐测试
        // ========================================
        std::cout << "创建对齐测试工作表..." << std::endl;
        auto& align_sheet = workbook.create_sheet("对齐测试");
        
        // 标题
        align_sheet["A1"]
            .value("对齐功能测试")
            .font("微软雅黑", 16)
            .bold()
            .color(Color::White)
            .background_color(Color::Blue);
        
        // 水平对齐测试
        align_sheet["A3"].value("水平对齐测试").bold();
        
        Alignment left_align;
        left_align.horizontal = Alignment::Horizontal::Left;
        left_align.vertical = Alignment::Vertical::Center;
        align_sheet["A4"].value("左对齐").align(left_align);
        
        Alignment center_align;
        center_align.horizontal = Alignment::Horizontal::Center;
        center_align.vertical = Alignment::Vertical::Center;
        align_sheet["B4"].value("居中对齐").align(center_align);
        
        Alignment right_align;
        right_align.horizontal = Alignment::Horizontal::Right;
        right_align.vertical = Alignment::Vertical::Center;
        align_sheet["C4"].value("右对齐").align(right_align);
        
        // 垂直对齐测试
        align_sheet["A6"].value("垂直对齐测试").bold();
        
        Alignment top_align;
        top_align.horizontal = Alignment::Horizontal::Center;
        top_align.vertical = Alignment::Vertical::Top;
        align_sheet["A7"].value("顶部对齐").align(top_align);
        
        Alignment middle_align;
        middle_align.horizontal = Alignment::Horizontal::Center;
        middle_align.vertical = Alignment::Vertical::Center;
        align_sheet["B7"].value("中间对齐").align(middle_align);
        
        Alignment bottom_align;
        bottom_align.horizontal = Alignment::Horizontal::Center;
        bottom_align.vertical = Alignment::Vertical::Bottom;
        align_sheet["C7"].value("底部对齐").align(bottom_align);
        
        // ========================================
        // 3. 数字格式测试
        // ========================================
        std::cout << "创建数字格式测试工作表..." << std::endl;
        auto& number_sheet = workbook.create_sheet("数字格式");
        
        // 标题
        number_sheet["A1"]
            .value("数字格式测试")
            .font("微软雅黑", 16)
            .bold()
            .background_color(Color::LightBlue);
        
        // 基础数字格式
        number_sheet["A3"].value("格式类型").bold();
        number_sheet["B3"].value("原始值").bold();
        number_sheet["C3"].value("格式化结果").bold();
        
        // 整数格式
        number_sheet["A4"].value("整数");
        number_sheet["B4"].value(12345);
        number_sheet["C4"].value(12345).number_format("0");
        
        // 小数格式
        number_sheet["A5"].value("两位小数");
        number_sheet["B5"].value(12345.678);
        number_sheet["C5"].value(12345.678).number_format("0.00");
        
        // 千分位格式
        number_sheet["A6"].value("千分位");
        number_sheet["B6"].value(1234567);
        number_sheet["C6"].value(1234567).number_format("#,##0");
        
        // 百分比格式
        number_sheet["A7"].value("百分比");
        number_sheet["B7"].value(0.1234);
        number_sheet["C7"].value(0.1234).number_format("0.00%");
        
        // 货币格式
        number_sheet["A8"].value("货币");
        number_sheet["B8"].value(1234.56);
        number_sheet["C8"].value(1234.56).number_format("¥#,##0.00");
        
        // ========================================
        // 4. 边框测试
        // ========================================
        std::cout << "创建边框测试工作表..." << std::endl;
        auto& border_sheet = workbook.create_sheet("边框测试");
        
        // 标题
        border_sheet["A1"]
            .value("边框测试")
            .font("微软雅黑", 16)
            .bold()
            .background_color(Color::LightGreen);
        
        // 不同边框样式测试
        border_sheet["A3"].value("细边框").border(BorderType::All, BorderStyle::Thin);
        border_sheet["B3"].value("粗边框").border(BorderType::All, BorderStyle::Thick);
        border_sheet["C3"].value("虚线边框").border(BorderType::All, BorderStyle::Dashed);
        border_sheet["D3"].value("点线边框").border(BorderType::All, BorderStyle::Dotted);
        
        // 单边框测试
        border_sheet["A5"].value("上边框").border(BorderType::Top, BorderStyle::Thin);
        border_sheet["B5"].value("下边框").border(BorderType::Bottom, BorderStyle::Thin);
        border_sheet["C5"].value("左边框").border(BorderType::Left, BorderStyle::Thin);
        border_sheet["D5"].value("右边框").border(BorderType::Right, BorderStyle::Thin);
        
        // ========================================
        // 5. 综合样式测试
        // ========================================
        std::cout << "创建综合样式测试工作表..." << std::endl;
        auto& combo_sheet = workbook.create_sheet("综合样式");
        
        // 标题
        combo_sheet["A1"]
            .value("综合样式演示")
            .font("微软雅黑", 18)
            .bold()
            .color(Color::White)
            .background_color(Color::Blue)
            .align(center_align)
            .border(BorderType::All, BorderStyle::Thick);
        
        // 复杂样式组合
        combo_sheet["A3"]
            .value("销售报表")
            .font("Arial", 14)
            .bold()
            .italic()
            .color(Color::Red)
            .background_color(Color::Yellow)
            .align(center_align)
            .border(BorderType::All, BorderStyle::Thin);
        
        combo_sheet["A5"]
            .value(98765.43)
            .font("Times New Roman", 12)
            .color(Color::Green)
            .number_format("¥#,##0.00")
            .align(right_align)
            .border(BorderType::Bottom, BorderStyle::Double);
        
        // 保存文件
        const std::string filename = "comprehensive_style_test.xlsx";
        workbook.save(filename);
        
        std::cout << "\n✅ 综合样式测试完成！" << std::endl;
        std::cout << "📁 文件已保存为: " << filename << std::endl;
        std::cout << "📋 包含以下测试工作表:" << std::endl;
        std::cout << "   • 字体和颜色 - 字体、颜色、背景色测试" << std::endl;
        std::cout << "   • 对齐测试 - 水平和垂直对齐测试" << std::endl;
        std::cout << "   • 数字格式 - 各种数字格式测试" << std::endl;
        std::cout << "   • 边框测试 - 边框样式和类型测试" << std::endl;
        std::cout << "   • 综合样式 - 复杂样式组合测试" << std::endl;
        std::cout << "\n请用Excel/WPS打开查看所有样式效果！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
