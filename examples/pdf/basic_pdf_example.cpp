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

        // 测试英文和中文
        pdf::Font english_font;
        english_font.family = "Helvetica";  // 英文使用标准字体
        english_font.size = 12;
        english_font.color = tinakit::Color::Black;

        // 测试不同的字体选项
        std::cout << "\n📋 字体选项演示:" << std::endl;
        std::cout << "1. 标准字体（无嵌入）" << std::endl;
        std::cout << "2. 中文字体（自动嵌入）" << std::endl;
        std::cout << "3. 中文字体（禁用嵌入）" << std::endl;

        // 1. 标准英文字体（无嵌入）
        pdf.add_text("Hello World (Helvetica - No Embedding)", {100, 700}, english_font);

        // 2. 中文字体（自动嵌入）
        pdf::Font embedded_font;
        embedded_font.family = "SimSun";
        embedded_font.size = 12;
        embedded_font.color = tinakit::Color::Blue;
        pdf.add_text("你好世界！(SimSun - 自动嵌入)", {100, 680}, embedded_font);

        // 3. 中文字体（禁用嵌入）- 需要修改API来支持
        pdf::Font reference_font;
        reference_font.family = "SimSun";
        reference_font.size = 12;
        reference_font.color = tinakit::Color::Red;
        pdf.add_text("你好世界！(SimSun - 仅引用)", {100, 660}, reference_font);

        // 文件大小提示
        pdf::Font info_font;
        info_font.family = "Helvetica";
        info_font.size = 10;
        info_font.color = tinakit::Color::Black;
        pdf.add_text("注意：嵌入字体会增加文件大小（约18MB）", {100, 620}, info_font);
        pdf.add_text("但可确保在任何设备上正确显示", {100, 600}, info_font);

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
