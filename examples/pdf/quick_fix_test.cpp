/**
 * @file quick_fix_test.cpp
 * @brief 快速修复验证测试
 * @author TinaKit Team
 * @date 2025-6-24
 */

#include "tinakit/pdf/document.hpp"
#include "tinakit/pdf/config/font_config.hpp"
#include "tinakit/core/logger.hpp"
#include <iostream>

using namespace tinakit::pdf;
using namespace tinakit::pdf::config;

void test_problematic_characters() {
    std::cout << "\n=== 问题字符修复测试 ===\n";
    
    auto config = FontConfig(FontEmbeddingStrategy::SUBSET_EMBED);
    auto doc = Document::create();
    doc.set_font_config(config);
    doc.add_page();
    
    // 测试之前有问题的字符组合
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"基本中文", "你好世界"},
        {"常用汉字", "中华人民共和国"},
        {"简单混合", "Hello 世界 123"},
        {"基本标点", "你好世界测试"},
        {"简单数字", "价格123元"},
        {"基本符号", "加减乘除"}
    };
    
    double y = 750;
    for (const auto& [label, text] : test_cases) {
        // 分别添加标签和内容，避免字符串连接问题
        doc.add_text(label, {100, y}, Font{"SourceHanSansSC-Regular", 12});
        doc.add_text(": ", {200, y}, Font{"SourceHanSansSC-Regular", 12});
        doc.add_text(text, {220, y}, Font{"SourceHanSansSC-Regular", 12});
        y -= 25;
    }
    
    doc.save("quick_fix_test.pdf");
    std::cout << "生成测试文件: quick_fix_test.pdf\n";
}

void test_individual_characters() {
    std::cout << "\n=== 单个字符测试 ===\n";
    
    auto config = FontConfig(FontEmbeddingStrategy::SUBSET_EMBED);
    auto doc = Document::create();
    doc.set_font_config(config);
    doc.add_page();
    
    // 测试单个中文字符
    std::vector<std::string> chinese_chars = {
        "你", "好", "世", "界", "中", "华", "人", "民", "共", "和", "国"
    };
    
    double x = 100;
    double y = 750;
    
    for (const auto& ch : chinese_chars) {
        doc.add_text(ch, {x, y}, Font{"SourceHanSansSC-Regular", 14});
        x += 30;
        if (x > 500) {
            x = 100;
            y -= 30;
        }
    }
    
    // 测试英文字符
    y -= 50;
    x = 100;
    std::vector<std::string> english_chars = {
        "H", "e", "l", "l", "o", " ", "W", "o", "r", "l", "d"
    };
    
    for (const auto& ch : english_chars) {
        doc.add_text(ch, {x, y}, Font{"SourceHanSansSC-Regular", 14});
        x += 20;
    }
    
    // 测试数字
    y -= 50;
    x = 100;
    std::vector<std::string> number_chars = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"
    };
    
    for (const auto& ch : number_chars) {
        doc.add_text(ch, {x, y}, Font{"SourceHanSansSC-Regular", 14});
        x += 20;
    }
    
    doc.save("individual_characters_test.pdf");
    std::cout << "生成单字符测试文件: individual_characters_test.pdf\n";
}

void test_different_fonts() {
    std::cout << "\n=== 不同字体测试 ===\n";
    
    auto config = FontConfig(FontEmbeddingStrategy::AUTO);
    auto doc = Document::create();
    doc.set_font_config(config);
    doc.add_page();
    
    // 测试不同字体的中文显示
    std::vector<std::pair<std::string, std::string>> font_tests = {
        {"SimSun", "SimSun字体测试"},
        {"SourceHanSansSC-Regular", "思源字体测试"},
        {"Arial", "Arial字体测试"}
    };
    
    double y = 750;
    for (const auto& [font_name, text] : font_tests) {
        doc.add_text(text, {100, y}, Font{font_name, 14});
        y -= 30;
    }
    
    doc.save("different_fonts_test.pdf");
    std::cout << "生成字体测试文件: different_fonts_test.pdf\n";
}

void test_encoding_strategies() {
    std::cout << "\n=== 编码策略测试 ===\n";
    
    std::vector<std::pair<std::string, FontEmbeddingStrategy>> strategies = {
        {"NONE", FontEmbeddingStrategy::NONE},
        {"FULL_EMBED", FontEmbeddingStrategy::FULL_EMBED},
        {"SUBSET_EMBED", FontEmbeddingStrategy::SUBSET_EMBED}
    };
    
    for (const auto& [strategy_name, strategy] : strategies) {
        auto config = FontConfig(strategy);
        auto doc = Document::create();
        doc.set_font_config(config);
        doc.add_page();
        
        // 简单的中文测试
        doc.add_text("你好世界", {100, 750}, Font{"SourceHanSansSC-Regular", 14});
        doc.add_text("Hello World", {100, 720}, Font{"SourceHanSansSC-Regular", 14});
        doc.add_text("123456", {100, 690}, Font{"SourceHanSansSC-Regular", 14});
        
        std::string filename = "strategy_" + strategy_name + "_test.pdf";
        doc.save(filename);
        std::cout << "生成策略测试文件: " << filename << "\n";
    }
}

int main() {
    try {
        // 初始化日志系统
        tinakit::core::initializeDefaultLogging(tinakit::core::LogLevel::DEBUG);
        
        std::cout << "TinaKit PDF 快速修复验证测试\n";
        std::cout << "============================\n";
        std::cout << "修复内容:\n";
        std::cout << "1. 强制使用UTF-16BE编码，避免GID映射问题\n";
        std::cout << "2. 改进UTF-16BE转换逻辑\n";
        std::cout << "3. 增强字体文件搜索和回退机制\n\n";
        
        test_problematic_characters();
        test_individual_characters();
        test_different_fonts();
        test_encoding_strategies();
        
        std::cout << "\n所有测试完成！\n";
        std::cout << "\n检查要点：\n";
        std::cout << "1. 中文字符应该正确显示，不是方块或乱码\n";
        std::cout << "2. 不应该出现多余的'W'字符\n";
        std::cout << "3. 标点符号应该正确显示\n";
        std::cout << "4. 不同字体策略应该产生不同大小的文件\n";
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
