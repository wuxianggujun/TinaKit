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
        // 初始化日志系统 - 设置为DEBUG级别以查看详细信息
        tinakit::core::initializeDefaultLogging(tinakit::core::LogLevel::DEBUG);

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

        // 测试中英混合文本处理
        std::cout << "\n📋 中英混合文本处理演示:" << std::endl;
        std::cout << "1. 智能字体选择" << std::endl;
        std::cout << "2. 自动分段渲染" << std::endl;
        std::cout << "3. 字体回退机制" << std::endl;

        // 1. 使用英文字体显示混合文本（会自动回退到中文字体）
        pdf::Font english_font;
        english_font.family = "Helvetica";
        english_font.size = 12;
        english_font.color = tinakit::Color::Black;

        pdf.add_text("Mixed Text: Hello 世界! Welcome to TinaKit!", {100, 700}, english_font);

        // 2. 使用中文字体显示混合文本
        pdf::Font chinese_font;
        chinese_font.family = "SimSun";
        chinese_font.size = 12;
        chinese_font.color = tinakit::Color::Blue;
        pdf.add_text("混合文本: 使用 C++20 开发 PDF library", {100, 680}, chinese_font);

        // 3. 技术术语混合
        pdf::Font tech_font;
        tech_font.family = "SimSun";
        tech_font.size = 11;
        tech_font.color = tinakit::Color(0, 128, 0); // Dark Green
        pdf.add_text("技术演示: TinaKit supports OpenXML (.xlsx, .docx) 和 PDF 生成", {100, 660}, tech_font);

        // 4. 数字和符号混合
        pdf.add_text("价格信息: $99.99 USD = ¥688.00 CNY (优惠 20%)", {100, 640}, tech_font);

        // 功能说明
        pdf::Font info_font;
        info_font.family = "SimSun";
        info_font.size = 10;
        info_font.color = tinakit::Color(128, 128, 128); // Gray
        pdf.add_text("✨ 新功能: 智能中英文混合文本处理", {100, 600}, info_font);
        pdf.add_text("• 自动字体回退: 英文字体 → 中文字体 (CJK字符)", {120, 580}, info_font);
        pdf.add_text("• 智能分段: ASCII字符和Unicode字符分别优化渲染", {120, 560}, info_font);
        pdf.add_text("• 无乱码显示: 解决英文字符在中文字体中的显示问题", {120, 540}, info_font);

        // ========================================
        // 3. 保存PDF（跳过Excel部分）
        // ========================================
        // 3. 保存PDF文档
        // ========================================
        std::cout << "💾 保存PDF文档..." << std::endl;
        
        pdf.save("tinakit_pdf_example.pdf");
        
        std::cout << "   ✅ PDF文件已保存: tinakit_pdf_example.pdf" << std::endl;
        
        // ========================================
        // 7. 显示结果
        // ========================================
        std::cout << "\n🎉 PDF测试完成!" << std::endl;
        std::cout << "📁 生成的文件:" << std::endl;
        std::cout << "   • tinakit_pdf_example.pdf (简单PDF测试)" << std::endl;
        std::cout << "\n📊 功能演示:" << std::endl;
        std::cout << "   ✅ PDF文档创建" << std::endl;
        std::cout << "   ✅ 基本文本显示" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
}
