/**
 * @file style_chain_demo.cpp
 * @brief 链式样式调用演示
 * @author TinaKit Team
 * @date 2025-01-08
 */

#include <tinakit/tinakit.hpp>
#include <iostream>

using namespace tinakit;
using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== TinaKit 链式样式调用演示 ===" << std::endl;
        
        // 创建工作簿
        auto workbook = Workbook::create();
        auto& sheet = workbook.add_sheet("样式演示");
        
        // 演示链式调用设置样式
        sheet["A1"]
            .value("TinaKit Excel Library")
            .font("Arial", 16)
            .bold()
            .italic()
            .color(Color::from_hex("#FF0000"))  // 红色字体
            .background_color(Color::from_hex("#FFFFCC"));  // 淡黄色背景
        
        // 设置不同的字体样式
        sheet["A3"]
            .value("Bold Text")
            .font("Calibri", 12)
            .bold();
        
        sheet["A4"]
            .value("Italic Text")
            .font("Times New Roman", 12)
            .italic();
        
        sheet["A5"]
            .value("Colored Text")
            .font("Verdana", 12)
            .color(Color::Blue);
        
        // 背景色示例
        sheet["C3"]
            .value("Green Background")
            .background_color(Color::from_hex("#90EE90"));
        
        sheet["C4"]
            .value("Pink Background")
            .background_color(Color::from_hex("#FFB6C1"));
        
        // 边框示例
        sheet["E3"]
            .value("All Borders")
            .border(BorderType::All, BorderStyle::Thin);
        
        sheet["E4"]
            .value("Thick Bottom")
            .border(BorderType::Bottom, BorderStyle::Thick);
        
        // 数字格式示例
        sheet["A7"].value("数字格式:").bold();
        
        sheet["A8"].value(1234.567);
        sheet["B8"]
            .value(1234.567)
            .number_format("#,##0.00");  // 千分位，2位小数
        
        sheet["A9"].value(0.1234);
        sheet["B9"]
            .value(0.1234)
            .number_format("0.00%");  // 百分比格式
        
        sheet["A10"].value(999.99);
        sheet["B10"]
            .value(999.99)
            .number_format("¥#,##0.00");  // 货币格式
        
        // 组合样式示例
        sheet["A12"]
            .value("综合样式示例")
            .font("微软雅黑", 14)
            .bold()
            .color(Color::White)
            .background_color(Color::from_hex("#4472C4"))
            .border(BorderType::All, BorderStyle::Medium);
        
        // 保存文件
        const std::string filename = "style_chain_demo.xlsx";
        workbook.save(filename);
        
        std::cout << "\n文件已保存为: " << filename << std::endl;
        std::cout << "\n=== 演示完成 ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}