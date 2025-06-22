/**
 * @file font_encoding.hpp
 * @brief PDF字体编码和CJK字体支持
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace tinakit::pdf::encoding {

/**
 * @brief CJK字体类型
 */
enum class CJKFontType {
    SimplifiedChinese,   ///< 简体中文 (GB-EUC-H)
    TraditionalChinese,  ///< 繁体中文 (B5-H)
    Japanese,            ///< 日文 (90ms-RKSJ-H)
    Korean              ///< 韩文 (KSC-EUC-H)
};

/**
 * @brief 字体类型
 */
enum class FontType {
    Type1,              ///< Type1字体
    TrueType,           ///< TrueType字体
    Type0,              ///< Type0复合字体（用于CJK）
    Type3,              ///< Type3用户定义字体
    CIDFontType0,       ///< CID字体Type0
    CIDFontType2        ///< CID字体Type2
};

/**
 * @brief 字体描述符
 */
struct FontDescriptor {
    std::string name;                    ///< 字体名称
    FontType type = FontType::Type1;     ///< 字体类型
    std::string base_font;               ///< 基础字体名
    std::string encoding;                ///< 编码方式
    bool is_embedded = false;            ///< 是否嵌入
    bool supports_cjk = false;           ///< 是否支持CJK
    CJKFontType cjk_type = CJKFontType::SimplifiedChinese;  ///< CJK类型
    
    // 字体度量信息
    int ascent = 750;                    ///< 上升高度
    int descent = -250;                  ///< 下降深度
    int cap_height = 700;                ///< 大写字母高度
    int x_height = 500;                  ///< 小写字母高度
    int stem_v = 80;                     ///< 垂直笔画宽度
    int stem_h = 80;                     ///< 水平笔画宽度
    int italic_angle = 0;                ///< 斜体角度
    
    // 字体边界框 [xmin, ymin, xmax, ymax]
    std::vector<int> bbox = {-100, -250, 1000, 750};
    
    // 嵌入字体数据
    std::vector<std::uint8_t> font_data; ///< 字体文件数据
};

/**
 * @class FontEncoding
 * @brief 字体编码处理器
 * 
 * 处理各种字体编码，特别是CJK字体的编码转换
 */
class FontEncoding {
public:
    // ========================================
    // 编码映射
    // ========================================
    
    /**
     * @brief 获取CJK编码名称
     * @param type CJK字体类型
     * @return 编码名称
     */
    static std::string getCJKEncoding(CJKFontType type);
    
    /**
     * @brief 获取标准编码名称
     * @param encoding_name 编码名称
     * @return 标准化的编码名称
     */
    static std::string getStandardEncoding(const std::string& encoding_name);
    
    /**
     * @brief 检查编码是否支持Unicode
     * @param encoding 编码名称
     * @return 如果支持Unicode返回true
     */
    static bool supportsUnicode(const std::string& encoding);

    // ========================================
    // 字符映射
    // ========================================
    
    /**
     * @brief 将Unicode字符映射到字体编码
     * @param unicode_char Unicode字符码点
     * @param font_encoding 字体编码
     * @return 字体编码中的字符代码
     */
    static std::uint16_t mapUnicodeToFont(std::uint32_t unicode_char, const std::string& font_encoding);
    
    /**
     * @brief 将文本转换为字体编码
     * @param text UTF-8文本
     * @param font_encoding 目标字体编码
     * @return 编码后的字节序列
     */
    static std::vector<std::uint8_t> encodeText(const std::string& text, const std::string& font_encoding);
    
    /**
     * @brief 为CJK文本生成CID编码
     * @param text UTF-8文本
     * @param cjk_type CJK字体类型
     * @return CID编码的字节序列
     */
    static std::vector<std::uint8_t> encodeCJKText(const std::string& text, CJKFontType cjk_type);

    // ========================================
    // 字体度量
    // ========================================
    
    /**
     * @brief 计算文本宽度
     * @param text 文本
     * @param font_descriptor 字体描述符
     * @param font_size 字体大小
     * @return 文本宽度（点）
     */
    static double calculateTextWidth(const std::string& text, 
                                   const FontDescriptor& font_descriptor, 
                                   double font_size);
    
    /**
     * @brief 获取字符宽度
     * @param unicode_char Unicode字符
     * @param font_descriptor 字体描述符
     * @return 字符宽度（相对于1000单位）
     */
    static int getCharacterWidth(std::uint32_t unicode_char, const FontDescriptor& font_descriptor);

    // ========================================
    // 预定义字体
    // ========================================
    
    /**
     * @brief 获取标准字体描述符
     * @param font_name 字体名称（如"Arial", "Times-Roman"）
     * @return 字体描述符
     */
    static FontDescriptor getStandardFont(const std::string& font_name);
    
    /**
     * @brief 获取CJK字体描述符
     * @param cjk_type CJK字体类型
     * @param font_name 字体名称（可选）
     * @return 字体描述符
     */
    static FontDescriptor getCJKFont(CJKFontType cjk_type, const std::string& font_name = "");
    
    /**
     * @brief 检查字体是否为标准14字体
     * @param font_name 字体名称
     * @return 如果是标准字体返回true
     */
    static bool isStandardFont(const std::string& font_name);

    // ========================================
    // 字体回退
    // ========================================
    
    /**
     * @brief 为文本选择最佳字体
     * @param text UTF-8文本
     * @param preferred_fonts 首选字体列表
     * @return 最佳字体描述符
     */
    static FontDescriptor selectBestFont(const std::string& text, 
                                       const std::vector<std::string>& preferred_fonts);
    
    /**
     * @brief 分割文本为不同字体段
     * @param text UTF-8文本
     * @param font_fallback_chain 字体回退链
     * @return 文本段列表
     */
    struct TextSegment {
        std::string text;           ///< 文本内容
        FontDescriptor font;        ///< 使用的字体
        std::size_t start_pos;      ///< 起始位置
        std::size_t char_count;     ///< 字符数量
    };
    
    static std::vector<TextSegment> segmentTextByFont(const std::string& text,
                                                     const std::vector<std::string>& font_fallback_chain);

private:
    // ========================================
    // 内部数据和方法
    // ========================================
    
    // 标准14字体列表
    static const std::vector<std::string> standard_14_fonts_;
    
    // CJK编码映射
    static const std::map<CJKFontType, std::string> cjk_encodings_;
    
    // 字符宽度表（简化版，实际应该从字体文件读取）
    static const std::map<std::string, std::map<std::uint32_t, int>> character_widths_;
    
    /**
     * @brief 初始化标准字体数据
     */
    static void initializeStandardFonts();
    
    /**
     * @brief 检查字符是否被字体支持
     * @param unicode_char Unicode字符
     * @param font_descriptor 字体描述符
     * @return 如果支持返回true
     */
    static bool isCharacterSupported(std::uint32_t unicode_char, const FontDescriptor& font_descriptor);
};

// ========================================
// 便利函数
// ========================================

/**
 * @brief 快速获取中文字体
 * @param font_name 字体名称（可选）
 * @return 中文字体描述符
 */
inline FontDescriptor getChineseFont(const std::string& font_name = "SimSun") {
    return FontEncoding::getCJKFont(CJKFontType::SimplifiedChinese, font_name);
}

/**
 * @brief 快速检查是否需要CJK字体
 * @param text 文本
 * @return 如果需要CJK字体返回true
 */
bool needsCJKFont(const std::string& text);

} // namespace tinakit::pdf::encoding
