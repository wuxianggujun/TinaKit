/**
 * @file tinakit_comprehensive_demo.cpp
 * @brief TinaKit 综合功能演示
 * @author TinaKit Team
 * @date 2025-01-08
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "tinakit/tinakit.hpp"

using namespace tinakit;

// 演示 XmlParser 的改进功能
void demo_xml_parser() {
    std::cout << "=== XmlParser 功能演示 ===\n";
    
    // 创建示例 XML
    std::string xml_data = R"(<?xml version="1.0" encoding="UTF-8"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
    <sheets>
        <sheet name="Sheet1" sheetId="1" r:id="rId1"/>
        <sheet name="Sheet2" sheetId="2" r:id="rId2"/>
    </sheets>
    <calcPr calcId="191029"/>
</workbook>)";
    
    std::istringstream stream(xml_data);
    core::XmlParser parser(stream, "workbook.xml");
    
    // 1. 使用传统的迭代器方式
    std::cout << "\n1. 传统迭代器方式：\n";
    for (auto it = parser.begin(); it != parser.end(); ++it) {
        if (it.is_start_element()) {
            std::cout << "  开始元素: " << it.name();
            
            // 获取所有属性
            auto attrs = it.attributes();
            if (!attrs.empty()) {
                std::cout << " (属性: ";
                for (const auto& [key, value] : attrs) {
                    std::cout << key << "=\"" << value << "\" ";
                }
                std::cout << ")";
            }
            
            std::cout << " [行:" << it.line() << ", 列:" << it.column() << "]\n";
        }
    }
    
    // 2. 使用新的便利方法
    std::cout << "\n2. 使用便利方法 for_each_element:\n";
    stream.clear();
    stream.seekg(0);
    core::XmlParser parser2(stream, "workbook.xml");
    
    parser2.for_each_element("sheet", [](core::XmlParser::iterator& it) {
        std::cout << "  找到工作表: " 
                  << it.attribute("name").value_or("unnamed") 
                  << " (ID: " << it.attribute("sheetId").value_or("?") << ")\n";
    });
    
    // 3. 测试 text_content 方法
    std::string xml_with_text = R"(<?xml version="1.0"?>
<data>
    <item>第一个项目</item>
    <item>第二个项目</item>
    <nested>
        <sub>嵌套文本</sub>
    </nested>
</data>)";
    
    std::cout << "\n3. 测试 text_content 方法:\n";
    std::istringstream text_stream(xml_with_text);
    core::XmlParser parser3(text_stream, "data.xml");
    
    for (auto it = parser3.begin(); it != parser3.end(); ++it) {
        if (it.is_start_element() && it.name() == "item") {
            std::cout << "  项目内容: " << it.text_content() << "\n";
        }
    }
}

// 演示样式管理器
void demo_style_manager() {
    std::cout << "\n\n=== StyleManager 功能演示 ===\n";
    
    excel::StyleManager style_mgr;
    
    // 1. 创建不同的字体
    excel::Font header_font;
    header_font.name = "Arial";
    header_font.size = 14;
    header_font.bold = true;
    header_font.color = core::Color::from_hex("FF0000"); // 红色
    
    excel::Font normal_font;
    normal_font.name = "Calibri";
    normal_font.size = 11;
    
    auto header_font_id = style_mgr.add_font(header_font);
    auto normal_font_id = style_mgr.add_font(normal_font);
    
    std::cout << "创建了 " << style_mgr.font_count() << " 个字体\n";
    
    // 2. 创建填充样式
    excel::Fill solid_blue;
    solid_blue.pattern_type = excel::Fill::PatternType::Solid;
    solid_blue.fg_color = core::Color::from_hex("4472C4");
    
    excel::Fill gradient_fill;
    gradient_fill.pattern_type = excel::Fill::PatternType::Gray125;
    
    auto blue_fill_id = style_mgr.add_fill(solid_blue);
    auto gradient_fill_id = style_mgr.add_fill(gradient_fill);
    
    std::cout << "创建了 " << style_mgr.fill_count() << " 个填充样式\n";
    
    // 3. 创建边框
    excel::Border thin_border;
    thin_border.left.style = excel::Border::Style::Thin;
    thin_border.right.style = excel::Border::Style::Thin;
    thin_border.top.style = excel::Border::Style::Thin;
    thin_border.bottom.style = excel::Border::Style::Thin;
    thin_border.left.color = core::Color::from_hex("000000");
    thin_border.right.color = core::Color::from_hex("000000");
    thin_border.top.color = core::Color::from_hex("000000");
    thin_border.bottom.color = core::Color::from_hex("000000");
    
    auto border_id = style_mgr.add_border(thin_border);
    
    // 4. 创建单元格样式
    excel::CellStyle header_style;
    header_style.font_id = header_font_id;
    header_style.fill_id = blue_fill_id;
    header_style.border_id = border_id;
    header_style.alignment = excel::Alignment();
    header_style.alignment->horizontal = excel::Alignment::Horizontal::Center;
    header_style.alignment->vertical = excel::Alignment::Vertical::Center;
    
    auto header_style_id = style_mgr.add_cell_style(header_style);
    
    std::cout << "创建了标题样式，ID: " << header_style_id << "\n";
    
    // 生成样式 XML
    std::string styles_xml = style_mgr.generate_xml();
    std::cout << "\n生成的样式 XML 片段:\n";
    std::cout << styles_xml.substr(0, 500) << "...\n";
}

// 创建一个完整的 Excel 文件
void demo_create_excel() {
    std::cout << "\n\n=== 创建 Excel 文件演示 ===\n";
    
    // 创建工作簿
    excel::Workbook wb;
    
    // 添加工作表
    auto& ws1 = wb.add_worksheet("销售数据");
    auto& ws2 = wb.add_worksheet("汇总");
    
    // 获取样式管理器和共享字符串
    auto& style_mgr = wb.style_manager();
    auto& shared_strings = wb.shared_strings();
    
    // 创建标题样式
    excel::Font title_font;
    title_font.name = "微软雅黑";
    title_font.size = 16;
    title_font.bold = true;
    title_font.color = core::Color::from_hex("FFFFFF");
    
    excel::Fill title_fill;
    title_fill.pattern_type = excel::Fill::PatternType::Solid;
    title_fill.fg_color = core::Color::from_hex("366092");
    
    excel::CellStyle title_style;
    title_style.font_id = style_mgr.add_font(title_font);
    title_style.fill_id = style_mgr.add_fill(title_fill);
    title_style.alignment = excel::Alignment();
    title_style.alignment->horizontal = excel::Alignment::Horizontal::Center;
    title_style.alignment->vertical = excel::Alignment::Vertical::Center;
    
    auto title_style_id = style_mgr.add_cell_style(title_style);
    
    // 创建表头样式
    excel::Font header_font;
    header_font.name = "微软雅黑";
    header_font.size = 12;
    header_font.bold = true;
    
    excel::Fill header_fill;
    header_fill.pattern_type = excel::Fill::PatternType::Solid;
    header_fill.fg_color = core::Color::from_hex("D9E2F3");
    
    excel::Border header_border;
    header_border.bottom.style = excel::Border::Style::Medium;
    header_border.bottom.color = core::Color::from_hex("366092");
    
    excel::CellStyle header_style;
    header_style.font_id = style_mgr.add_font(header_font);
    header_style.fill_id = style_mgr.add_fill(header_fill);
    header_style.border_id = style_mgr.add_border(header_border);
    header_style.alignment = excel::Alignment();
    header_style.alignment->horizontal = excel::Alignment::Horizontal::Center;
    
    auto header_style_id = style_mgr.add_cell_style(header_style);
    
    // 创建数据样式
    excel::Border data_border;
    data_border.left.style = excel::Border::Style::Thin;
    data_border.right.style = excel::Border::Style::Thin;
    data_border.top.style = excel::Border::Style::Thin;
    data_border.bottom.style = excel::Border::Style::Thin;
    
    excel::CellStyle data_style;
    data_style.border_id = style_mgr.add_border(data_border);
    
    auto data_style_id = style_mgr.add_cell_style(data_style);
    
    // 创建货币格式样式
    excel::NumberFormat currency_format;
    currency_format.id = 164; // 自定义格式ID
    currency_format.format_code = R"(￥#,##0.00)";
    
    excel::CellStyle currency_style = data_style;
    currency_style.number_format_id = style_mgr.add_number_format(currency_format);
    currency_style.apply_number_format = true;
    
    auto currency_style_id = style_mgr.add_cell_style(currency_style);
    
    // 填充数据
    // 标题
    ws1.cell("A1").set_value("2024年销售数据报表");
    ws1.cell("A1").set_style_id(title_style_id);
    ws1.merge_cells("A1:E1");
    
    // 表头
    ws1.cell("A3").set_value("产品");
    ws1.cell("B3").set_value("数量");
    ws1.cell("C3").set_value("单价");
    ws1.cell("D3").set_value("总价");
    ws1.cell("E3").set_value("备注");
    
    for (int col = 1; col <= 5; ++col) {
        ws1.cell(3, col).set_style_id(header_style_id);
    }
    
    // 数据
    const std::vector<std::tuple<std::string, int, double, std::string>> data = {
        {"笔记本电脑", 50, 4999.00, "热销产品"},
        {"无线鼠标", 200, 99.90, "库存充足"},
        {"机械键盘", 150, 399.00, "新品上市"},
        {"显示器", 80, 1299.00, "促销中"},
        {"USB集线器", 300, 59.90, "办公必备"}
    };
    
    int row = 4;
    for (const auto& [product, quantity, price, note] : data) {
        ws1.cell(row, 1).set_value(product);
        ws1.cell(row, 1).set_style_id(data_style_id);
        
        ws1.cell(row, 2).set_value(quantity);
        ws1.cell(row, 2).set_style_id(data_style_id);
        
        ws1.cell(row, 3).set_value(price);
        ws1.cell(row, 3).set_style_id(currency_style_id);
        
        // 使用公式计算总价
        std::string formula = "B" + std::to_string(row) + "*C" + std::to_string(row);
        ws1.cell(row, 4).set_formula(formula);
        ws1.cell(row, 4).set_style_id(currency_style_id);
        
        ws1.cell(row, 5).set_value(note);
        ws1.cell(row, 5).set_style_id(data_style_id);
        
        row++;
    }
    
    // 添加合计行
    ws1.cell(row, 1).set_value("合计");
    ws1.cell(row, 1).set_style_id(header_style_id);
    
    ws1.cell(row, 2).set_formula("SUM(B4:B" + std::to_string(row-1) + ")");
    ws1.cell(row, 2).set_style_id(header_style_id);
    
    ws1.cell(row, 4).set_formula("SUM(D4:D" + std::to_string(row-1) + ")");
    ws1.cell(row, 4).set_style_id(header_style_id);
    
    // 设置列宽
    ws1.set_column_width(1, 15.0);  // 产品列
    ws1.set_column_width(2, 10.0);  // 数量列
    ws1.set_column_width(3, 12.0);  // 单价列
    ws1.set_column_width(4, 12.0);  // 总价列
    ws1.set_column_width(5, 20.0);  // 备注列
    
    // 保存文件
    wb.save("tinakit_comprehensive_demo.xlsx");
    
    std::cout << "已创建 Excel 文件: tinakit_comprehensive_demo.xlsx\n";
    std::cout << "文件包含:\n";
    std::cout << "  - " << wb.worksheet_count() << " 个工作表\n";
    std::cout << "  - " << shared_strings.count() << " 个共享字符串\n";
    std::cout << "  - " << style_mgr.font_count() << " 个字体样式\n";
    std::cout << "  - " << style_mgr.fill_count() << " 个填充样式\n";
    std::cout << "  - " << style_mgr.border_count() << " 个边框样式\n";
    std::cout << "  - " << style_mgr.cell_style_count() << " 个单元格样式\n";
}

int main() {
    try {
        std::cout << "TinaKit 综合功能演示\n";
        std::cout << "=====================\n\n";
        
        // 1. 演示改进的 XmlParser
        demo_xml_parser();
        
        // 2. 演示样式管理器
        demo_style_manager();
        
        // 3. 创建完整的 Excel 文件
        demo_create_excel();
        
        std::cout << "\n演示完成！\n";
    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 