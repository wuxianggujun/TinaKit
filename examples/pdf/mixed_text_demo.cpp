/**
 * @file mixed_text_demo.cpp
 * @brief 中英混合文本显示测试
 * @author TinaKit Team
 * @date 2025-6-22
 *
 * 专注于测试中英文混合文本的基本显示功能，
 * 使用思源黑体字体进行测试。
 */

#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include "tinakit/tinakit.hpp"
#include "tinakit/core/logger.hpp"

using namespace tinakit;

// 加载字体文件数据
std::vector<uint8_t> loadFontData(const std::string& font_path) {
    std::vector<uint8_t> font_data;

    std::ifstream file(font_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "❌ 无法打开字体文件: " << font_path << std::endl;
        return font_data;
    }

    // 获取文件大小
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    // 读取文件数据
    font_data.resize(file_size);
    file.read(reinterpret_cast<char*>(font_data.data()), file_size);

    if (file.good()) {
        std::cout << "✅ 字体文件加载成功: " << font_path << " (" << file_size << " bytes)" << std::endl;
    } else {
        std::cerr << "❌ 读取字体文件失败: " << font_path << std::endl;
        font_data.clear();
    }

    return font_data;
}

int main() {
    try {
        // 初始化日志系统，使用DEBUG级别查看详细信息
        tinakit::core::initializeDefaultLogging(tinakit::core::LogLevel::DEBUG, "mixed_text_demo.log");

        std::cout << "🌏 TinaKit PDF 中英混合文本测试（思源黑体）" << std::endl;
        std::cout << "=========================================" << std::endl;

        // 加载思源黑体字体文件
        const std::string font_path = "SourceHanSansSC-Regular.otf";
        std::cout << "📂 加载字体文件: " << font_path << std::endl;

        if (!std::filesystem::exists(font_path)) {
            std::cerr << "❌ 字体文件不存在: " << font_path << std::endl;
            std::cerr << "请确保思源黑体文件位于当前目录下" << std::endl;
            return 1;
        }

        auto font_data = loadFontData(font_path);
        if (font_data.empty()) {
            std::cerr << "❌ 字体文件加载失败" << std::endl;
            return 1;
        }

        // 创建PDF文档
        std::cout << "📄 创建PDF文档..." << std::endl;
        auto pdf = pdf::Document::create();

        // 注册思源黑体字体
        std::cout << "🔤 注册思源黑体字体..." << std::endl;
        const std::string font_name = "SourceHanSansSC-Regular";
        std::string font_resource_id = pdf.register_font(font_name, font_data, true);
        std::cout << "✅ 字体注册成功，资源ID: " << font_resource_id << std::endl;
        std::cout << "📊 原始字体大小: " << (font_data.size() / 1024.0 / 1024.0) << " MB" << std::endl;

        // 显示字体子集化信息
        std::cout << "ℹ️ 字体子集化功能需要pyftsubset工具，当前使用完整字体" << std::endl;

        // 添加页面
        pdf.add_page();

        // 使用思源黑体进行测试
        pdf::Font font;
        font.family = font_name;  // 使用注册的字体名称
        font.size = 14;
        font.color = tinakit::Color::Black;

        std::cout << "✍️ 添加测试文本..." << std::endl;

        // 性能测试：记录开始时间
        auto start_time = std::chrono::high_resolution_clock::now();

        // 基本中英文混合测试
        pdf.add_text("Hello 世界", {100, 700}, font);
        pdf.add_text("测试 Test", {100, 680}, font);
        pdf.add_text("价格 ¥100", {100, 660}, font);
        pdf.add_text("English 中文 Mixed", {100, 640}, font);
        pdf.add_text("纯中文测试", {100, 620}, font);
        pdf.add_text("Pure English Test", {100, 600}, font);

        // 重复文本测试（验证缓存效果）
        pdf.add_text("Hello 世界", {100, 580}, font);  // 重复文本
        pdf.add_text("测试 Test", {100, 560}, font);   // 重复文本

        auto text_time = std::chrono::high_resolution_clock::now();
        auto text_duration = std::chrono::duration_cast<std::chrono::milliseconds>(text_time - start_time);
        std::cout << "⏱️ 文本添加耗时: " << text_duration.count() << "ms" << std::endl;

        // 保存文档
        const std::string output_filename = "mixed_text_demo.pdf";
        std::cout << "💾 保存文档: " << output_filename << std::endl;

        auto save_start = std::chrono::high_resolution_clock::now();
        pdf.save(output_filename);
        auto save_end = std::chrono::high_resolution_clock::now();
        auto save_duration = std::chrono::duration_cast<std::chrono::milliseconds>(save_end - save_start);

        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(save_end - start_time);

        std::cout << "✅ 测试完成！" << std::endl;
        std::cout << "📁 输出文件: " << output_filename << std::endl;
        std::cout << "⏱️ 保存耗时: " << save_duration.count() << "ms" << std::endl;
        std::cout << "⏱️ 总耗时: " << total_duration.count() << "ms" << std::endl;
        std::cout << "\n🔍 请检查PDF文件中的中英文字符显示是否正确" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
}
