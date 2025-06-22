/**
 * @file unicode.hpp
 * @brief TinaKit核心Unicode处理工具
 * @author TinaKit Team
 * @date 2025-6-22
 * 
 * 提供统一的Unicode字符串处理接口，包括UTF-8/UTF-16转换、
 * CJK字符判断、文本分段等功能。封装utfcpp库的功能。
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace tinakit::core::unicode {

// ========================================
// 编码转换
// ========================================

/**
 * @brief 将UTF-8字符串转换为宽字符串
 * @param utf8_str UTF-8编码的字符串
 * @return 宽字符串，转换失败时返回空字符串
 */
std::wstring utf8_to_wstring(const std::string& utf8_str);

/**
 * @brief 将宽字符串转换为UTF-8字符串
 * @param wstr 宽字符串
 * @return UTF-8编码的字符串
 */
std::string wstring_to_utf8(const std::wstring& wstr);

/**
 * @brief 将UTF-8字符串转换为UTF-16字符串
 * @param utf8_str UTF-8编码的字符串
 * @return UTF-16编码的字符串，转换失败时返回空字符串
 */
std::u16string utf8_to_utf16(const std::string& utf8_str);

/**
 * @brief 将UTF-16字符串转换为UTF-8字符串
 * @param utf16_str UTF-16编码的字符串
 * @return UTF-8编码的字符串
 */
std::string utf16_to_utf8(const std::u16string& utf16_str);

/**
 * @brief 将UTF-8字符串转换为UTF-16BE十六进制字符串（用于PDF）
 * @param utf8_str UTF-8编码的字符串
 * @return UTF-16BE十六进制字符串，格式为"<FEFF...>"
 */
std::string utf8_to_utf16be_hex(const std::string& utf8_str);

// ========================================
// 字符分析
// ========================================

/**
 * @brief 判断Unicode码点是否为CJK字符
 * @param codepoint Unicode码点
 * @return 是CJK字符返回true，否则返回false
 */
bool is_cjk_character(std::uint32_t codepoint);

/**
 * @brief 判断宽字符是否为CJK字符
 * @param wc 宽字符
 * @return 是CJK字符返回true，否则返回false
 */
bool is_cjk_character(wchar_t wc);

/**
 * @brief 判断字符串是否包含非ASCII字符
 * @param str 要检查的字符串
 * @return 包含非ASCII字符返回true，否则返回false
 */
bool contains_non_ascii(const std::string& str);

/**
 * @brief 判断字符串是否包含CJK字符
 * @param str 要检查的字符串
 * @return 包含CJK字符返回true，否则返回false
 */
bool contains_cjk(const std::string& str);

// ========================================
// 文本分段
// ========================================

/**
 * @brief 文本段类型
 */
enum class TextSegmentType {
    ASCII,      ///< ASCII字符段
    CJK,        ///< CJK字符段
    OTHER       ///< 其他Unicode字符段
};

/**
 * @brief 文本段结构
 */
struct TextSegment {
    std::string text;           ///< 文本内容
    TextSegmentType type;       ///< 段类型
    std::size_t start_pos;      ///< 在原字符串中的起始位置
    std::size_t end_pos;        ///< 在原字符串中的结束位置
};

/**
 * @brief 将混合文本分割为不同类型的段
 * @param text 要分割的文本
 * @return 文本段列表
 * 
 * 将包含ASCII、CJK和其他Unicode字符的混合文本分割为
 * 同质的文本段，便于后续的字体选择和渲染处理。
 */
std::vector<TextSegment> segment_text(const std::string& text);

// ========================================
// 字符串处理
// ========================================

/**
 * @brief 转义字符串中的特殊字符
 * @param text 原始文本
 * @return 转义后的文本
 *
 * 转义字符串中的特殊字符，如括号、反斜杠等。
 * 主要用于PDF等格式的字符串处理。
 */
std::string escape_string(const std::string& text);

/**
 * @brief 转义PDF字符串中的特殊字符（已弃用，请使用escape_string）
 * @deprecated 请使用escape_string代替
 */
[[deprecated("Use escape_string instead")]]
inline std::string escape_pdf_string(const std::string& text) {
    return escape_string(text);
}

/**
 * @brief 获取UTF-8字符串的字符数（不是字节数）
 * @param utf8_str UTF-8编码的字符串
 * @return 字符数，无效UTF-8时返回0
 */
std::size_t get_character_count(const std::string& utf8_str);

/**
 * @brief 安全地截取UTF-8字符串的前N个字符
 * @param utf8_str UTF-8编码的字符串
 * @param char_count 要截取的字符数
 * @return 截取后的字符串
 */
std::string substring_by_characters(const std::string& utf8_str, std::size_t char_count);

// ========================================
// 错误处理
// ========================================

/**
 * @brief Unicode处理异常类
 */
class UnicodeException : public std::exception {
public:
    explicit UnicodeException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

// ========================================
// 实用工具
// ========================================

/**
 * @brief 验证UTF-8字符串的有效性
 * @param utf8_str 要验证的字符串
 * @return 有效返回true，否则返回false
 */
bool is_valid_utf8(const std::string& utf8_str);

/**
 * @brief 修复损坏的UTF-8字符串
 * @param utf8_str 可能损坏的UTF-8字符串
 * @param replacement 替换无效字符的字符串（默认为"?"）
 * @return 修复后的UTF-8字符串
 */
std::string fix_utf8(const std::string& utf8_str, const std::string& replacement = "?");

} // namespace tinakit::core::unicode
