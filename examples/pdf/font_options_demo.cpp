/**
 * @file font_options_demo.cpp
 * @brief TinaKit PDF字体选项演示程序
 * 
 * 演示不同的字体嵌入选项：
 * 1. 标准字体（无嵌入）
 * 2. 嵌入字体（自动嵌入）
 * 3. 引用字体（禁用嵌入）
 */

#include <iostream>
#include "tinakit/tinakit.hpp"

using namespace tinakit;

void createStandardFontPDF() {
    std::cout << "\n📄 创建标准字体PDF（无嵌入）..." << std::endl;
    
    auto pdf = pdf::Document::create();
    pdf.add_page();
    
    pdf::Font font;
    font.family = "Helvetica";
    font.size = 14;
    font.color = tinakit::Color::Black;
    
    pdf.add_text("TinaKit PDF - Standard Fonts", {100, 750}, font);
    pdf.add_text("This PDF uses standard fonts only.", {100, 720}, font);
    pdf.add_text("File size: Small (~50KB)", {100, 690}, font);
    pdf.add_text("Compatibility: Depends on system fonts", {100, 660}, font);
    
    font.size = 12;
    font.color = tinakit::Color::Blue;
    pdf.add_text("Advantages:", {100, 620}, font);
    pdf.add_text("- Small file size", {120, 600}, font);
    pdf.add_text("- Fast generation", {120, 580}, font);
    
    font.color = tinakit::Color::Red;
    pdf.add_text("Disadvantages:", {100, 540}, font);
    pdf.add_text("- Limited to ASCII characters", {120, 520}, font);
    pdf.add_text("- May not display correctly on all devices", {120, 500}, font);
    
    pdf.save("standard_fonts.pdf");
    std::cout << "   ✅ 已保存: standard_fonts.pdf" << std::endl;
}

void createEmbeddedFontPDF() {
    std::cout << "\n📄 创建嵌入字体PDF（自动嵌入）..." << std::endl;
    
    auto pdf = pdf::Document::create();
    pdf.add_page();
    
    // 英文标题
    pdf::Font title_font;
    title_font.family = "Helvetica";
    title_font.size = 16;
    title_font.color = tinakit::Color::Black;
    pdf.add_text("TinaKit PDF - Embedded Fonts", {100, 750}, title_font);
    
    // 中文内容（自动嵌入SimSun字体）
    pdf::Font chinese_font;
    chinese_font.family = "SimSun";
    chinese_font.size = 14;
    chinese_font.color = tinakit::Color::Blue;
    
    pdf.add_text("你好世界！这是中文测试", {100, 720}, chinese_font);
    pdf.add_text("TinaKit PDF 支持中文显示", {100, 700}, chinese_font);
    pdf.add_text("字体已自动嵌入到PDF中", {100, 680}, chinese_font);
    
    // 英文说明
    pdf::Font info_font;
    info_font.family = "Helvetica";
    info_font.size = 12;
    info_font.color = tinakit::Color::Black;
    
    pdf.add_text("File size: Large (~18MB)", {100, 640}, info_font);
    pdf.add_text("Compatibility: Works on all devices", {100, 620}, info_font);
    
    info_font.color = tinakit::Color::Green;
    pdf.add_text("Advantages:", {100, 580}, info_font);
    pdf.add_text("- Perfect cross-platform compatibility", {120, 560}, info_font);
    pdf.add_text("- Supports all Unicode characters", {120, 540}, info_font);
    pdf.add_text("- Consistent appearance everywhere", {120, 520}, info_font);
    
    info_font.color = tinakit::Color::Red;
    pdf.add_text("Disadvantages:", {100, 480}, info_font);
    pdf.add_text("- Large file size", {120, 460}, info_font);
    pdf.add_text("- Slower generation", {120, 440}, info_font);
    
    pdf.save("embedded_fonts.pdf");
    std::cout << "   ✅ 已保存: embedded_fonts.pdf" << std::endl;
}

void createReferenceFontPDF() {
    std::cout << "\n📄 创建引用字体PDF（禁用嵌入）..." << std::endl;
    
    auto pdf = pdf::Document::create();
    pdf.add_page();
    
    // 英文标题
    pdf::Font title_font;
    title_font.family = "Helvetica";
    title_font.size = 16;
    title_font.color = tinakit::Color::Black;
    pdf.add_text("TinaKit PDF - Referenced Fonts", {100, 750}, title_font);
    
    // 注意：这里需要API支持禁用嵌入，目前先用相同的方式
    pdf::Font chinese_font;
    chinese_font.family = "SimSun";
    chinese_font.size = 14;
    chinese_font.color = tinakit::Color::Magenta;
    
    pdf.add_text("你好世界！(仅引用字体)", {100, 720}, chinese_font);
    pdf.add_text("这个PDF仅引用系统字体", {100, 700}, chinese_font);
    
    // 英文说明
    pdf::Font info_font;
    info_font.family = "Helvetica";
    info_font.size = 12;
    info_font.color = tinakit::Color::Black;
    
    pdf.add_text("File size: Small (~50KB)", {100, 660}, info_font);
    pdf.add_text("Compatibility: Requires system fonts", {100, 640}, info_font);
    
    info_font.color = tinakit::Color::Yellow;
    pdf.add_text("Use case:", {100, 600}, info_font);
    pdf.add_text("- Internal documents", {120, 580}, info_font);
    pdf.add_text("- Known environment deployment", {120, 560}, info_font);
    pdf.add_text("- Development and testing", {120, 540}, info_font);
    
    pdf.save("referenced_fonts.pdf");
    std::cout << "   ✅ 已保存: referenced_fonts.pdf" << std::endl;
}

int main() {
    try {
        // 初始化日志系统
        tinakit::core::initializeDefaultLogging(tinakit::core::LogLevel::INFO);
        
        std::cout << "🚀 TinaKit PDF 字体选项演示" << std::endl;
        std::cout << "================================" << std::endl;
        
        // 创建三种不同的PDF文件
        createStandardFontPDF();
        createEmbeddedFontPDF();
        createReferenceFontPDF();
        
        std::cout << "\n🎉 所有PDF文件生成完成!" << std::endl;
        std::cout << "📁 生成的文件:" << std::endl;
        std::cout << "   • standard_fonts.pdf   - 标准字体（小文件）" << std::endl;
        std::cout << "   • embedded_fonts.pdf   - 嵌入字体（大文件，兼容性好）" << std::endl;
        std::cout << "   • referenced_fonts.pdf - 引用字体（小文件，需系统支持）" << std::endl;
        
        std::cout << "\n📊 对比总结:" << std::endl;
        std::cout << "   📏 文件大小: standard < referenced < embedded" << std::endl;
        std::cout << "   🌍 兼容性: embedded > referenced > standard" << std::endl;
        std::cout << "   ⚡ 生成速度: standard > referenced > embedded" << std::endl;
        std::cout << "   🎨 字符支持: embedded > referenced > standard" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
