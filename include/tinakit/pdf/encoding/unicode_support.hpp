/**
 * @file unicode_support.hpp
 * @brief Unicode和中文字符编码支持
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace tinakit::pdf::encoding {

/**
 * @class UnicodeSupport
 * @brief Unicode字符处理和编码转换
 * 
 * 提供完整的Unicode支持，特别优化了中文字符的处理
 */
class UnicodeSupport {
public:
    // ========================================
    // 字符编码检测
    // ========================================
    
    /**
     * @brief 检测文本是否包含中文字符
     * @param text UTF-8编码的文本
     * @return 如果包含中文字符返回true
     */
    static bool containsChinese(const std::string& text);
    
    /**
     * @brief 检测文本是否包含Unicode字符（非ASCII）
     * @param text UTF-8编码的文本
     * @return 如果包含Unicode字符返回true
     */
    static bool containsUnicode(const std::string& text);
    
    /**
     * @brief 检测文本编码类型
     * @param text 文本数据
     * @return 编码类型字符串（"UTF-8", "GBK", "ASCII"等）
     */
    static std::string detectEncoding(const std::string& text);
    
    /**
     * @brief 验证UTF-8编码的有效性
     * @param text 待验证的文本
     * @return 如果是有效的UTF-8编码返回true
     */
    static bool isValidUtf8(const std::string& text);

    // ========================================
    // 编码转换
    // ========================================
    
    /**
     * @brief UTF-8转UTF-16BE（PDF Unicode字符串格式）
     * @param utf8 UTF-8编码的字符串
     * @return UTF-16BE编码的字节序列
     */
    static std::vector<std::uint8_t> utf8ToUtf16BE(const std::string& utf8);
    
    /**
     * @brief UTF-8转GBK编码
     * @param utf8 UTF-8编码的字符串
     * @return GBK编码的字符串
     */
    static std::string utf8ToGBK(const std::string& utf8);
    
    /**
     * @brief UTF-8转Unicode码点序列
     * @param utf8 UTF-8编码的字符串
     * @return Unicode码点列表
     */
    static std::vector<std::uint32_t> utf8ToUnicodePoints(const std::string& utf8);
    
    /**
     * @brief Unicode码点转UTF-8
     * @param codepoints Unicode码点列表
     * @return UTF-8编码的字符串
     */
    static std::string unicodePointsToUtf8(const std::vector<std::uint32_t>& codepoints);

    // ========================================
    // PDF编码
    // ========================================
    
    /**
     * @brief 转换为PDF十六进制字符串格式
     * @param text UTF-8编码的文本
     * @return PDF十六进制字符串（如<FEFF4E2D6587>）
     */
    static std::string toPdfHexString(const std::string& text);
    
    /**
     * @brief 转换为PDF Unicode字符串格式
     * @param text UTF-8编码的文本
     * @return PDF Unicode字符串（带BOM）
     */
    static std::string toPdfUnicodeString(const std::string& text);
    
    /**
     * @brief 转义PDF字符串中的特殊字符
     * @param text 原始文本
     * @return 转义后的文本
     */
    static std::string escapePdfString(const std::string& text);
    
    /**
     * @brief 智能选择PDF字符串编码方式
     * @param text UTF-8编码的文本
     * @return PDF字符串（自动选择最佳编码）
     */
    static std::string toOptimalPdfString(const std::string& text);

    // ========================================
    // 字符分析
    // ========================================
    
    /**
     * @brief 获取字符串的字符数量（Unicode字符计数）
     * @param utf8 UTF-8编码的字符串
     * @return 字符数量
     */
    static std::size_t getCharacterCount(const std::string& utf8);
    
    /**
     * @brief 截取Unicode字符串
     * @param utf8 UTF-8编码的字符串
     * @param start 起始字符位置
     * @param length 字符长度
     * @return 截取的字符串
     */
    static std::string substring(const std::string& utf8, std::size_t start, std::size_t length = std::string::npos);
    
    /**
     * @brief 分析文本的字符类型分布
     * @param text UTF-8编码的文本
     * @return 字符类型统计信息
     */
    struct CharacterStats {
        std::size_t ascii_count = 0;        ///< ASCII字符数量
        std::size_t chinese_count = 0;      ///< 中文字符数量
        std::size_t other_unicode_count = 0; ///< 其他Unicode字符数量
        std::size_t total_bytes = 0;        ///< 总字节数
        std::size_t total_chars = 0;        ///< 总字符数
    };
    
    static CharacterStats analyzeText(const std::string& text);

private:
    // ========================================
    // 内部辅助方法
    // ========================================
    
    /**
     * @brief 解码单个UTF-8字符
     * @param utf8 UTF-8字节序列
     * @param pos 当前位置（会被更新）
     * @return Unicode码点
     */
    static std::uint32_t decodeUtf8Char(const std::string& utf8, std::size_t& pos);
    
    /**
     * @brief 编码单个Unicode码点为UTF-8
     * @param codepoint Unicode码点
     * @return UTF-8字节序列
     */
    static std::string encodeUtf8Char(std::uint32_t codepoint);
    
    /**
     * @brief 检查是否为中文字符码点
     * @param codepoint Unicode码点
     * @return 如果是中文字符返回true
     */
    static bool isChineseCodepoint(std::uint32_t codepoint);
    
    /**
     * @brief 将16位值转换为大端序字节
     * @param value 16位值
     * @return 大端序字节对
     */
    static std::pair<std::uint8_t, std::uint8_t> toBigEndian16(std::uint16_t value);
};

// ========================================
// 便利函数
// ========================================

/**
 * @brief 快速检查是否需要Unicode编码
 * @param text 文本
 * @return 如果需要Unicode编码返回true
 */
inline bool needsUnicodeEncoding(const std::string& text) {
    return UnicodeSupport::containsUnicode(text);
}

/**
 * @brief 快速转换为PDF字符串
 * @param text 文本
 * @return PDF格式的字符串
 */
inline std::string toPdfString(const std::string& text) {
    return UnicodeSupport::toOptimalPdfString(text);
}

} // namespace tinakit::pdf::encoding
