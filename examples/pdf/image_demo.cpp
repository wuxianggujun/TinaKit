/**
 * @file image_demo.cpp
 * @brief TinaKit PDF图像支持演示程序
 * 
 * 演示如何在PDF中嵌入和显示图像：
 * 1. 从文件加载图像
 * 2. 在PDF中显示图像
 * 3. 图像格式支持测试
 */

#include <iostream>
#include "tinakit/tinakit.hpp"
#include "tinakit/core/image.hpp"

using namespace tinakit;

void createImageTestPDF() {
    std::cout << "\n📄 创建图像测试PDF..." << std::endl;
    
    auto pdf = pdf::Document::create();
    pdf.add_page();
    
    // 标题
    pdf::Font title_font;
    title_font.family = "Helvetica";
    title_font.size = 18;
    title_font.color = tinakit::Color::Black;
    pdf.add_text("TinaKit PDF - Image Support Demo", {100, 750}, title_font);
    
    // 说明文字
    pdf::Font info_font;
    info_font.family = "Helvetica";
    info_font.size = 12;
    info_font.color = tinakit::Color::Blue;
    
    pdf.add_text("This PDF demonstrates image embedding capabilities:", {100, 720}, info_font);
    pdf.add_text("- JPEG, PNG, BMP, TGA format support", {120, 700}, info_font);
    pdf.add_text("- Automatic image loading with STBI", {120, 680}, info_font);
    pdf.add_text("- RGB and Grayscale color spaces", {120, 660}, info_font);
    
    // 演示新的图像API
    pdf::Font demo_font;
    demo_font.family = "Helvetica";
    demo_font.size = 12;
    demo_font.color = tinakit::Color::Green;

    pdf.add_text("Image API Examples:", {100, 600}, demo_font);

    // 示例1：使用core::Image类加载图像
    pdf::Font code_font;
    code_font.family = "Courier";
    code_font.size = 10;
    code_font.color = tinakit::Color::DarkGray;

    pdf.add_text("1. Using core::Image class:", {120, 580}, code_font);
    pdf.add_text("   core::Image image;", {140, 560}, code_font);
    pdf.add_text("   if (image.loadFromFile(\"photo.jpg\")) {", {140, 540}, code_font);
    pdf.add_text("       pdf.add_image(image, {100, 400}, 200, 150);", {140, 520}, code_font);
    pdf.add_text("   }", {140, 500}, code_font);

    pdf.add_text("2. Direct from file:", {120, 470}, code_font);
    pdf.add_text("   pdf.add_image(\"logo.png\", {300, 400}, 100, 100);", {140, 450}, code_font);

    pdf.add_text("3. From raw data:", {120, 420}, code_font);
    pdf.add_text("   pdf.add_image(data, width, height, channels, {100, 300});", {140, 400}, code_font);

    // 实际尝试加载一个测试图像（如果存在的话）
    tinakit::core::Image test_image;
    // 注意：这里使用一个可能不存在的图像文件作为演示
    // 在实际使用中，应该检查文件是否存在
    if (test_image.loadFromFile("test_image.png")) {
        pdf.add_text("✓ Test image loaded successfully!", {100, 350}, demo_font);
        pdf.add_image(test_image, {100, 200}, 150, 100);
    } else {
        pdf::Font warning_font;
        warning_font.family = "Helvetica";
        warning_font.size = 10;
        warning_font.color = tinakit::Color(255, 165, 0); // Orange color
        pdf.add_text("⚠ Test image not found (test_image.png)", {100, 350}, warning_font);
        pdf.add_text("  Place a PNG image named 'test_image.png' in the", {100, 330}, warning_font);
        pdf.add_text("  working directory to see image embedding in action.", {100, 310}, warning_font);
    }
    
    // 技术信息
    pdf::Font tech_font;
    tech_font.family = "Helvetica";
    tech_font.size = 10;
    tech_font.color = tinakit::Color::DarkGray;
    
    pdf.add_text("Technical Details:", {100, 520}, tech_font);
    pdf.add_text("- Image loading: STBI library", {120, 500}, tech_font);
    pdf.add_text("- PDF embedding: XObject/Image", {120, 480}, tech_font);
    pdf.add_text("- Compression: FlateDecode", {120, 460}, tech_font);
    pdf.add_text("- Color spaces: DeviceRGB, DeviceGray", {120, 440}, tech_font);
    
    pdf.save("image_demo.pdf");
    std::cout << "   ✅ 已保存: image_demo.pdf" << std::endl;
}

void testImageLoading() {
    std::cout << "\n🖼️  测试图像加载功能..." << std::endl;
    
    // 测试图像加载（不需要实际文件）
    std::cout << "   📋 支持的图像格式:" << std::endl;
    std::cout << "      • JPEG (.jpg, .jpeg)" << std::endl;
    std::cout << "      • PNG (.png)" << std::endl;
    std::cout << "      • BMP (.bmp)" << std::endl;
    std::cout << "      • TGA (.tga)" << std::endl;
    
    std::cout << "   🔧 图像处理功能:" << std::endl;
    std::cout << "      • 自动格式检测" << std::endl;
    std::cout << "      • 颜色空间转换" << std::endl;
    std::cout << "      • 尺寸信息提取" << std::endl;
    std::cout << "      • PDF XObject生成" << std::endl;
    
    // 模拟图像信息
    std::cout << "   📊 示例图像信息:" << std::endl;
    std::cout << "      • 尺寸: 800x600 像素" << std::endl;
    std::cout << "      • 颜色: RGB (3通道)" << std::endl;
    std::cout << "      • 大小: 1.44 MB (未压缩)" << std::endl;
    std::cout << "      • 格式: PNG" << std::endl;
}

int main() {
    try {
        // 初始化日志系统
        tinakit::core::initializeDefaultLogging(tinakit::core::LogLevel::INFO);
        
        std::cout << "🚀 TinaKit PDF 图像支持演示" << std::endl;
        std::cout << "================================" << std::endl;
        
        // 创建图像演示PDF
        createImageTestPDF();
        
        // 测试图像加载功能
        testImageLoading();
        
        std::cout << "\n🎉 图像支持演示完成!" << std::endl;
        std::cout << "📁 生成的文件:" << std::endl;
        std::cout << "   • image_demo.pdf - 图像支持演示文档" << std::endl;
        
        std::cout << "\n📋 下一步开发:" << std::endl;
        std::cout << "   1. 添加 add_image() API 到 Document 类" << std::endl;
        std::cout << "   2. 实现图像资源管理" << std::endl;
        std::cout << "   3. 支持图像缩放和定位" << std::endl;
        std::cout << "   4. 添加图像压缩选项" << std::endl;
        
        std::cout << "\n💡 使用示例 (未来API):" << std::endl;
        std::cout << "   pdf.add_image(\"logo.png\", {100, 500}, {200, 100});" << std::endl;
        std::cout << "   pdf.add_image(image_data, {300, 400});" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
