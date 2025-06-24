#include "tinakit/pdf/core/font_manager.hpp"
#include "tinakit/core/logger.hpp"
#include "tinakit/core/unicode.hpp"
#include <utf8.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <hb.h>
#include <hb-ft.h>

#include <sstream>
#include <iomanip>
#include <algorithm>

namespace tinakit::pdf::core {

/**
 * @brief 字体数据结构
 */
struct FontManager::FontData {
    FT_Face ft_face;                    ///< FreeType字体面
    hb_font_t* hb_font;                ///< HarfBuzz字体
    std::vector<uint8_t> font_data;    ///< 字体数据（用于保持内存有效）
    std::string font_name;             ///< 字体名称
    
    FontData() : ft_face(nullptr), hb_font(nullptr) {}
    
    ~FontData() {
        if (hb_font) {
            hb_font_destroy(hb_font);
        }
        if (ft_face) {
            FT_Done_Face(ft_face);
        }
    }
};

FontManager::FontManager() : ft_library_(nullptr) {
    if (!initializeFreeType()) {
        CORE_ERROR("Failed to initialize FreeType library");
    }
}

FontManager::~FontManager() {
    fonts_.clear();  // 先清理字体数据
    cleanupFreeType();
}

bool FontManager::initializeFreeType() {
    FT_Error error = FT_Init_FreeType(&ft_library_);
    if (error) {
        CORE_ERROR("FreeType initialization failed with error: " + std::to_string(error));
        return false;
    }
    
    CORE_DEBUG("FreeType library initialized successfully");
    return true;
}

void FontManager::cleanupFreeType() {
    if (ft_library_) {
        FT_Done_FreeType(ft_library_);
        ft_library_ = nullptr;
        CORE_DEBUG("FreeType library cleaned up");
    }
}

bool FontManager::loadFont(const std::string& font_path, const std::string& font_name) {
    if (!ft_library_) {
        CORE_ERROR("FreeType library not initialized");
        return false;
    }
    
    auto font_data = std::make_unique<FontData>();
    font_data->font_name = font_name;
    
    // 加载字体文件
    FT_Error error = FT_New_Face(ft_library_, font_path.c_str(), 0, &font_data->ft_face);
    if (error) {
        CORE_ERROR("Failed to load font from path: " + font_path + ", error: " + std::to_string(error));
        return false;
    }

    // 调试信息：检查字体基本信息
    CORE_DEBUG("Font loaded - family: " + std::string(font_data->ft_face->family_name ? font_data->ft_face->family_name : "Unknown") +
               ", style: " + std::string(font_data->ft_face->style_name ? font_data->ft_face->style_name : "Unknown") +
               ", units_per_EM: " + std::to_string(font_data->ft_face->units_per_EM) +
               ", num_faces: " + std::to_string(font_data->ft_face->num_faces) +
               ", face_index: " + std::to_string(font_data->ft_face->face_index));

    // 对于TTC文件，检查字符宽度是否正常
    if (font_data->ft_face->num_faces > 1) {
        CORE_DEBUG("TTC file detected with " + std::to_string(font_data->ft_face->num_faces) + " faces");

        // 测试一个常见字符的宽度，检查是否正常
        FT_UInt test_gid = FT_Get_Char_Index(font_data->ft_face, 'A');
        if (test_gid != 0) {
            FT_Error test_error = FT_Load_Glyph(font_data->ft_face, test_gid, FT_LOAD_NO_HINTING | FT_LOAD_NO_SCALE);
            if (test_error == 0) {
                double test_advance = static_cast<double>(font_data->ft_face->glyph->advance.x) / 64.0;
                CORE_DEBUG("Test character 'A' advance: " + std::to_string(test_advance) + " font-units");

                // 如果所有字符都是相同的很小的宽度，说明这个字体面有问题
                if (test_advance <= 5.0) {
                    CORE_WARN("Face 0 appears to have invalid glyph metrics, trying face 1");

                    FT_Done_Face(font_data->ft_face);
                    error = FT_New_Face(ft_library_, font_path.c_str(), 1, &font_data->ft_face);
                    if (error == 0) {
                        CORE_DEBUG("Loaded face 1 - family: " + std::string(font_data->ft_face->family_name ? font_data->ft_face->family_name : "Unknown") +
                                   ", units_per_EM: " + std::to_string(font_data->ft_face->units_per_EM));

                        // 再次测试字符宽度
                        FT_UInt test_gid2 = FT_Get_Char_Index(font_data->ft_face, 'A');
                        if (test_gid2 != 0) {
                            FT_Error test_error2 = FT_Load_Glyph(font_data->ft_face, test_gid2, FT_LOAD_NO_HINTING | FT_LOAD_NO_SCALE);
                            if (test_error2 == 0) {
                                double test_advance2 = static_cast<double>(font_data->ft_face->glyph->advance.x) / 64.0;
                                CORE_DEBUG("Face 1 - Test character 'A' advance: " + std::to_string(test_advance2) + " font-units");
                            }
                        }
                    } else {
                        CORE_ERROR("Failed to load face 1, reverting to face 0");
                        error = FT_New_Face(ft_library_, font_path.c_str(), 0, &font_data->ft_face);
                        if (error) {
                            CORE_ERROR("Failed to revert to face 0");
                            return false;
                        }
                    }
                }
            }
        }
    }
    
    // 强制选择Unicode charmap
    FT_Error charmap_error = FT_Select_Charmap(font_data->ft_face, FT_ENCODING_UNICODE);
    if (charmap_error) {
        CORE_WARN("Font has no Unicode charmap, may have limited character support: " + font_name);
    } else {
        CORE_DEBUG("Successfully selected Unicode charmap for: " + font_name);
    }

    // 创建HarfBuzz字体
    font_data->hb_font = hb_ft_font_create(font_data->ft_face, nullptr);
    if (!font_data->hb_font) {
        CORE_ERROR("Failed to create HarfBuzz font for: " + font_name);
        return false;
    }
    
    fonts_[font_name] = std::move(font_data);
    CORE_INFO("Font loaded successfully: " + font_name + " from " + font_path);
    return true;
}

bool FontManager::loadFont(const std::vector<uint8_t>& font_data_vec, const std::string& font_name) {
    if (!ft_library_) {
        CORE_ERROR("FreeType library not initialized");
        return false;
    }
    
    auto font_data = std::make_unique<FontData>();
    font_data->font_name = font_name;
    font_data->font_data = font_data_vec;  // 保存数据副本
    
    // 从内存加载字体
    FT_Error error = FT_New_Memory_Face(ft_library_, 
                                       font_data->font_data.data(), 
                                       static_cast<FT_Long>(font_data->font_data.size()), 
                                       0, 
                                       &font_data->ft_face);
    if (error) {
        CORE_ERROR("Failed to load font from memory: " + font_name + ", error: " + std::to_string(error));
        return false;
    }
    
    // 创建HarfBuzz字体
    font_data->hb_font = hb_ft_font_create(font_data->ft_face, nullptr);
    if (!font_data->hb_font) {
        CORE_ERROR("Failed to create HarfBuzz font for: " + font_name);
        return false;
    }
    
    fonts_[font_name] = std::move(font_data);
    CORE_INFO("Font loaded successfully from memory: " + font_name);
    return true;
}

FontMetrics FontManager::getFontMetrics(const std::string& font_name, double font_size) const {
    FontMetrics metrics = {};
    
    FontData* font = getFontData(font_name);
    if (!font) {
        CORE_ERROR("Font not found: " + font_name);
        return metrics;
    }
    
    // 设置字体大小
    FT_Error error = FT_Set_Char_Size(font->ft_face, 0, static_cast<FT_F26Dot6>(font_size * 64), 72, 72);
    if (error) {
        CORE_ERROR("Failed to set font size for: " + font_name);
        return metrics;
    }
    
    FT_Face face = font->ft_face;
    double scale = font_size / face->units_per_EM;
    
    metrics.ascent = static_cast<int>(face->ascender * scale);
    metrics.descent = static_cast<int>(face->descender * scale);
    metrics.line_height = static_cast<int>(face->height * scale);
    metrics.max_advance = static_cast<int>(face->max_advance_width * scale);
    
    return metrics;
}

double FontManager::getCharacterWidth(const std::string& font_name, uint32_t codepoint, double font_size) const {
    FontData* font = getFontData(font_name);
    if (!font) {
        CORE_ERROR("Font not found: " + font_name);
        return 0.0;
    }
    
    // 设置字体大小
    FT_Error error = FT_Set_Char_Size(font->ft_face, 0, static_cast<FT_F26Dot6>(font_size * 64), 72, 72);
    if (error) {
        CORE_ERROR("Failed to set font size for: " + font_name);
        return 0.0;
    }
    
    // 获取字符索引
    FT_UInt glyph_index = FT_Get_Char_Index(font->ft_face, codepoint);
    if (glyph_index == 0) {
        CORE_WARN("Character not found in font: U+" + std::to_string(codepoint) + " in " + font_name);
        return 0.0;
    }
    
    // 加载字形
    error = FT_Load_Glyph(font->ft_face, glyph_index, FT_LOAD_DEFAULT);
    if (error) {
        CORE_ERROR("Failed to load glyph for character: U+" + std::to_string(codepoint));
        return 0.0;
    }
    
    // 返回字符宽度（转换为点数）
    return font->ft_face->glyph->advance.x / 64.0;
}

double FontManager::getTextWidth(const std::string& font_name, const std::string& text, double font_size) const {
    FontData* font = getFontData(font_name);
    if (!font) {
        CORE_ERROR("Font not found: " + font_name);
        return 0.0;
    }
    
    // 使用HarfBuzz进行文本整形
    hb_buffer_t* buffer = hb_buffer_create();
    hb_buffer_add_utf8(buffer, text.c_str(), -1, 0, -1);
    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
    hb_buffer_set_script(buffer, HB_SCRIPT_COMMON);
    hb_buffer_set_language(buffer, hb_language_from_string("en", -1));
    
    // 设置字体大小
    hb_font_set_scale(font->hb_font, 
                     static_cast<int>(font_size * 64), 
                     static_cast<int>(font_size * 64));
    
    // 进行文本整形
    hb_shape(font->hb_font, buffer, nullptr, 0);
    
    // 获取整形结果
    unsigned int glyph_count;
    hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
    hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);
    
    // 计算总宽度
    double total_width = 0.0;
    for (unsigned int i = 0; i < glyph_count; i++) {
        total_width += glyph_pos[i].x_advance / 64.0;
    }
    
    hb_buffer_destroy(buffer);
    return total_width;
}

FontManager::FontData* FontManager::getFontData(const std::string& font_name) const {
    auto it = fonts_.find(font_name);
    return (it != fonts_.end()) ? it->second.get() : nullptr;
}

double FontManager::fontUnitsToPoints(int font_units, double font_size, int units_per_em) const {
    return (static_cast<double>(font_units) * font_size) / units_per_em;
}

int FontManager::getGlyphAdvanceInPdfUnits(FT_Face ft_face, unsigned int glyph_index) const {
    // 获取字体名称用于缓存
    std::string font_name = ft_face->family_name ? ft_face->family_name : "unknown";

    // 检查缓存
    GlyphWidthCacheKey cache_key{font_name, glyph_index};
    auto cache_it = glyph_width_cache_.find(cache_key);
    if (cache_it != glyph_width_cache_.end()) {
        return cache_it->second;
    }

    // 保存原始字体大小设置
    FT_Size_Metrics original_metrics = ft_face->size->metrics;

    // 方法1：不缩放加载（标准方式）
    FT_Error error1 = FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_NO_HINTING | FT_LOAD_NO_SCALE);
    double advance1 = 0.0;
    if (error1 == 0) {
        advance1 = static_cast<double>(ft_face->glyph->advance.x);
    }

    // 检查是否需要使用备用方法
    bool use_scaled_method = (advance1 <= 5.0 || error1 != 0);

    int width = 1000;  // 默认宽度

    if (use_scaled_method) {
        // 方法2：设置字体大小后加载
        FT_Error size_error = FT_Set_Char_Size(ft_face, 0, ft_face->units_per_EM * 64, 72, 72);
        FT_Error error2 = FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_NO_HINTING);

        if (error2 == 0 && size_error == 0) {
            // 这时 advance.x 已经是以 units_per_EM 为单位的值
            double advance2 = static_cast<double>(ft_face->glyph->advance.x) / 64.0;

            // 转换为PDF的1000单位制
            width = static_cast<int>((advance2 * 1000.0) / ft_face->units_per_EM + 0.5);
        }

        // 恢复原始字体大小设置
        if (original_metrics.x_ppem > 0) {
            FT_Set_Char_Size(ft_face, 0, original_metrics.y_ppem * 64, 72, 72);
        }
    } else {
        // 使用标准方法：转换为PDF的1000单位制
        width = static_cast<int>((advance1 * 1000.0) / ft_face->units_per_EM + 0.5);
    }

    // 缓存结果
    glyph_width_cache_[cache_key] = width;

    return width;
}

bool FontManager::isFontLoaded(const std::string& font_name) const {
    return fonts_.find(font_name) != fonts_.end();
}

std::vector<std::string> FontManager::getLoadedFonts() const {
    std::vector<std::string> font_names;
    font_names.reserve(fonts_.size());

    for (const auto& [name, data] : fonts_) {
        font_names.push_back(name);
    }

    return font_names;
}

std::string FontManager::textToGIDHex(const std::string& font_name, const std::string& text) const {
    // 检查缓存
    TextShapeCacheKey cache_key{font_name, text};
    auto cache_it = text_shape_cache_.find(cache_key);
    if (cache_it != text_shape_cache_.end()) {
        return cache_it->second;
    }

    FontData* font = getFontData(font_name);
    if (!font) {
        CORE_ERROR("Font not found: " + font_name);
        return "";
    }

    // 修复：输出真正的GID，而不是Unicode CID
    // 这样可以与/CIDToGIDMap /Identity正确配合工作
    std::ostringstream oss;
    oss << "<";

    try {
        // 将UTF-8文本转换为Unicode码点序列
        std::vector<uint32_t> codepoints;
        auto it = text.begin();
        while (it != text.end()) {
            uint32_t cp = utf8::next(it, text.end());
            codepoints.push_back(cp);
        }

        // 将每个Unicode码点转换为对应的GID
        for (uint32_t codepoint : codepoints) {
            FT_UInt gid = FT_Get_Char_Index(font->ft_face, codepoint);
            if (gid == 0) {
                CORE_WARN("Character U+" + std::to_string(codepoint) + " not found in font " + font_name);
                gid = 0;  // 使用.notdef
            }

            // 确保GID在2字节范围内
            if (gid > 0xFFFF) {
                CORE_WARN("GID " + std::to_string(gid) + " exceeds 2-byte limit, using .notdef");
                gid = 0;
            }

            oss << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << gid;
        }
    } catch (const std::exception& e) {
        CORE_ERROR("Failed to convert text to GID: " + std::string(e.what()));
        oss.str("");
        oss << "<";
        // 回退：对于ASCII字符，GID通常等于码点
        for (unsigned char c : text) {
            if (c < 128) {
                oss << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << static_cast<int>(c);
            } else {
                oss << "0000";  // .notdef
            }
        }
    }

    oss << ">";
    std::string result = oss.str();

    // 缓存结果
    text_shape_cache_[cache_key] = result;

    CORE_DEBUG("Text '" + text + "' converted to GID hex: " + result);

    return result;
}

int FontManager::checkFontCoverage(const std::string& font_name, const std::string& text) const {
    FontData* font = getFontData(font_name);
    if (!font) {
        CORE_ERROR("Font not found: " + font_name);
        return -1;
    }

    // 使用HarfBuzz进行文本整形
    hb_buffer_t* buffer = hb_buffer_create();
    hb_buffer_add_utf8(buffer, text.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties(buffer);
    hb_shape(font->hb_font, buffer, nullptr, 0);

    // 获取整形后的结果
    unsigned int glyph_count;
    hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);

    int missing_count = 0;
    for (unsigned int i = 0; i < glyph_count; i++) {
        uint32_t gid = glyph_info[i].codepoint;
        if (gid == 0) {
            missing_count++;
        }
    }

    hb_buffer_destroy(buffer);

    if (missing_count > 0) {
        CORE_WARN("Font '" + font_name + "' is missing " + std::to_string(missing_count) +
                  " out of " + std::to_string(glyph_count) + " glyphs for text: '" + text + "'");
    }

    return missing_count;
}

std::string FontManager::generateWidthArray(const std::string& font_name, double font_size,
                                           const std::vector<uint32_t>& codepoints) const {
    FontData* font = getFontData(font_name);
    if (!font) {
        CORE_ERROR("Font not found: " + font_name);
        return "[]";
    }

    std::ostringstream oss;
    oss << "[ ";

    // 按码点排序
    std::vector<uint32_t> sorted_codepoints = codepoints;
    std::sort(sorted_codepoints.begin(), sorted_codepoints.end());

    // 修复：生成UTF-16BE码点->宽度映射，与内容流的UTF-16BE编码保持一致
    std::map<uint32_t, int> utf16_to_width;  // UTF-16BE码点 -> width mapping

    for (uint32_t codepoint : sorted_codepoints) {
        // 获取字符对应的GID
        FT_UInt glyph_index = FT_Get_Char_Index(font->ft_face, codepoint);
        if (glyph_index != 0) {
            // 直接使用字体库中的真实字符宽度，不做任何调整
            int width = getGlyphAdvanceInPdfUnits(font->ft_face, glyph_index);

            // 对于UTF-16BE编码，使用Unicode码点作为索引
            if (codepoint <= 0xFFFF) {
                utf16_to_width[codepoint] = width;
            } else {
                // 对于超出BMP的字符，需要代理对
                uint32_t adjusted = codepoint - 0x10000;
                uint16_t high = 0xD800 + (adjusted >> 10);
                uint16_t low = 0xDC00 + (adjusted & 0x3FF);
                utf16_to_width[high] = width;
                utf16_to_width[low] = width;
            }

            // 调试：输出字符宽度信息
            CORE_DEBUG("Character U+" + std::to_string(codepoint) + " (GID " + std::to_string(glyph_index) +
                      ") width: " + std::to_string(width) + " PDF units");
        }
    }

    // 按UTF-16BE码点排序输出宽度数组
    for (const auto& [utf16_code, width] : utf16_to_width) {
        oss << utf16_code << " [" << width << "] ";
    }

    oss << "]";

    CORE_DEBUG("Generated width array for " + font_name + " with " + std::to_string(sorted_codepoints.size()) + " characters");
    return oss.str();
}

std::string FontManager::generateToUnicodeCMap(const std::string& font_name,
                                             const std::vector<uint32_t>& codepoints) const {
    FontData* font = getFontData(font_name);
    if (!font) {
        CORE_ERROR("Font not found: " + font_name);
        return "";
    }

    std::ostringstream oss;

    oss << "/CIDInit /ProcSet findresource begin\n";
    oss << "12 dict begin\n";
    oss << "begincmap\n";
    oss << "/CIDSystemInfo\n";
    oss << "<< /Registry (Adobe)\n";
    oss << "   /Ordering (UCS)\n";
    oss << "   /Supplement 0\n";
    oss << ">> def\n";
    oss << "/CMapName /Adobe-Identity-UCS def\n";
    oss << "/CMapType 2 def\n";
    oss << "1 begincodespacerange\n";
    oss << "<0000> <FFFF>\n";  // UTF-16BE的基本多文种平面
    oss << "endcodespacerange\n";

    // 修复：生成UTF-16BE码点到Unicode的映射（因为现在内容流使用UTF-16BE编码）
    std::vector<std::pair<uint32_t, uint32_t>> utf16_to_unicode;
    for (uint32_t codepoint : codepoints) {
        // 对于UTF-16BE编码，码点直接映射到自身
        if (codepoint <= 0xFFFF) {
            utf16_to_unicode.emplace_back(codepoint, codepoint);
        } else {
            // 对于超出BMP的字符，需要代理对
            uint32_t adjusted = codepoint - 0x10000;
            uint16_t high = 0xD800 + (adjusted >> 10);
            uint16_t low = 0xDC00 + (adjusted & 0x3FF);
            utf16_to_unicode.emplace_back(high, codepoint);
            utf16_to_unicode.emplace_back(low, codepoint);
        }
    }

    // 添加字符映射：UTF-16BE码点 -> Unicode
    oss << utf16_to_unicode.size() << " beginbfchar\n";

    for (const auto& [utf16_code, unicode] : utf16_to_unicode) {
        oss << "<" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << utf16_code << "> ";
        oss << "<" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << unicode << ">\n";
    }

    oss << "endbfchar\n";
    oss << "endcmap\n";
    oss << "CMapName currentdict /CMap defineresource pop\n";
    oss << "end\n";
    oss << "end\n";

    CORE_DEBUG("Generated ToUnicode CMap for " + font_name + " with " + std::to_string(utf16_to_unicode.size()) + " UTF-16BE->Unicode mappings");
    return oss.str();
}

void FontManager::clearCache(const std::string& font_name) const {
    if (font_name.empty()) {
        // 清理所有缓存
        glyph_width_cache_.clear();
        text_shape_cache_.clear();
        font_width_arrays_.clear();
        CORE_DEBUG("All FontManager caches cleared");
    } else {
        // 清理指定字体的缓存
        auto glyph_it = glyph_width_cache_.begin();
        while (glyph_it != glyph_width_cache_.end()) {
            if (glyph_it->first.font_name == font_name) {
                glyph_it = glyph_width_cache_.erase(glyph_it);
            } else {
                ++glyph_it;
            }
        }

        auto text_it = text_shape_cache_.begin();
        while (text_it != text_shape_cache_.end()) {
            if (text_it->first.font_name == font_name) {
                text_it = text_shape_cache_.erase(text_it);
            } else {
                ++text_it;
            }
        }

        font_width_arrays_.erase(font_name);

        CORE_DEBUG("FontManager cache cleared for font: " + font_name);
    }
}

std::string FontManager::getCacheStatistics() const {
    std::ostringstream oss;
    oss << "FontManager Cache Statistics:\n";
    oss << "  Glyph Width Cache: " << glyph_width_cache_.size() << " entries\n";
    oss << "  Text Shape Cache: " << text_shape_cache_.size() << " entries\n";
    oss << "  Font Width Arrays: " << font_width_arrays_.size() << " fonts\n";

    // 按字体统计
    std::map<std::string, int> glyph_count_by_font;
    std::map<std::string, int> text_count_by_font;

    for (const auto& [key, value] : glyph_width_cache_) {
        glyph_count_by_font[key.font_name]++;
    }

    for (const auto& [key, value] : text_shape_cache_) {
        text_count_by_font[key.font_name]++;
    }

    oss << "  Per-font breakdown:\n";
    for (const auto& [font_name, data] : fonts_) {
        oss << "    " << font_name << ": "
            << glyph_count_by_font[font_name] << " glyphs, "
            << text_count_by_font[font_name] << " texts\n";
    }

    return oss.str();
}

FT_Face FontManager::getFontFace(const std::string& font_name) const {
    FontData* font = getFontData(font_name);
    return font ? font->ft_face : nullptr;
}

} // namespace tinakit::pdf::core
