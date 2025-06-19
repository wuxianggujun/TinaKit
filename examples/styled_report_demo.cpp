/**
 * @file styled_report_demo.cpp
 * @brief 带样式的报表演示
 * @author TinaKit Team
 * @date 2025-01-08
 */

#include <tinakit/tinakit.hpp>
#include <iostream>
#include <iomanip>
#include <format>
#include <vector>
#include <chrono>

using namespace tinakit;
using namespace tinakit::excel;

// 销售数据结构
struct SalesData {
    std::string region;
    std::string product;
    int units_sold;
    double unit_price;
    double total_revenue;
    double profit_margin;
    std::string status;
};

int main() {
    try {
        std::cout << "=== TinaKit 专业报表演示 ===" << std::endl;
        
        // 创建工作簿
        auto workbook = Workbook::create();
        auto& sheet = workbook.add_sheet("销售报表");
        auto& style_manager = workbook.style_manager();
        
        // ========== 定义样式 ==========
        
        // 1. 标题样式
        Font title_font;
        title_font.name = "微软雅黑";
        title_font.size = 20.0;
        title_font.bold = true;
        title_font.color = Color::White;
        auto title_font_id = style_manager.add_font(title_font);
        
        Fill title_fill;
        title_fill.pattern_type = Fill::PatternType::Solid;
        title_fill.fg_color = Color::from_hex("#1F4E78");  // 深蓝色
        auto title_fill_id = style_manager.add_fill(title_fill);
        
        Alignment title_alignment;
        title_alignment.horizontal = Alignment::Horizontal::Center;
        title_alignment.vertical = Alignment::Vertical::Center;
        
        CellStyle title_style;
        title_style.font_id = title_font_id;
        title_style.fill_id = title_fill_id;
        title_style.alignment = title_alignment;
        title_style.apply_font = true;
        title_style.apply_fill = true;
        title_style.apply_alignment = true;
        auto title_style_id = style_manager.add_cell_style(title_style);
        
        // 2. 副标题样式
        Font subtitle_font;
        subtitle_font.name = "微软雅黑";
        subtitle_font.size = 12.0;
        subtitle_font.italic = true;
        subtitle_font.color = Color::from_hex("#666666");
        auto subtitle_font_id = style_manager.add_font(subtitle_font);
        
        CellStyle subtitle_style;
        subtitle_style.font_id = subtitle_font_id;
        subtitle_style.apply_font = true;
        auto subtitle_style_id = style_manager.add_cell_style(subtitle_style);
        
        // 3. 表头样式
        Font header_font;
        header_font.name = "Arial";
        header_font.size = 11.0;
        header_font.bold = true;
        header_font.color = Color::White;
        auto header_font_id = style_manager.add_font(header_font);
        
        Fill header_fill;
        header_fill.pattern_type = Fill::PatternType::Solid;
        header_fill.fg_color = Color::from_hex("#366092");
        auto header_fill_id = style_manager.add_fill(header_fill);
        
        Border header_border;
        header_border.bottom.style = Border::Style::Medium;
        header_border.bottom.color = Color::from_hex("#1F4E78");
        auto header_border_id = style_manager.add_border(header_border);
        
        Alignment header_alignment;
        header_alignment.horizontal = Alignment::Horizontal::Center;
        header_alignment.vertical = Alignment::Vertical::Center;
        header_alignment.wrap_text = true;
        
        CellStyle header_style;
        header_style.font_id = header_font_id;
        header_style.fill_id = header_fill_id;
        header_style.border_id = header_border_id;
        header_style.alignment = header_alignment;
        header_style.apply_font = true;
        header_style.apply_fill = true;
        header_style.apply_border = true;
        header_style.apply_alignment = true;
        auto header_style_id = style_manager.add_cell_style(header_style);
        
        // 4. 数据单元格样式
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
        
        // 5. 货币格式样式
        NumberFormat currency_format;
        currency_format.id = 164;
        currency_format.format_code = "¥#,##0.00";
        auto currency_format_id = style_manager.add_number_format(currency_format);
        
        CellStyle currency_style;
        currency_style.border_id = data_border_id;
        currency_style.number_format_id = currency_format_id;
        currency_style.apply_border = true;
        currency_style.apply_number_format = true;
        auto currency_style_id = style_manager.add_cell_style(currency_style);
        
        // 6. 百分比格式样式
        NumberFormat percent_format;
        percent_format.id = 165;
        percent_format.format_code = "0.00%";
        auto percent_format_id = style_manager.add_number_format(percent_format);
        
        CellStyle percent_style;
        percent_style.border_id = data_border_id;
        percent_style.number_format_id = percent_format_id;
        percent_style.apply_border = true;
        percent_style.apply_number_format = true;
        auto percent_style_id = style_manager.add_cell_style(percent_style);
        
        // 7. 状态样式（条件格式）
        // 良好状态 - 绿色
        Fill good_fill;
        good_fill.pattern_type = Fill::PatternType::Solid;
        good_fill.fg_color = Color::from_hex("#C6EFCE");
        auto good_fill_id = style_manager.add_fill(good_fill);
        
        Font good_font;
        good_font.color = Color::from_hex("#006100");
        auto good_font_id = style_manager.add_font(good_font);
        
        CellStyle good_style;
        good_style.fill_id = good_fill_id;
        good_style.font_id = good_font_id;
        good_style.border_id = data_border_id;
        good_style.apply_fill = true;
        good_style.apply_font = true;
        good_style.apply_border = true;
        auto good_style_id = style_manager.add_cell_style(good_style);
        
        // 警告状态 - 黄色
        Fill warning_fill;
        warning_fill.pattern_type = Fill::PatternType::Solid;
        warning_fill.fg_color = Color::from_hex("#FFEB9C");
        auto warning_fill_id = style_manager.add_fill(warning_fill);
        
        Font warning_font;
        warning_font.color = Color::from_hex("#9C5700");
        auto warning_font_id = style_manager.add_font(warning_font);
        
        CellStyle warning_style;
        warning_style.fill_id = warning_fill_id;
        warning_style.font_id = warning_font_id;
        warning_style.border_id = data_border_id;
        warning_style.apply_fill = true;
        warning_style.apply_font = true;
        warning_style.apply_border = true;
        auto warning_style_id = style_manager.add_cell_style(warning_style);
        
        // 8. 总计行样式
        Font total_font;
        total_font.bold = true;
        total_font.size = 12.0;
        auto total_font_id = style_manager.add_font(total_font);
        
        Fill total_fill;
        total_fill.pattern_type = Fill::PatternType::Solid;
        total_fill.fg_color = Color::from_hex("#E7E6E6");
        auto total_fill_id = style_manager.add_fill(total_fill);
        
        Border total_border;
        total_border.top.style = Border::Style::Double;
        total_border.top.color = Color::Black;
        total_border.bottom.style = Border::Style::Double;
        total_border.bottom.color = Color::Black;
        auto total_border_id = style_manager.add_border(total_border);
        
        CellStyle total_style;
        total_style.font_id = total_font_id;
        total_style.fill_id = total_fill_id;
        total_style.border_id = total_border_id;
        total_style.apply_font = true;
        total_style.apply_fill = true;
        total_style.apply_border = true;
        auto total_style_id = style_manager.add_cell_style(total_style);
        
        // ========== 填充数据 ==========
        
        // 标题行
        sheet["A1"].value("2025年第一季度销售报表").style_id(title_style_id);
        // TODO: 合并单元格 A1:G1
        
        // 副标题
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &time_t);
#else
        tm = *std::localtime(&time_t);
#endif
        sheet["A2"].value(std::format("生成时间：{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
                         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                         tm.tm_hour, tm.tm_min, tm.tm_sec))
               .style_id(subtitle_style_id);
        
        // 表头
        std::vector<std::string> headers = {
            "地区", "产品", "销售数量", "单价", "销售额", "利润率", "状态"
        };
        
        int col = 1;
        for (const auto& header : headers) {
            sheet.cell(4, col++).value(header).style_id(header_style_id);
        }
        
        // 示例数据
        std::vector<SalesData> sales_data = {
            {"华东", "TinaKit Pro", 150, 299.99, 44998.50, 0.35, "优秀"},
            {"华东", "TinaKit Standard", 280, 99.99, 27997.20, 0.28, "良好"},
            {"华南", "TinaKit Pro", 120, 299.99, 35998.80, 0.33, "良好"},
            {"华南", "TinaKit Standard", 200, 99.99, 19998.00, 0.25, "一般"},
            {"华北", "TinaKit Pro", 180, 299.99, 53998.20, 0.38, "优秀"},
            {"华北", "TinaKit Standard", 350, 99.99, 34996.50, 0.30, "良好"},
            {"西南", "TinaKit Pro", 80, 299.99, 23999.20, 0.22, "一般"},
            {"西南", "TinaKit Standard", 150, 99.99, 14998.50, 0.20, "警告"}
        };
        
        // 填充数据
        int row = 5;
        for (const auto& data : sales_data) {
            sheet.cell(row, 1).value(data.region).style_id(data_style_id);
            sheet.cell(row, 2).value(data.product).style_id(data_style_id);
            sheet.cell(row, 3).value(data.units_sold).style_id(data_style_id);
            sheet.cell(row, 4).value(data.unit_price).style_id(currency_style_id);
            sheet.cell(row, 5).value(data.total_revenue).style_id(currency_style_id);
            sheet.cell(row, 6).value(data.profit_margin).style_id(percent_style_id);
            
            // 根据状态设置不同的样式
            if (data.status == "优秀") {
                sheet.cell(row, 7).value(data.status).style_id(good_style_id);
            } else if (data.status == "警告" || data.status == "一般") {
                sheet.cell(row, 7).value(data.status).style_id(warning_style_id);
            } else {
                sheet.cell(row, 7).value(data.status).style_id(data_style_id);
            }
            
            row++;
        }
        
        // 总计行
        sheet.cell(row, 1).value("总计").style_id(total_style_id);
        sheet.cell(row, 2).value("").style_id(total_style_id);
        sheet.cell(row, 3).formula(std::format("=SUM(C5:C{})", row - 1)).style_id(total_style_id);
        sheet.cell(row, 4).value("").style_id(total_style_id);
        sheet.cell(row, 5).formula(std::format("=SUM(E5:E{})", row - 1)).style_id(total_style_id);
        sheet.cell(row, 6).formula(std::format("=AVERAGE(F5:F{})", row - 1)).style_id(total_style_id);
        sheet.cell(row, 7).value("").style_id(total_style_id);
        
        // 设置列宽（未来实现）
        // sheet.column("A").width(12);  // 地区
        // sheet.column("B").width(20);  // 产品
        // sheet.column("C").width(12);  // 销售数量
        // sheet.column("D").width(10);  // 单价
        // sheet.column("E").width(12);  // 销售额
        // sheet.column("F").width(10);  // 利润率
        // sheet.column("G").width(10);  // 状态
        
        // 保存文件
        const std::string filename = "styled_sales_report.xlsx";
        workbook.save(filename);
        
        std::cout << "\n报表生成成功！" << std::endl;
        std::cout << "文件名: " << filename << std::endl;
        std::cout << "样式数量: " << style_manager.cell_style_count() << std::endl;
        std::cout << "字体数量: " << style_manager.font_count() << std::endl;
        std::cout << "填充数量: " << style_manager.fill_count() << std::endl;
        std::cout << "边框数量: " << style_manager.border_count() << std::endl;
        
        std::cout << "\n=== 演示完成 ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
