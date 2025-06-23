#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

// FreeType前向声明
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_* FT_Library;
struct FT_FaceRec_;
typedef struct FT_FaceRec_* FT_Face;

// HarfBuzz前向声明
struct hb_font_t;
struct hb_buffer_t;

namespace tinakit::pdf::core {

/**
 * @brief 字体度量信息
 */
struct FontMetrics {
    int ascent;          ///< 上升高度
    int descent;         ///< 下降高度
    int line_height;     ///< 行高
    int max_advance;     ///< 最大字符宽度
};

/**
 * @brief 字符宽度信息
 */
struct CharacterWidth {
    uint32_t codepoint;  ///< Unicode码点
    uint16_t cid;        ///< CID (Character ID)
    int width;           ///< 字符宽度（单位：字体单位）
};

/**
 * @brief 字体管理器
 * 
 * 使用FreeType和HarfBuzz来动态获取字体信息，
 * 替代硬编码的字符宽度数组
 */
class FontManager {
public:
    /**
     * @brief 构造函数
     */
    FontManager();
    
    /**
     * @brief 析构函数
     */
    ~FontManager();
    
    // 禁用拷贝和移动
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    FontManager(FontManager&&) = delete;
    FontManager& operator=(FontManager&&) = delete;
    
    /**
     * @brief 加载字体文件
     * @param font_path 字体文件路径
     * @param font_name 字体名称（用于标识）
     * @return 成功返回true
     */
    bool loadFont(const std::string& font_path, const std::string& font_name);
    
    /**
     * @brief 加载字体数据
     * @param font_data 字体数据
     * @param font_name 字体名称（用于标识）
     * @return 成功返回true
     */
    bool loadFont(const std::vector<uint8_t>& font_data, const std::string& font_name);
    
    /**
     * @brief 获取字体度量信息
     * @param font_name 字体名称
     * @param font_size 字体大小（点数）
     * @return 字体度量信息
     */
    FontMetrics getFontMetrics(const std::string& font_name, double font_size) const;
    
    /**
     * @brief 获取字符宽度
     * @param font_name 字体名称
     * @param codepoint Unicode码点
     * @param font_size 字体大小（点数）
     * @return 字符宽度（点数）
     */
    double getCharacterWidth(const std::string& font_name, uint32_t codepoint, double font_size) const;
    
    /**
     * @brief 获取文本宽度
     * @param font_name 字体名称
     * @param text UTF-8文本
     * @param font_size 字体大小（点数）
     * @return 文本宽度（点数）
     */
    double getTextWidth(const std::string& font_name, const std::string& text, double font_size) const;

    /**
     * @brief 将文本转换为GID十六进制字符串
     * @param font_name 字体名称
     * @param text UTF-8文本
     * @return GID十六进制字符串（如"<0041><0042>"）
     */
    std::string textToGIDHex(const std::string& font_name, const std::string& text) const;

    /**
     * @brief 检查字体对文本的字符覆盖情况
     * @param font_name 字体名称
     * @param text UTF-8文本
     * @return 缺失字符的数量
     */
    int checkFontCoverage(const std::string& font_name, const std::string& text) const;
    
    /**
     * @brief 生成PDF字符宽度数组
     * @param font_name 字体名称
     * @param font_size 字体大小（点数）
     * @param codepoints 需要包含的码点列表
     * @return PDF格式的宽度数组字符串
     */
    std::string generateWidthArray(const std::string& font_name, double font_size, 
                                 const std::vector<uint32_t>& codepoints) const;
    
    /**
     * @brief 生成ToUnicode CMap
     * @param font_name 字体名称
     * @param codepoints 需要映射的码点列表
     * @return CMap内容字符串
     */
    std::string generateToUnicodeCMap(const std::string& font_name, 
                                    const std::vector<uint32_t>& codepoints) const;
    
    /**
     * @brief 检查字体是否已加载
     * @param font_name 字体名称
     * @return 已加载返回true
     */
    bool isFontLoaded(const std::string& font_name) const;

    /**
     * @brief 获取已加载的字体列表
     * @return 字体名称列表
     */
    std::vector<std::string> getLoadedFonts() const;

    /**
     * @brief 清理缓存
     * @param font_name 字体名称，为空则清理所有缓存
     */
    void clearCache(const std::string& font_name = "") const;

    /**
     * @brief 获取缓存统计信息
     * @return 缓存统计信息
     */
    std::string getCacheStatistics() const;

private:
    struct FontData;

    // 缓存结构
    struct GlyphWidthCacheKey {
        std::string font_name;
        uint32_t glyph_id;

        bool operator<(const GlyphWidthCacheKey& other) const {
            if (font_name != other.font_name) return font_name < other.font_name;
            return glyph_id < other.glyph_id;
        }
    };

    struct TextShapeCacheKey {
        std::string font_name;
        std::string text;

        bool operator<(const TextShapeCacheKey& other) const {
            if (font_name != other.font_name) return font_name < other.font_name;
            return text < other.text;
        }
    };

    FT_Library ft_library_;                           ///< FreeType库实例
    std::map<std::string, std::unique_ptr<FontData>> fonts_;  ///< 已加载的字体

    // 性能优化缓存
    mutable std::map<GlyphWidthCacheKey, int> glyph_width_cache_;  ///< 字符宽度缓存
    mutable std::map<TextShapeCacheKey, std::string> text_shape_cache_;  ///< 文本整形缓存
    mutable std::map<std::string, std::map<uint32_t, int>> font_width_arrays_;  ///< 字体宽度数组缓存
    
    /**
     * @brief 初始化FreeType库
     * @return 成功返回true
     */
    bool initializeFreeType();
    
    /**
     * @brief 清理FreeType库
     */
    void cleanupFreeType();
    
    /**
     * @brief 获取字体数据
     * @param font_name 字体名称
     * @return 字体数据指针，未找到返回nullptr
     */
    FontData* getFontData(const std::string& font_name) const;
    
    /**
     * @brief 将字体单位转换为点数
     * @param font_units 字体单位
     * @param font_size 字体大小（点数）
     * @param units_per_em 每em的单位数
     * @return 点数
     */
    double fontUnitsToPoints(int font_units, double font_size, int units_per_em) const;

    /**
     * @brief 获取字形在PDF单位制下的宽度
     * @param ft_face FreeType字体面
     * @param glyph_index 字形索引
     * @return PDF单位制下的宽度（1000单位 = 1em）
     */
    int getGlyphAdvanceInPdfUnits(FT_FaceRec_* ft_face, unsigned int glyph_index) const;
};

} // namespace tinakit::pdf::core
