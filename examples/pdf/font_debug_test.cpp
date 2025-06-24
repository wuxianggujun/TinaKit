/**
 * @file font_debug_test.cpp
 * @brief 字体调试测试程序
 * @author TinaKit Team
 * @date 2025-6-24
 */

#include "tinakit/pdf/document.hpp"
#include "tinakit/pdf/config/font_config.hpp"
#include "tinakit/core/logger.hpp"
#include <iostream>
#include <filesystem>

using namespace tinakit::pdf;
using namespace tinakit::pdf::config;

void test_font_loading() {
    std::cout << "\n=== 字体加载测试 ===\n";
    
    // 测试不同的字体配置策略
    std::vector<std::pair<std::string, FontEmbeddingStrategy>> test_cases = {
        {"NONE (系统字体)", FontEmbeddingStrategy::NONE},
        {"FULL_EMBED (完整嵌入)", FontEmbeddingStrategy::FULL_EMBED},
        {"SUBSET_EMBED (子集化)", FontEmbeddingStrategy::SUBSET_EMBED}
    };
    
    for (const auto& [strategy_name, strategy] : test_cases) {
        std::cout << "\n--- 测试策略: " << strategy_name << " ---\n";
        
        auto config = FontConfig(strategy);
        auto doc = Document::create();
        doc.set_font_config(config);
        doc.add_page();
        
        // 测试不同字体
        std::vector<std::pair<std::string, std::string>> fonts = {
            {"Arial", "Arial: Hello World"},
            {"SimSun", "SimSun: 你好世界"},
            {"SourceHanSansSC-Regular", "思源字体: 你好世界 Hello"}
        };
        
        double y = 750;
        for (const auto& [font_name, text] : fonts) {
            doc.add_text(text, {100, y}, Font{font_name, 14});
            y -= 30;
        }
        
        std::string filename = "font_test_" + strategy_name + ".pdf";
        // 替换文件名中的特殊字符
        std::replace(filename.begin(), filename.end(), ' ', '_');
        std::replace(filename.begin(), filename.end(), '(', '_');
        std::replace(filename.begin(), filename.end(), ')', '_');
        
        doc.save(filename);
        
        // 检查文件大小
        if (std::filesystem::exists(filename)) {
            auto file_size = std::filesystem::file_size(filename);
            std::cout << "生成文件: " << filename << " (大小: " << file_size << " 字节)\n";
            
            // 根据策略分析文件大小
            if (strategy == FontEmbeddingStrategy::NONE && file_size > 100000) {
                std::cout << "⚠️  警告: NONE策略文件过大，可能字体被意外嵌入\n";
            } else if (strategy == FontEmbeddingStrategy::FULL_EMBED && file_size < 50000) {
                std::cout << "⚠️  警告: FULL_EMBED策略文件过小，可能字体未正确嵌入\n";
            } else if (strategy == FontEmbeddingStrategy::SUBSET_EMBED && file_size > 5000000) {
                std::cout << "⚠️  警告: SUBSET_EMBED策略文件过大，子集化可能失败\n";
            } else {
                std::cout << "✅ 文件大小符合预期\n";
            }
        } else {
            std::cout << "❌ 文件生成失败\n";
        }
    }
}

void test_chinese_encoding() {
    std::cout << "\n=== 中文编码测试 ===\n";
    
    auto config = FontConfig(FontEmbeddingStrategy::SUBSET_EMBED);
    auto doc = Document::create();
    doc.set_font_config(config);
    doc.add_page();
    
    // 测试各种中文字符
    std::vector<std::pair<std::string, std::string>> test_texts = {
        {"基本中文", "你好世界"},
        {"常用汉字", "中华人民共和国"},
        {"混合文本", "Hello 世界 123 ￥¥"},
        {"标点符号", "你好，世界！这是测试。"},
        {"数字货币", "价格：￥123.45 $67.89"},
        {"特殊字符", "©®™℃℉±×÷"}
    };
    
    double y = 750;
    for (const auto& [label, text] : test_texts) {
        doc.add_text(label + ": " + text, {100, y}, Font{"SourceHanSansSC-Regular", 12});
        y -= 25;
    }
    
    doc.save("chinese_encoding_test.pdf");
    
    if (std::filesystem::exists("chinese_encoding_test.pdf")) {
        auto file_size = std::filesystem::file_size("chinese_encoding_test.pdf");
        std::cout << "生成中文编码测试文件: chinese_encoding_test.pdf (大小: " << file_size << " 字节)\n";
        std::cout << "请在PDF阅读器中检查中文是否正确显示\n";
    }
}

void test_font_fallback() {
    std::cout << "\n=== 字体回退测试 ===\n";
    
    auto config = FontConfig(FontEmbeddingStrategy::AUTO);
    auto doc = Document::create();
    doc.set_font_config(config);
    doc.add_page();
    
    // 测试不存在的字体（应该触发回退）
    std::vector<std::pair<std::string, std::string>> fallback_tests = {
        {"NonExistentFont", "这个字体不存在"},
        {"FakeChineseFont", "假的中文字体"},
        {"SourceHanSansSC-Regular", "真实的思源字体"}
    };
    
    double y = 750;
    for (const auto& [font_name, text] : fallback_tests) {
        doc.add_text(font_name + ": " + text, {100, y}, Font{font_name, 12});
        y -= 30;
    }
    
    doc.save("font_fallback_test.pdf");
    
    if (std::filesystem::exists("font_fallback_test.pdf")) {
        auto file_size = std::filesystem::file_size("font_fallback_test.pdf");
        std::cout << "生成字体回退测试文件: font_fallback_test.pdf (大小: " << file_size << " 字节)\n";
        std::cout << "检查不存在的字体是否正确回退到可用字体\n";
    }
}

int main() {
    try {
        // 初始化日志系统
        tinakit::core::initializeDefaultLogging(tinakit::core::LogLevel::DEBUG);
        
        std::cout << "TinaKit PDF 字体调试测试\n";
        std::cout << "========================\n";
        
        test_font_loading();
        test_chinese_encoding();
        test_font_fallback();
        
        std::cout << "\n所有测试完成！\n";
        std::cout << "\n检查要点：\n";
        std::cout << "1. 文件大小是否符合字体策略预期\n";
        std::cout << "2. 中文字符是否正确显示（不是方块或乱码）\n";
        std::cout << "3. 字体回退是否正常工作\n";
        std::cout << "4. 查看调试日志了解字体加载过程\n";
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
