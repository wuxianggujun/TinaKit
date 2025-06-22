/**
 * @file basic_pdf_example.cpp
 * @brief PDF基础功能示例
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include <iostream>
#include "tinakit/tinakit.hpp"

using namespace tinakit;

int main() {
    try {
        std::cout << "🚀 TinaKit PDF 基础示例" << std::endl;
        std::cout << "========================" << std::endl;

        // ========================================
        // 1. 创建PDF文档
        // ========================================
        std::cout << "📄 创建PDF文档..." << std::endl;
        
        auto pdf = pdf::Document::create();
        
        // 设置文档信息
        pdf::DocumentInfo info;
        info.title = "TinaKit PDF 示例";
        info.author = "TinaKit Team";
        info.subject = "PDF功能演示";
        info.keywords = "PDF, TinaKit, C++";
        
        pdf.set_document_info(info);
        pdf.set_page_size(pdf::PageSize::A4);
        
        // ========================================
        // 2. 添加第一页
        // ========================================
        std::cout << "📝 添加内容..." << std::endl;
        
        pdf.add_page();
        
        // 添加标题
        pdf::Font title_font;
        title_font.family = "Arial";
        title_font.size = 24;
        title_font.bold = true;
        title_font.color = Color::Blue;
        
        pdf.add_text("TinaKit PDF 功能演示", {100, 750}, title_font);
        
        // 添加正文
        pdf::Font body_font;
        body_font.family = "Arial";
        body_font.size = 12;
        body_font.color = Color::Black;
        
        pdf.add_text("这是一个使用TinaKit库创建的PDF文档。", {100, 700}, body_font);
        pdf.add_text("TinaKit支持从Excel数据生成PDF报表。", {100, 680}, body_font);
        
        // ========================================
        // 3. 创建Excel数据并导入PDF
        // ========================================
        std::cout << "📊 创建Excel数据..." << std::endl;
        
        // 创建Excel工作簿
        auto workbook = excel::Workbook::create();
        auto sheet = workbook.active_sheet();
        sheet.set_name("销售数据");
        
        // 添加表头
        auto header_style = excel::Style()
            .font("微软雅黑", 12)
            .bold()
            .color(Color::White)
            .background_color(Color::Blue)
            .align_horizontal(excel::Alignment::Horizontal::Center);
        
        sheet["A1"].value("产品名称").style(header_style);
        sheet["B1"].value("销售数量").style(header_style);
        sheet["C1"].value("单价").style(header_style);
        sheet["D1"].value("总金额").style(header_style);
        
        // 添加数据
        auto data_style = excel::Style()
            .font("微软雅黑", 10)
            .align_horizontal(excel::Alignment::Horizontal::Center);
        
        sheet["A2"].value("笔记本电脑").style(data_style);
        sheet["B2"].value(10).style(data_style);
        sheet["C2"].value(5999.99).style(data_style);
        sheet["D2"].value(59999.90).style(data_style);
        
        sheet["A3"].value("台式机").style(data_style);
        sheet["B3"].value(5).style(data_style);
        sheet["C3"].value(3999.99).style(data_style);
        sheet["D3"].value(19999.95).style(data_style);
        
        sheet["A4"].value("显示器").style(data_style);
        sheet["B4"].value(15).style(data_style);
        sheet["C4"].value(1299.99).style(data_style);
        sheet["D4"].value(19499.85).style(data_style);
        
        // 保存Excel文件
        workbook.save("sales_data.xlsx");
        std::cout << "   ✅ Excel文件已保存: sales_data.xlsx" << std::endl;
        
        // ========================================
        // 4. 将Excel表格添加到PDF
        // ========================================
        std::cout << "📋 将Excel数据导入PDF..." << std::endl;
        
        // 添加表格标题
        pdf.add_text("销售数据表", {100, 600}, title_font);
        
        // 从Excel导入表格到PDF
        pdf.add_excel_table(sheet, "A1:D4", {100, 450}, true);
        
        // ========================================
        // 5. 添加更多内容
        // ========================================
        
        // 添加总结
        pdf.add_text("数据总结:", {100, 350}, body_font);
        pdf.add_text("• 总销售数量: 30 件", {120, 330}, body_font);
        pdf.add_text("• 总销售金额: ¥99,499.70", {120, 310}, body_font);
        pdf.add_text("• 平均单价: ¥3,766.66", {120, 290}, body_font);
        
        // 添加页脚
        pdf::Font footer_font;
        footer_font.family = "Arial";
        footer_font.size = 10;
        footer_font.color = Color::Black;
        
        pdf.add_text("由 TinaKit PDF 库生成", {100, 50}, footer_font);
        pdf.add_text("生成时间: 2025-06-22", {400, 50}, footer_font);
        
        // ========================================
        // 6. 保存PDF文档
        // ========================================
        std::cout << "💾 保存PDF文档..." << std::endl;
        
        pdf.save("tinakit_pdf_example.pdf");
        
        std::cout << "   ✅ PDF文件已保存: tinakit_pdf_example.pdf" << std::endl;
        
        // ========================================
        // 7. 显示结果
        // ========================================
        std::cout << "\n🎉 PDF示例完成!" << std::endl;
        std::cout << "📁 生成的文件:" << std::endl;
        std::cout << "   • sales_data.xlsx (Excel源数据)" << std::endl;
        std::cout << "   • tinakit_pdf_example.pdf (PDF报表)" << std::endl;
        std::cout << "\n📊 功能演示:" << std::endl;
        std::cout << "   ✅ PDF文档创建" << std::endl;
        std::cout << "   ✅ 文本添加和格式化" << std::endl;
        std::cout << "   ✅ Excel数据导入" << std::endl;
        std::cout << "   ✅ 表格生成" << std::endl;
        std::cout << "   ✅ 样式保留" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
}
