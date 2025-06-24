/**
 * @file encoding_test.cpp
 * @brief 字符编码测试程序
 * @author TinaKit Team
 * @date 2025-6-24
 */

#include "tinakit/pdf/document.hpp"
#include "tinakit/core/unicode.hpp"
#include "tinakit/core/logger.hpp"
#include <iostream>
#include <iomanip>

using namespace tinakit::pdf;
using namespace tinakit::core;

void test_unicode_conversion() {
    std::cout << "\n=== Unicode 转换测试 ===\n";
    
    // 测试各种字符的编码转换
    std::vector<std::string> test_strings = {
        "Hello",           // 纯ASCII
        "你好",            // 基本中文
        "世界",            // 基本中文
        "测试",            // 基本中文
        "Hello世界",       // 混合
        "123",             // 数字
        "！",              // 中文标点
        "。",              // 中文标点
        "，",              // 中文标点
    };
    
    for (const auto& text : test_strings) {
        std::cout << "\n测试字符串: \"" << text << "\"\n";
        
        // 测试UTF-8到UTF-16BE转换
        std::string utf16be_hex = unicode::utf8_to_utf16be_hex(text);
        std::cout << "UTF-16BE Hex: " << utf16be_hex << "\n";
        
        // 手动验证每个字符的Unicode码点
        std::cout << "字符分析: ";
        for (size_t i = 0; i < text.length(); ) {
            uint32_t codepoint = 0;
            int bytes = 1;
            
            unsigned char c = text[i];
            if (c < 0x80) {
                codepoint = c;
                bytes = 1;
            } else if ((c & 0xE0) == 0xC0) {
                codepoint = (c & 0x1F) << 6;
                if (i + 1 < text.length()) {
                    codepoint |= (text[i + 1] & 0x3F);
                }
                bytes = 2;
            } else if ((c & 0xF0) == 0xE0) {
                codepoint = (c & 0x0F) << 12;
                if (i + 1 < text.length()) {
                    codepoint |= (text[i + 1] & 0x3F) << 6;
                }
                if (i + 2 < text.length()) {
                    codepoint |= (text[i + 2] & 0x3F);
                }
                bytes = 3;
            }
            
            std::cout << "U+" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << codepoint << " ";
            i += bytes;
        }
        std::cout << "\n";
    }
}

void test_simple_pdf() {
    std::cout << "\n=== 简单PDF测试 ===\n";
    
    auto doc = Document::create();
    doc.add_page();
    
    // 测试单个字符，避免字符串连接问题
    std::vector<std::pair<std::string, double>> simple_tests = {
        {"Hello", 750},
        {"你好", 720},
        {"世界", 690},
        {"测试", 660},
        {"123", 630},
    };
    
    for (const auto& [text, y] : simple_tests) {
        std::cout << "添加文本: \"" << text << "\" at y=" << y << "\n";
        doc.add_text(text, {100, y}, Font{"SourceHanSansSC-Regular", 14});
    }
    
    doc.save("simple_encoding_test.pdf");
    std::cout << "生成文件: simple_encoding_test.pdf\n";
}

void test_character_by_character() {
    std::cout << "\n=== 逐字符测试 ===\n";
    
    auto doc = Document::create();
    doc.add_page();
    
    // 测试问题字符
    std::vector<std::string> problem_chars = {
        "特", "殊", "字", "符", "©", "®", "™", "℃", "℉", "±", "×", "÷",
        "标", "点", "符", "号", "你", "好", "，", "世", "界", "！", "这", "是", "测", "试", "。",
        "数", "字", "货", "币", "价", "格", "：", "￥", "1", "2", "3", ".", "4", "5", " ", "$", "6", "7", ".", "8", "9"
    };
    
    double x = 100;
    double y = 750;
    
    for (const auto& ch : problem_chars) {
        std::cout << "测试字符: \"" << ch << "\"\n";
        
        // 检查Unicode码点
        if (!ch.empty()) {
            uint32_t codepoint = 0;
            unsigned char c = ch[0];
            if (c < 0x80) {
                codepoint = c;
            } else if ((c & 0xE0) == 0xC0 && ch.length() >= 2) {
                codepoint = ((c & 0x1F) << 6) | (ch[1] & 0x3F);
            } else if ((c & 0xF0) == 0xE0 && ch.length() >= 3) {
                codepoint = ((c & 0x0F) << 12) | ((ch[1] & 0x3F) << 6) | (ch[2] & 0x3F);
            }
            
            std::cout << "  Unicode: U+" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << codepoint << "\n";
        }
        
        doc.add_text(ch, {x, y}, Font{"SourceHanSansSC-Regular", 12});
        
        x += 20;
        if (x > 500) {
            x = 100;
            y -= 30;
        }
    }
    
    doc.save("character_by_character_test.pdf");
    std::cout << "生成文件: character_by_character_test.pdf\n";
}

void test_string_concatenation() {
    std::cout << "\n=== 字符串连接测试 ===\n";
    
    auto doc = Document::create();
    doc.add_page();
    
    // 测试字符串连接是否导致编码问题
    std::string label = "特殊字符";
    std::string content = "©®™℃℉±×÷";
    
    std::cout << "标签: \"" << label << "\"\n";
    std::cout << "内容: \"" << content << "\"\n";
    
    // 方法1：直接连接
    std::string combined1 = label + ": " + content;
    std::cout << "直接连接: \"" << combined1 << "\"\n";
    doc.add_text(combined1, {100, 750}, Font{"SourceHanSansSC-Regular", 12});
    
    // 方法2：分别添加
    doc.add_text(label, {100, 720}, Font{"SourceHanSansSC-Regular", 12});
    doc.add_text(": ", {200, 720}, Font{"SourceHanSansSC-Regular", 12});
    doc.add_text(content, {220, 720}, Font{"SourceHanSansSC-Regular", 12});
    
    // 方法3：使用ostringstream
    std::ostringstream oss;
    oss << label << ": " << content;
    std::string combined3 = oss.str();
    std::cout << "ostringstream: \"" << combined3 << "\"\n";
    doc.add_text(combined3, {100, 690}, Font{"SourceHanSansSC-Regular", 12});
    
    doc.save("string_concatenation_test.pdf");
    std::cout << "生成文件: string_concatenation_test.pdf\n";
}

int main() {
    try {
        // 初始化日志系统
        tinakit::core::initializeDefaultLogging(tinakit::core::LogLevel::DEBUG);
        
        std::cout << "TinaKit 字符编码调试测试\n";
        std::cout << "========================\n";
        
        test_unicode_conversion();
        test_simple_pdf();
        test_character_by_character();
        test_string_concatenation();
        
        std::cout << "\n所有编码测试完成！\n";
        std::cout << "请检查生成的PDF文件中的字符显示是否正确\n";
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
