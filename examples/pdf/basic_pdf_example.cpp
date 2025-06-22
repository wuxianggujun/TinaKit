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

        // 最简单的测试 - 只添加一行文本
        pdf::Font test_font;
        test_font.family = "Helvetica";  // PDF内建字体
        test_font.size = 12;
        test_font.color = tinakit::Color::Black;

        pdf.add_text("Hello World", {100, 700}, test_font);

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
