/**
 * @file excel_with_styles.cpp
 * @brief 使用样式创建格式化的 Excel 文档
 * @author TinaKit Team
 * @date 2025-01-08
 */

#include <tinakit/tinakit.hpp>
#include <iostream>
#include <format>
#include <vector>

using namespace tinakit;
using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== 创建带样式的 Excel 文档 ===" << std::endl;
        
        // 创建新工作簿
        auto workbook = Workbook::create();
        auto& sheet = workbook["Sheet1"];
        auto& style_manager = workbook.style_manager();
        auto& shared_strings = workbook.shared_strings();
        
        // 1. 创建标题样式
        Font title_font;
        title_font.name = "Arial";
        title_font.size = 16.0;
        title_font.bold = true;
        title_font.color = Color::from_hex("#FFFFFF");
        auto title_font_id = style_manager.add_font(title_font);
        
        Fill title_fill;
        title_fill.pattern_type = Fill::PatternType::Solid;
        title_fill.fg_color = Color::from_hex("#366092");  // 深蓝色背景
        auto title_fill_id = style_manager.add_fill(title_fill);
        
        CellStyle title_style;
        title_style.font_id = title_font_id;
        title_style.fill_id = title_fill_id;
        title_style.apply_font = true;
        title_style.apply_fill = true;
        auto title_style_id = style_manager.add_cell_style(title_style);
        
        // 2. 创建表头样式
        Font header_font;
        header_font.name = "Calibri";
        header_font.size = 12.0;
        header_font.bold = true;
        auto header_font_id = style_manager.add_font(header_font);
        
        Fill header_fill;
        header_fill.pattern_type = Fill::PatternType::Solid;
        header_fill.fg_color = Color::from_hex("#D9E2F3");  // 浅蓝色背景
        auto header_fill_id = style_manager.add_fill(header_fill);
        
        Border header_border;
        header_border.bottom.style = Border::Style::Thin;
        header_border.bottom.color = Color::from_hex("#000000");
        auto header_border_id = style_manager.add_border(header_border);
        
        Alignment header_align;
        header_align.horizontal = Alignment::Horizontal::Center;
        
        CellStyle header_style;
        header_style.font_id = header_font_id;
        header_style.fill_id = header_fill_id;
        header_style.border_id = header_border_id;
        header_style.alignment = header_align;
        header_style.apply_font = true;
        header_style.apply_fill = true;
        header_style.apply_border = true;
        header_style.apply_alignment = true;
        auto header_style_id = style_manager.add_cell_style(header_style);
        
        // 3. 创建数据样式
        Border data_border;
        data_border.left.style = Border::Style::Thin;
        data_border.right.style = Border::Style::Thin;
        data_border.top.style = Border::Style::Thin;
        data_border.bottom.style = Border::Style::Thin;
        data_border.left.color = Color::from_hex("#D0D0D0");
        data_border.right.color = Color::from_hex("#D0D0D0");
        data_border.top.color = Color::from_hex("#D0D0D0");
        data_border.bottom.color = Color::from_hex("#D0D0D0");
        auto data_border_id = style_manager.add_border(data_border);
        
        CellStyle data_style;
        data_style.border_id = data_border_id;
        data_style.apply_border = true;
        auto data_style_id = style_manager.add_cell_style(data_style);
        
        // 4. 创建金额样式（带数字格式）
        NumberFormat currency_format;
        currency_format.id = 164;  // 自定义格式 ID
        currency_format.format_code = "$#,##0.00";
        auto currency_format_id = style_manager.add_number_format(currency_format);
        
        CellStyle currency_style;
        currency_style.border_id = data_border_id;
        currency_style.number_format_id = currency_format_id;
        currency_style.apply_border = true;
        currency_style.apply_number_format = true;
        auto currency_style_id = style_manager.add_cell_style(currency_style);
        
        // 5. 填充数据
        // 标题
        sheet["A1"].value("销售报表 - 2025年1月").style_id(title_style_id);
        
        // 表头
        std::vector<std::string> headers = {"产品名称", "单价", "数量", "总额", "备注"};
        for (size_t i = 0; i < headers.size(); ++i) {
            sheet.cell(3, i + 1).value(headers[i]).style_id(header_style_id);
        }
        
        // 数据
        struct Product {
            std::string name;
            double price;
            int quantity;
            std::string note;
        };
        
        std::vector<Product> products = {
            {"TinaKit Pro 许可证", 299.99, 10, "企业版"},
            {"TinaKit Standard 许可证", 99.99, 25, "标准版"},
            {"技术支持服务", 499.99, 5, "年度合同"},
            {"培训课程", 1999.99, 2, "现场培训"}
        };
        
        int row = 4;
        for (const auto& product : products) {
            // 产品名称（使用共享字符串）
            auto name_idx = shared_strings.add_string(product.name);
            sheet.cell(row, 1).value(product.name).style_id(data_style_id);
            
            // 单价
            sheet.cell(row, 2).value(product.price).style_id(currency_style_id);
            
            // 数量
            sheet.cell(row, 3).value(product.quantity).style_id(data_style_id);
            
            // 总额（使用公式）
            sheet.cell(row, 4)
                .formula(std::format("=B{}*C{}", row, row))
                .style_id(currency_style_id);
            
            // 备注
            auto note_idx = shared_strings.add_string(product.note);
            sheet.cell(row, 5).value(product.note).style_id(data_style_id);
            
            row++;
        }
        
        // 总计行
        Font total_font;
        total_font.bold = true;
        auto total_font_id = style_manager.add_font(total_font);
        
        CellStyle total_style;
        total_style.font_id = total_font_id;
        total_style.border_id = data_border_id;
        total_style.apply_font = true;
        total_style.apply_border = true;
        auto total_style_id = style_manager.add_cell_style(total_style);
        
        sheet.cell(row, 3).value("总计：").style_id(total_style_id);
        sheet.cell(row, 4)
            .formula(std::format("=SUM(D4:D{})", row - 1))
            .style_id(currency_style_id);
        
        // 设置列宽（这个功能可能需要后续实现）
        // sheet.column(1).width(20);  // 产品名称列
        // sheet.column(2).width(12);  // 单价列
        // sheet.column(3).width(10);  // 数量列
        // sheet.column(4).width(12);  // 总额列
        // sheet.column(5).width(15);  // 备注列
        
        // 6. 保存文件
        const std::string filename = "sales_report_with_styles.xlsx";
        workbook.save(filename);
        
        std::cout << "\n文件已保存为: " << filename << std::endl;
        std::cout << "共享字符串数量: " << shared_strings.count() << std::endl;
        std::cout << "样式数量: " << style_manager.cell_style_count() << std::endl;
        
        std::cout << "\n=== 创建完成 ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}