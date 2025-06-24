/**
 * @file font_subset_manager.hpp
 * @brief 字体子集化管理器
 * @author TinaKit Team
 * @date 2025-6-23
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

namespace tinakit::pdf::core {

/**
 * @brief 字体使用信息
 */
struct FontUsage {
    std::string font_name;              ///< 字体名称
    std::vector<std::uint8_t> font_data; ///< 原始字体数据
    std::set<uint32_t> used_codepoints; ///< 使用的Unicode码点
    bool enable_subsetting;             ///< 是否启用子集化
    bool embed_font;                    ///< 是否嵌入字体
};

/**
 * @brief 字体子集化结果
 */
struct FontSubsetResult {
    std::string font_name;              ///< 字体名称
    std::vector<std::uint8_t> subset_data; ///< 子集字体数据
    size_t original_size;               ///< 原始大小
    size_t subset_size;                 ///< 子集大小
    size_t codepoints_count;            ///< 码点数量
    bool success;                       ///< 是否成功
};

/**
 * @brief 字体子集化管理器
 * 
 * 负责管理整个字体子集化流程：
 * 1. 收集字体使用信息
 * 2. 执行字体子集化
 * 3. 提供子集化结果
 */
class FontSubsetManager {
public:
    /**
     * @brief 构造函数
     */
    FontSubsetManager();
    
    /**
     * @brief 析构函数
     */
    ~FontSubsetManager();
    
    /**
     * @brief 注册字体用于子集化
     * @param font_name 字体名称
     * @param font_data 字体数据
     * @param enable_subsetting 是否启用子集化
     * @param embed_font 是否嵌入字体
     */
    void registerFont(const std::string& font_name,
                     const std::vector<std::uint8_t>& font_data,
                     bool enable_subsetting = true,
                     bool embed_font = true);
    
    /**
     * @brief 记录字体使用的字符
     * @param font_name 字体名称
     * @param text 使用的文本
     */
    void recordTextUsage(const std::string& font_name, const std::string& text);
    
    /**
     * @brief 记录字体使用的Unicode码点
     * @param font_name 字体名称
     * @param codepoint Unicode码点
     */
    void recordCodepointUsage(const std::string& font_name, uint32_t codepoint);
    
    /**
     * @brief 执行所有字体的子集化
     * @return 子集化结果列表
     */
    std::vector<FontSubsetResult> performSubsetting();
    
    /**
     * @brief 获取字体的子集化结果
     * @param font_name 字体名称
     * @return 子集化结果，如果不存在返回nullptr
     */
    const FontSubsetResult* getSubsetResult(const std::string& font_name) const;
    
    /**
     * @brief 获取字体的最终数据（子集或原始）
     * @param font_name 字体名称
     * @return 字体数据，如果不存在返回空vector
     */
    std::vector<std::uint8_t> getFinalFontData(const std::string& font_name) const;
    
    /**
     * @brief 检查字体是否已注册
     * @param font_name 字体名称
     * @return 已注册返回true
     */
    bool isFontRegistered(const std::string& font_name) const;
    
    /**
     * @brief 检查字体是否启用了子集化
     * @param font_name 字体名称
     * @return 启用返回true
     */
    bool isSubsettingEnabled(const std::string& font_name) const;
    
    /**
     * @brief 获取统计信息
     * @return 统计信息字符串
     */
    std::string getStatistics() const;
    
    /**
     * @brief 清除所有数据
     */
    void clear();

    /**
     * @brief 获取字体使用的字符集合
     * @param font_name 字体名称
     * @return 使用的字符集合
     */
    std::set<std::string> getUsedCharacters(const std::string& font_name) const;

private:
    /**
     * @brief 将UTF-8文本转换为Unicode码点
     * @param text UTF-8文本
     * @param codepoints 输出的码点集合
     */
    void extractCodepoints(const std::string& text, std::set<uint32_t>& codepoints);
    
    /**
     * @brief 执行单个字体的子集化
     * @param usage 字体使用信息
     * @return 子集化结果
     */
    FontSubsetResult performSingleSubsetting(const FontUsage& usage);

    /**
     * @brief 创建基本的字体子集
     * @param font_data 原始字体数据
     * @param used_codepoints 使用的字符码点
     * @return 子集化后的字体数据
     */
    std::vector<std::uint8_t> createBasicSubset(const std::vector<std::uint8_t>& font_data,
                                               const std::set<uint32_t>& used_codepoints);

    /**
     * @brief 使用FreeType创建字体子集
     * @param font_data 原始字体数据
     * @param used_codepoints 使用的字符码点
     * @return 子集化后的字体数据
     */
    std::vector<std::uint8_t> createFreeTypeSubset(const std::vector<std::uint8_t>& font_data,
                                                   const std::set<uint32_t>& used_codepoints);

    /**
     * @brief 重建字体表结构
     * @param face FreeType字体面
     * @param used_glyphs 使用的字形ID集合
     * @return 重建后的字体数据
     */
    std::vector<std::uint8_t> rebuildFontTables(FT_Face face, const std::set<uint16_t>& used_glyphs);

    /**
     * @brief 使用HarfBuzz创建字体子集
     * @param font_data 原始字体数据
     * @param used_glyphs 使用的字形ID集合
     * @return 子集化后的字体数据
     */
    std::vector<std::uint8_t> createHarfBuzzSubset(const std::vector<std::uint8_t>& font_data,
                                                   const std::set<uint16_t>& used_glyphs);

    /**
     * @brief 将Unicode码点转换为字形ID
     * @param face FreeType字体面
     * @param codepoints Unicode码点集合
     * @return 字形ID集合
     */
    std::set<uint16_t> codepointsToGlyphs(FT_Face face, const std::set<uint32_t>& codepoints);
    
    std::map<std::string, FontUsage> font_usages_;        ///< 字体使用信息
    std::map<std::string, FontSubsetResult> subset_results_; ///< 子集化结果

    // FreeType相关
    FT_Library ft_library_;                               ///< FreeType库实例
    bool ft_initialized_;                                 ///< FreeType是否已初始化
    
    // 统计信息
    mutable size_t total_original_size_ = 0;  ///< 总原始大小
    mutable size_t total_subset_size_ = 0;    ///< 总子集大小
    mutable size_t total_fonts_ = 0;          ///< 字体总数
    mutable size_t subsetted_fonts_ = 0;      ///< 子集化字体数
};

} // namespace tinakit::pdf::core
