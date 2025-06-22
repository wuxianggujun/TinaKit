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

        // 测试多种中文字体
        std::vector<std::string> chinese_fonts = {
            "SimSun",           // 宋体
            "NSimSun",          // 新宋体
            "Microsoft YaHei",  // 微软雅黑（带空格）
            "MicrosoftYaHei",   // 微软雅黑（无空格）
            "SimHei",           // 黑体
            "KaiTi"             // 楷体
        };

        pdf.add_text("Hello World", {100, 700}, english_font);

        float y_pos = 680;
        for (const auto& font_name : chinese_fonts) {
            pdf::Font chinese_font;
            chinese_font.family = font_name;
            chinese_font.size = 12;
            chinese_font.color = tinakit::Color::Black;

            std::string test_text = font_name + ": 你好世界";
            pdf.add_text(test_text, {100, y_pos}, chinese_font);
            y_pos -= 25;
        }

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
