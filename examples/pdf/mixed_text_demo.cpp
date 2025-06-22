/**
 * @file mixed_text_demo.cpp
 * @brief 中英混合文本处理演示
 * @author TinaKit Team
 * @date 2025-6-22
 * 
 * 演示TinaKit PDF模块对中英混合文本的智能处理能力，
 * 包括自动字体切换和分段渲染功能。
 */

#include <iostream>
#include "tinakit/tinakit.hpp"
#include "tinakit/core/logger.hpp"

using namespace tinakit;

int main() {
    try {
        // 初始化日志系统
        core::Logger::initialize();
        
        std::cout << "🌏 TinaKit PDF 中英混合文本处理演示" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        // ========================================
        // 1. 创建PDF文档
        // ========================================
        std::cout << "📄 创建PDF文档..." << std::endl;
        
        pdf::Document pdf;
        pdf.add_page();
        
        // ========================================
        // 2. 测试不同类型的混合文本
        // ========================================
        std::cout << "✍️ 添加混合文本内容..." << std::endl;
        
        // 标题
        pdf::Font title_font;
        title_font.family = "SimSun";
        title_font.size = 18;
        title_font.color = tinakit::Color::Blue;
        
        pdf.add_text("TinaKit 中英混合文本处理演示", {100, 750}, title_font);
        
        // 测试用例1：简单中英混合
        pdf::Font test_font;
        test_font.family = "SimSun";
        test_font.size = 12;
        test_font.color = tinakit::Color::Black;
        
        pdf.add_text("测试1: Hello 世界! Welcome to 中国!", {100, 700}, test_font);
        
        // 测试用例2：技术术语混合
        pdf.add_text("测试2: 使用 C++ 开发 PDF library 很有趣!", {100, 680}, test_font);
        
        // 测试用例3：数字和符号混合
        pdf.add_text("测试3: 价格 $99.99 人民币 ¥688.00 优惠 20%", {100, 660}, test_font);
        
        // 测试用例4：长句子混合
        pdf.add_text("测试4: TinaKit is a powerful C++20 library for processing OpenXML files like Excel (.xlsx), Word (.docx), and generating PDF documents. 它支持中英文混合文本的智能处理。", {100, 640}, test_font);
        
        // 测试用例5：纯英文（使用中文字体）
        pdf.add_text("Test 5: Pure English text using Chinese font (SimSun)", {100, 620}, test_font);
        
        // 测试用例6：纯中文
        pdf.add_text("测试6: 纯中文文本测试，应该正常显示", {100, 600}, test_font);
        
        // ========================================
        // 3. 测试不同字体的回退机制
        // ========================================
        std::cout << "🔤 测试字体回退机制..." << std::endl;
        
        // 使用英文字体显示混合文本（应该自动回退到中文字体）
        pdf::Font english_font;
        english_font.family = "Helvetica";
        english_font.size = 12;
        english_font.color = tinakit::Color::Red;
        
        pdf.add_text("字体回退测试: Using Helvetica font for 中英混合 text", {100, 560}, english_font);
        
        // 使用Times字体
        pdf::Font times_font;
        times_font.family = "Times-Roman";
        times_font.size = 12;
        times_font.color = tinakit::Color::Green;
        
        pdf.add_text("Times字体测试: Times-Roman font with 中文字符 mixed content", {100, 540}, times_font);
        
        // ========================================
        // 4. 测试特殊字符和标点
        // ========================================
        std::cout << "🔣 测试特殊字符..." << std::endl;
        
        pdf::Font special_font;
        special_font.family = "SimSun";
        special_font.size = 11;
        special_font.color = tinakit::Color::DarkGray;
        
        pdf.add_text("特殊字符: 《TinaKit》—— 一个现代化的 C++ 库！", {100, 500}, special_font);
        pdf.add_text("标点符号: Hello, 世界! How are you? 你好吗？", {100, 480}, special_font);
        pdf.add_text("数学符号: α + β = γ, 面积 = π × r²", {100, 460}, special_font);
        
        // ========================================
        // 5. 性能和质量说明
        // ========================================
        std::cout << "📊 添加说明信息..." << std::endl;
        
        pdf::Font info_font;
        info_font.family = "SimSun";
        info_font.size = 10;
        info_font.color = tinakit::Color::Blue;
        
        pdf.add_text("技术说明:", {100, 420}, info_font);
        pdf.add_text("• 智能文本分段：自动识别中英文字符，分别处理", {120, 400}, info_font);
        pdf.add_text("• 字体回退机制：英文字体自动回退到中文字体显示CJK字符", {120, 380}, info_font);
        pdf.add_text("• Unicode支持：完整支持UTF-8编码和UTF-16BE转换", {120, 360}, info_font);
        pdf.add_text("• 渲染优化：ASCII字符使用括号格式，CJK字符使用十六进制格式", {120, 340}, info_font);
        
        // 版本信息
        pdf::Font version_font;
        version_font.family = "Helvetica";
        version_font.size = 8;
        version_font.color = tinakit::Color::Gray;
        
        pdf.add_text("Generated by TinaKit PDF Engine v1.0 - Mixed Text Processing Demo", {100, 300}, version_font);
        
        // ========================================
        // 6. 保存文档
        // ========================================
        const std::string output_filename = "mixed_text_demo.pdf";
        std::cout << "💾 保存文档: " << output_filename << std::endl;
        
        pdf.save(output_filename);
        
        std::cout << "✅ 演示完成！" << std::endl;
        std::cout << "📁 输出文件: " << output_filename << std::endl;
        std::cout << "\n🔍 请打开PDF文件检查以下内容：" << std::endl;
        std::cout << "   1. 中英文字符是否都能正确显示" << std::endl;
        std::cout << "   2. 英文字符是否没有出现乱码或全角显示" << std::endl;
        std::cout << "   3. 字体切换是否平滑自然" << std::endl;
        std::cout << "   4. 特殊字符和标点符号是否正确" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
}
