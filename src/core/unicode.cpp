/**
 * @file unicode.cpp
 * @brief TinaKit核心Unicode处理工具实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/core/unicode.hpp"
#include "tinakit/core/logger.hpp"
#include <utf8.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace tinakit::core::unicode {

// ========================================
// 编码转换
// ========================================

std::wstring utf8_to_wstring(const std::string& utf8_str) {
    if (utf8_str.empty()) {
        return L"";
    }

    try {
        std::wstring result;
        utf8::utf8to32(utf8_str.begin(), utf8_str.end(), std::back_inserter(result));
        return result;
    } catch (const std::exception& e) {
        CORE_ERROR("UTF-8 to wstring conversion failed: " + std::string(e.what()));
        return L"";
    }
}

std::string wstring_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) {
        return "";
    }

    try {
        std::string result;
        utf8::utf32to8(wstr.begin(), wstr.end(), std::back_inserter(result));
        return result;
    } catch (const std::exception& e) {
        CORE_ERROR("wstring to UTF-8 conversion failed: " + std::string(e.what()));
        return "";
    }
}

std::u16string utf8_to_utf16(const std::string& utf8_str) {
    if (utf8_str.empty()) {
        return u"";
    }

    try {
        std::u16string result;
        utf8::utf8to16(utf8_str.begin(), utf8_str.end(), std::back_inserter(result));
        return result;
    } catch (const std::exception& e) {
        CORE_ERROR("UTF-8 to UTF-16 conversion failed: " + std::string(e.what()));
        return u"";
    }
}

std::string utf16_to_utf8(const std::u16string& utf16_str) {
    if (utf16_str.empty()) {
        return "";
    }

    try {
        std::string result;
        utf8::utf16to8(utf16_str.begin(), utf16_str.end(), std::back_inserter(result));
        return result;
    } catch (const std::exception& e) {
        CORE_ERROR("UTF-16 to UTF-8 conversion failed: " + std::string(e.what()));
        return "";
    }
}

std::string utf8_to_utf16be_hex(const std::string& utf8_str) {
    if (utf8_str.empty()) {
        return "<>";
    }

    std::ostringstream oss;
    oss << "<";

    try {
        // 使用utf8库直接转换为Unicode码点，然后转为UTF-16BE
        auto it = utf8_str.begin();
        while (it != utf8_str.end()) {
            uint32_t codepoint = utf8::next(it, utf8_str.end());

            // 将Unicode码点转换为UTF-16BE
            if (codepoint <= 0xFFFF) {
                // BMP字符，直接输出
                oss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << codepoint;
            } else {
                // 超出BMP的字符，需要代理对
                codepoint -= 0x10000;
                uint16_t high = 0xD800 + (codepoint >> 10);
                uint16_t low = 0xDC00 + (codepoint & 0x3FF);
                oss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << high;
                oss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << low;
            }
        }
    } catch (const std::exception& e) {
        CORE_ERROR("UTF-8 to UTF-16BE hex conversion failed: " + std::string(e.what()));
        // 清空并重新开始
        oss.str("");
        oss << "<";

        // 更安全的回退处理：逐字节分析UTF-8
        for (size_t i = 0; i < utf8_str.length(); ) {
            unsigned char c = utf8_str[i];

            if (c < 0x80) {
                // ASCII字符
                oss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<uint16_t>(c);
                i++;
            } else if ((c & 0xE0) == 0xC0 && i + 1 < utf8_str.length()) {
                // 2字节UTF-8
                uint16_t codepoint = ((c & 0x1F) << 6) | (utf8_str[i + 1] & 0x3F);
                oss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << codepoint;
                i += 2;
            } else if ((c & 0xF0) == 0xE0 && i + 2 < utf8_str.length()) {
                // 3字节UTF-8
                uint16_t codepoint = ((c & 0x0F) << 12) | ((utf8_str[i + 1] & 0x3F) << 6) | (utf8_str[i + 2] & 0x3F);
                oss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << codepoint;
                i += 3;
            } else {
                // 无效字符，使用替换字符
                oss << "003F"; // 问号
                i++;
            }
        }
    }

    oss << ">";
    return oss.str();
}

// ========================================
// 字符分析
// ========================================

bool is_cjk_character(std::uint32_t codepoint) {
    // CJK统一汉字基本区块 (U+4E00-U+9FFF)
    if (codepoint >= 0x4E00 && codepoint <= 0x9FFF) {
        return true;
    }
    
    // CJK统一汉字扩展A区 (U+3400-U+4DBF)
    if (codepoint >= 0x3400 && codepoint <= 0x4DBF) {
        return true;
    }
    
    // CJK统一汉字扩展B区 (U+20000-U+2A6DF)
    if (codepoint >= 0x20000 && codepoint <= 0x2A6DF) {
        return true;
    }
    
    // CJK兼容汉字 (U+F900-U+FAFF)
    if (codepoint >= 0xF900 && codepoint <= 0xFAFF) {
        return true;
    }
    
    // 日文平假名 (U+3040-U+309F)
    if (codepoint >= 0x3040 && codepoint <= 0x309F) {
        return true;
    }
    
    // 日文片假名 (U+30A0-U+30FF)
    if (codepoint >= 0x30A0 && codepoint <= 0x30FF) {
        return true;
    }
    
    // 韩文音节 (U+AC00-U+D7AF)
    if (codepoint >= 0xAC00 && codepoint <= 0xD7AF) {
        return true;
    }
    
    // CJK符号和标点 (U+3000-U+303F)
    if (codepoint >= 0x3000 && codepoint <= 0x303F) {
        return true;
    }
    
    // CJK笔画 (U+31C0-U+31EF)
    if (codepoint >= 0x31C0 && codepoint <= 0x31EF) {
        return true;
    }
    
    return false;
}

bool is_cjk_character(wchar_t wc) {
    return is_cjk_character(static_cast<std::uint32_t>(wc));
}

bool contains_non_ascii(const std::string& str) {
    for (unsigned char c : str) {
        if (c > 127) {
            return true;
        }
    }
    return false;
}

bool contains_cjk(const std::string& str) {
    if (str.empty()) {
        return false;
    }

    try {
        auto it = str.begin();
        while (it != str.end()) {
            std::uint32_t codepoint = utf8::next(it, str.end());
            if (is_cjk_character(codepoint)) {
                return true;
            }
        }
    } catch (const std::exception&) {
        // UTF-8解析失败，保守地返回false
        return false;
    }

    return false;
}

// ========================================
// 文本分段
// ========================================

std::vector<TextSegment> segment_text(const std::string& text) {
    std::vector<TextSegment> segments;
    
    if (text.empty()) {
        return segments;
    }

    TextSegment current_segment;
    current_segment.start_pos = 0;
    current_segment.type = TextSegmentType::ASCII; // 默认类型
    bool segment_started = false;

    try {
        auto it = text.begin();
        std::size_t byte_pos = 0;

        while (it != text.end()) {
            auto char_start = it;
            std::size_t char_start_pos = byte_pos;
            
            std::uint32_t codepoint = utf8::next(it, text.end());
            std::size_t char_end_pos = byte_pos + std::distance(char_start, it);
            byte_pos = char_end_pos;

            TextSegmentType char_type;
            if (codepoint <= 127) {
                char_type = TextSegmentType::ASCII;
            } else if (is_cjk_character(codepoint)) {
                char_type = TextSegmentType::CJK;
            } else {
                char_type = TextSegmentType::OTHER;
            }

            if (!segment_started) {
                // 开始新段
                current_segment.type = char_type;
                current_segment.start_pos = char_start_pos;
                current_segment.text.append(char_start, it);
                segment_started = true;
            } else if (current_segment.type == char_type) {
                // 继续当前段
                current_segment.text.append(char_start, it);
            } else {
                // 类型变化，结束当前段并开始新段
                current_segment.end_pos = char_start_pos;
                segments.push_back(std::move(current_segment));

                current_segment = TextSegment{};
                current_segment.type = char_type;
                current_segment.start_pos = char_start_pos;
                current_segment.text.append(char_start, it);
            }
        }

        // 添加最后一个段
        if (segment_started) {
            current_segment.end_pos = text.length();
            segments.push_back(std::move(current_segment));
        }

    } catch (const std::exception& e) {
        CORE_ERROR("Text segmentation failed: " + std::string(e.what()));
        // 回退：将整个文本作为一个段
        segments.clear();
        TextSegment fallback_segment;
        fallback_segment.text = text;
        fallback_segment.type = contains_cjk(text) ? TextSegmentType::CJK : TextSegmentType::ASCII;
        fallback_segment.start_pos = 0;
        fallback_segment.end_pos = text.length();
        segments.push_back(fallback_segment);
    }

    return segments;
}

// ========================================
// 字符串处理
// ========================================

std::string escape_string(const std::string& text) {
    std::string result;
    result.reserve(text.length() * 2);

    for (char c : text) {
        switch (c) {
            case '(':
                result += "\\(";
                break;
            case ')':
                result += "\\)";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += c;
                break;
        }
    }

    return result;
}

std::size_t get_character_count(const std::string& utf8_str) {
    if (utf8_str.empty()) {
        return 0;
    }

    try {
        return utf8::distance(utf8_str.begin(), utf8_str.end());
    } catch (const std::exception&) {
        return 0;
    }
}

std::string substring_by_characters(const std::string& utf8_str, std::size_t char_count) {
    if (utf8_str.empty() || char_count == 0) {
        return "";
    }

    try {
        auto it = utf8_str.begin();
        auto end_it = utf8_str.end();

        for (std::size_t i = 0; i < char_count && it != end_it; ++i) {
            utf8::next(it, end_it);
        }

        return std::string(utf8_str.begin(), it);
    } catch (const std::exception&) {
        return utf8_str; // 回退：返回原字符串
    }
}

// ========================================
// 实用工具
// ========================================

bool is_valid_utf8(const std::string& utf8_str) {
    try {
        return utf8::is_valid(utf8_str.begin(), utf8_str.end());
    } catch (const std::exception&) {
        return false;
    }
}

std::string fix_utf8(const std::string& utf8_str, const std::string& replacement) {
    if (utf8_str.empty()) {
        return "";
    }

    try {
        std::string result;
        // utf8::replace_invalid只接受3个参数，第4个参数是替换字符（单个字符）
        if (!replacement.empty()) {
            utf8::replace_invalid(utf8_str.begin(), utf8_str.end(), std::back_inserter(result),
                                static_cast<std::uint32_t>(replacement[0]));
        } else {
            utf8::replace_invalid(utf8_str.begin(), utf8_str.end(), std::back_inserter(result));
        }
        return result;
    } catch (const std::exception&) {
        // 极端情况下的回退处理
        std::string result;
        for (char c : utf8_str) {
            if (static_cast<unsigned char>(c) < 128) {
                result += c;
            } else {
                result += replacement;
            }
        }
        return result;
    }
}

} // namespace tinakit::core::unicode
