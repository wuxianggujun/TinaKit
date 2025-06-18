/**
 * @file test_shared_strings_and_styles.cpp
 * @brief 测试共享字符串和样式功能
 * @author TinaKit Team
 * @date 2025-01-08
 */

#include <tinakit/tinakit.hpp>
#include <iostream>
#include <format>

using namespace tinakit;
using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== 测试共享字符串和样式功能 ===" << std::endl;
        
        // 创建新工作簿
        auto workbook = Workbook::create();
        auto& sheet = workbook.active_sheet();
        
        // 获取管理器
        auto& shared_strings = workbook.shared_strings();
        auto& style_manager = workbook.style_manager();
        
        // 1. 测试共享字符串
        std::cout << "\n1. 测试共享字符串功能：" << std::endl;
        
        // 添加一些字符串
        auto idx1 = shared_strings.add_string("Hello, World!");
        auto idx2 = shared_strings.add_string("TinaKit Excel");
        auto idx3 = shared_strings.add_string("Hello, World!");  // 重复字符串应该返回相同索引
        
        std::cout << "   - 字符串 'Hello, World!' 的索引: " << idx1 << std::endl;
        std::cout << "   - 字符串 'TinaKit Excel' 的索引: " << idx2 << std::endl;
        std::cout << "   - 重复字符串 'Hello, World!' 的索引: " << idx3 
                  << " (应该与第一个相同)" << std::endl;
        std::cout << "   - 共享字符串总数: " << shared_strings.count() << std::endl;
        
        // 验证获取字符串
        std::cout << "   - 获取索引 " << idx1 << " 的字符串: " 
                  << shared_strings.get_string(idx1) << std::endl;
        
        // 2. 测试样式功能
        std::cout << "\n2. 测试样式功能：" << std::endl;
        
        // 创建自定义字体
        Font custom_font;
        custom_font.name = "Arial";
        custom_font.size = 14.0;
        custom_font.bold = true;
        custom_font.color = Color::from_hex("#FF0000");  // 红色
        
        auto font_id = style_manager.add_font(custom_font);
        std::cout << "   - 添加自定义字体，ID: " << font_id << std::endl;
        
        // 创建填充样式
        Fill solid_fill;
        solid_fill.pattern_type = Fill::PatternType::Solid;
        solid_fill.fg_color = Color::from_hex("#FFFF00");  // 黄色背景
        
        auto fill_id = style_manager.add_fill(solid_fill);
        std::cout << "   - 添加填充样式，ID: " << fill_id << std::endl;
        
        // 创建边框样式
        Border custom_border;
        custom_border.left.style = Border::Style::Thin;
        custom_border.right.style = Border::Style::Thin;
        custom_border.top.style = Border::Style::Thin;
        custom_border.bottom.style = Border::Style::Thin;
        custom_border.left.color = Color::from_hex("#000000");  // 黑色边框
        custom_border.right.color = Color::from_hex("#000000");
        custom_border.right.color = Color::from_hex("#000000");
        custom_border.top.color = Color::from_hex("#000000");
        custom_border.bottom.color = Color::from_hex("#000000");
        
        auto border_id = style_manager.add_border(custom_border);
        std::cout << "   - 添加边框样式，ID: " << border_id << std::endl;
        
        // 创建对齐样式
        Alignment center_align;
        center_align.horizontal = Alignment::Horizontal::Center;
        center_align.vertical = Alignment::Vertical::Center;
        center_align.wrap_text = true;
        
        // 创建单元格样式
        CellStyle header_style;
        header_style.font_id = font_id;
        header_style.fill_id = fill_id;
        header_style.border_id = border_id;
        header_style.alignment = center_align;
        header_style.apply_font = true;
        header_style.apply_fill = true;
        header_style.apply_border = true;
        header_style.apply_alignment = true;
        
        auto style_id = style_manager.add_cell_style(header_style);
        std::cout << "   - 创建标题单元格样式，ID: " << style_id << std::endl;
        
        std::cout << "\n3. 样式统计：" << std::endl;
        std::cout << "   - 字体数量: " << style_manager.font_count() << std::endl;
        std::cout << "   - 填充样式数量: " << style_manager.fill_count() << std::endl;
        std::cout << "   - 边框样式数量: " << style_manager.border_count() << std::endl;
        std::cout << "   - 单元格样式数量: " << style_manager.cell_style_count() << std::endl;
        
        // 4. 生成 XML 预览
        std::cout << "\n4. 生成的共享字符串 XML 预览：" << std::endl;
        auto shared_strings_xml = shared_strings.generate_xml();
        std::cout << shared_strings_xml.substr(0, 300) << "..." << std::endl;
        
        std::cout << "\n5. 生成的样式 XML 预览：" << std::endl;
        auto styles_xml = style_manager.generate_xml();
        std::cout << styles_xml.substr(0, 300) << "..." << std::endl;
        
        // 5. 保存文件
        const std::string filename = "test_shared_strings_and_styles.xlsx";
        workbook.save(filename);
        std::cout << "\n6. 文件已保存为: " << filename << std::endl;
        
        // 6. 重新打开文件并验证
        std::cout << "\n7. 重新打开文件并验证：" << std::endl;
        auto workbook2 = Workbook::open(filename);
        auto& shared_strings2 = workbook2.shared_strings();
        auto& style_manager2 = workbook2.style_manager();
        
        std::cout << "   - 共享字符串数量: " << shared_strings2.count() << std::endl;
        std::cout << "   - 样式数量: " << style_manager2.cell_style_count() << std::endl;
        
        std::cout << "\n=== 测试完成 ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}