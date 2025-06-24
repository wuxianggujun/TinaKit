/**
 * @file font_subset_manager.cpp
 * @brief 字体子集化管理器实现
 * @author TinaKit Team
 * @date 2025-6-23
 */

#include "tinakit/pdf/core/font_subset_manager.hpp"
#include "tinakit/core/logger.hpp"
#include <sstream>
#include <iomanip>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include <hb.h>
#include <hb-subset.h>

namespace tinakit::pdf::core {

FontSubsetManager::FontSubsetManager() : ft_initialized_(false) {
    // 初始化FreeType库
    FT_Error error = FT_Init_FreeType(&ft_library_);
    if (error) {
        PDF_ERROR("Failed to initialize FreeType library: " + std::to_string(error));
        ft_initialized_ = false;
    } else {
        ft_initialized_ = true;
        PDF_DEBUG("FontSubsetManager initialized with FreeType support");
    }
}

FontSubsetManager::~FontSubsetManager() {
    if (ft_initialized_) {
        FT_Done_FreeType(ft_library_);
        PDF_DEBUG("FreeType library cleaned up");
    }
    PDF_DEBUG("FontSubsetManager destroyed");
}

void FontSubsetManager::registerFont(const std::string& font_name,
                                   const std::vector<std::uint8_t>& font_data,
                                   bool enable_subsetting,
                                   bool embed_font) {
    if (font_data.empty()) {
        PDF_ERROR("Empty font data for: " + font_name);
        return;
    }
    
    FontUsage usage;
    usage.font_name = font_name;
    usage.font_data = font_data;
    usage.enable_subsetting = enable_subsetting;
    usage.embed_font = embed_font;
    
    font_usages_[font_name] = std::move(usage);
    
    PDF_DEBUG("Font registered for subset management: " + font_name + 
              " (" + std::to_string(font_data.size()) + " bytes, subsetting: " + 
              (enable_subsetting ? "enabled" : "disabled") + ")");
}

void FontSubsetManager::recordTextUsage(const std::string& font_name, const std::string& text) {
    auto it = font_usages_.find(font_name);
    if (it == font_usages_.end()) {
        PDF_WARN("Font not registered: " + font_name);
        return;
    }
    
    extractCodepoints(text, it->second.used_codepoints);
    
    PDF_DEBUG("Recorded text usage for " + font_name + ": '" + text + 
              "' (total codepoints: " + std::to_string(it->second.used_codepoints.size()) + ")");
}

void FontSubsetManager::recordCodepointUsage(const std::string& font_name, uint32_t codepoint) {
    auto it = font_usages_.find(font_name);
    if (it == font_usages_.end()) {
        PDF_WARN("Font not registered: " + font_name);
        return;
    }
    
    it->second.used_codepoints.insert(codepoint);
}

std::vector<FontSubsetResult> FontSubsetManager::performSubsetting() {
    std::vector<FontSubsetResult> results;
    results.reserve(font_usages_.size());
    
    PDF_DEBUG("Starting font subsetting for " + std::to_string(font_usages_.size()) + " fonts");
    
    for (const auto& [font_name, usage] : font_usages_) {
        FontSubsetResult result = performSingleSubsetting(usage);
        subset_results_[font_name] = result;
        results.push_back(result);
    }
    
    // 更新统计信息
    total_fonts_ = font_usages_.size();
    total_original_size_ = 0;
    total_subset_size_ = 0;
    subsetted_fonts_ = 0;
    
    for (const auto& result : results) {
        total_original_size_ += result.original_size;
        total_subset_size_ += result.subset_size;
        if (result.success && !result.subset_data.empty()) {
            subsetted_fonts_++;
        }
    }
    
    PDF_DEBUG("Font subsetting completed: " + std::to_string(subsetted_fonts_) + 
              "/" + std::to_string(total_fonts_) + " fonts subsetted");
    
    return results;
}

const FontSubsetResult* FontSubsetManager::getSubsetResult(const std::string& font_name) const {
    auto it = subset_results_.find(font_name);
    return (it != subset_results_.end()) ? &it->second : nullptr;
}

std::vector<std::uint8_t> FontSubsetManager::getFinalFontData(const std::string& font_name) const {
    // 首先检查是否有子集化结果
    auto subset_it = subset_results_.find(font_name);
    if (subset_it != subset_results_.end() && subset_it->second.success && 
        !subset_it->second.subset_data.empty()) {
        return subset_it->second.subset_data;
    }
    
    // 如果没有子集化结果，返回原始字体数据
    auto usage_it = font_usages_.find(font_name);
    if (usage_it != font_usages_.end()) {
        return usage_it->second.font_data;
    }
    
    PDF_WARN("Font data not found: " + font_name);
    return {};
}

bool FontSubsetManager::isFontRegistered(const std::string& font_name) const {
    return font_usages_.find(font_name) != font_usages_.end();
}

bool FontSubsetManager::isSubsettingEnabled(const std::string& font_name) const {
    auto it = font_usages_.find(font_name);
    return (it != font_usages_.end()) ? it->second.enable_subsetting : false;
}

std::string FontSubsetManager::getStatistics() const {
    std::ostringstream oss;
    oss << "Font Subset Manager Statistics:\n";
    oss << "  Total Fonts: " << total_fonts_ << "\n";
    oss << "  Subsetted Fonts: " << subsetted_fonts_ << "\n";
    oss << "  Original Total Size: " << total_original_size_ << " bytes\n";
    oss << "  Subset Total Size: " << total_subset_size_ << " bytes\n";
    
    if (total_original_size_ > 0) {
        double compression_ratio = 100.0 * total_subset_size_ / total_original_size_;
        oss << "  Overall Compression: " << std::fixed << std::setprecision(1) 
            << compression_ratio << "%\n";
        oss << "  Size Reduction: " << (total_original_size_ - total_subset_size_) << " bytes\n";
    }
    
    return oss.str();
}

void FontSubsetManager::clear() {
    font_usages_.clear();
    subset_results_.clear();
    total_original_size_ = 0;
    total_subset_size_ = 0;
    total_fonts_ = 0;
    subsetted_fonts_ = 0;
    
    PDF_DEBUG("FontSubsetManager cleared");
}

void FontSubsetManager::extractCodepoints(const std::string& text, std::set<uint32_t>& codepoints) {
    for (size_t i = 0; i < text.length(); ) {
        uint32_t codepoint = 0;
        size_t bytes = 1;
        
        unsigned char c = static_cast<unsigned char>(text[i]);
        
        if (c < 0x80) {
            // ASCII字符
            codepoint = c;
            bytes = 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2字节UTF-8
            if (i + 1 < text.length()) {
                codepoint = ((c & 0x1F) << 6) | (text[i + 1] & 0x3F);
                bytes = 2;
            }
        } else if ((c & 0xF0) == 0xE0) {
            // 3字节UTF-8
            if (i + 2 < text.length()) {
                codepoint = ((c & 0x0F) << 12) | 
                           ((text[i + 1] & 0x3F) << 6) | 
                           (text[i + 2] & 0x3F);
                bytes = 3;
            }
        } else if ((c & 0xF8) == 0xF0) {
            // 4字节UTF-8
            if (i + 3 < text.length()) {
                codepoint = ((c & 0x07) << 18) | 
                           ((text[i + 1] & 0x3F) << 12) | 
                           ((text[i + 2] & 0x3F) << 6) | 
                           (text[i + 3] & 0x3F);
                bytes = 4;
            }
        }
        
        if (codepoint > 0) {
            codepoints.insert(codepoint);
        }
        
        i += bytes;
    }
}

FontSubsetResult FontSubsetManager::performSingleSubsetting(const FontUsage& usage) {
    FontSubsetResult result;
    result.font_name = usage.font_name;
    result.original_size = usage.font_data.size();
    result.codepoints_count = usage.used_codepoints.size();
    result.success = false;
    
    if (!usage.enable_subsetting || usage.used_codepoints.empty()) {
        PDF_DEBUG("Skipping subsetting for " + usage.font_name + 
                  " (subsetting disabled or no codepoints)");
        result.subset_data = usage.font_data;
        result.subset_size = usage.font_data.size();
        result.success = true;
        return result;
    }
    
    PDF_DEBUG("Creating subset for " + usage.font_name + " with " +
              std::to_string(usage.used_codepoints.size()) + " codepoints");

    // 实现真正的字体子集化
    if (ft_initialized_ && usage.used_codepoints.size() < 20000) {  // 放宽阈值，允许更多字体进行子集化
        result.subset_data = createFreeTypeSubset(usage.font_data, usage.used_codepoints);
        result.subset_size = result.subset_data.size();

        if (result.subset_size < result.original_size && !result.subset_data.empty()) {
            result.success = true;
            double compression_ratio = 100.0 * result.subset_size / result.original_size;
            PDF_DEBUG("Font subset created for " + usage.font_name + ": " +
                      std::to_string(result.subset_size) + " bytes (" +
                      std::to_string(compression_ratio) + "% of original)");
        } else {
            // 子集化没有减少大小或失败，使用原始字体
            result.subset_data = usage.font_data;
            result.subset_size = usage.font_data.size();
            result.success = true;
            PDF_DEBUG("Subset not beneficial or failed, using original font for: " + usage.font_name);
        }
    } else {
        // 使用字符太多或FreeType未初始化，不进行子集化
        result.subset_data = usage.font_data;
        result.subset_size = usage.font_data.size();
        result.success = true;
        PDF_DEBUG("Skipping subsetting for: " + usage.font_name +
                  " (FreeType: " + (ft_initialized_ ? "OK" : "Failed") +
                  ", Codepoints: " + std::to_string(usage.used_codepoints.size()) + ")");
    }
    
    return result;
}

std::vector<std::uint8_t> FontSubsetManager::createFreeTypeSubset(const std::vector<std::uint8_t>& font_data,
                                                                 const std::set<uint32_t>& used_codepoints) {
    if (!ft_initialized_) {
        PDF_ERROR("FreeType not initialized");
        return font_data;
    }

    PDF_DEBUG("Creating FreeType subset with " + std::to_string(used_codepoints.size()) + " codepoints");

    FT_Face face;
    FT_Error error = FT_New_Memory_Face(ft_library_,
                                       font_data.data(),
                                       static_cast<FT_Long>(font_data.size()),
                                       0, &face);

    if (error) {
        PDF_ERROR("Failed to load font face: " + std::to_string(error));
        return font_data;
    }

    // 将Unicode码点转换为字形ID
    std::set<uint16_t> used_glyphs = codepointsToGlyphs(face, used_codepoints);

    // 添加必要的字形（.notdef等）
    used_glyphs.insert(0);  // .notdef字形

    PDF_DEBUG("Converted " + std::to_string(used_codepoints.size()) +
              " codepoints to " + std::to_string(used_glyphs.size()) + " glyphs");

    // 重建字体表
    std::vector<std::uint8_t> subset_data = rebuildFontTables(face, used_glyphs);

    FT_Done_Face(face);

    if (subset_data.empty()) {
        PDF_WARN("Font subsetting failed, returning original font");
        return font_data;
    }

    PDF_DEBUG("FreeType subset created: " + std::to_string(subset_data.size()) +
              " bytes (reduced from " + std::to_string(font_data.size()) + " bytes)");

    return subset_data;
}

std::set<uint16_t> FontSubsetManager::codepointsToGlyphs(FT_Face face, const std::set<uint32_t>& codepoints) {
    std::set<uint16_t> glyphs;

    for (uint32_t codepoint : codepoints) {
        FT_UInt glyph_index = FT_Get_Char_Index(face, codepoint);
        if (glyph_index != 0) {  // 0表示字符不存在
            glyphs.insert(static_cast<uint16_t>(glyph_index));
        }
    }

    return glyphs;
}

std::vector<std::uint8_t> FontSubsetManager::rebuildFontTables(FT_Face face, const std::set<uint16_t>& used_glyphs) {
    PDF_DEBUG("Creating true font subset with " + std::to_string(used_glyphs.size()) + " glyphs");

    // 获取原始字体数据
    FT_ULong font_size = 0;
    FT_Error error = FT_Load_Sfnt_Table(face, 0, 0, nullptr, &font_size);
    if (error) {
        PDF_ERROR("Failed to get font size: " + std::to_string(error));
        return {};
    }

    std::vector<std::uint8_t> original_data(font_size);
    error = FT_Load_Sfnt_Table(face, 0, 0, original_data.data(), &font_size);
    if (error) {
        PDF_ERROR("Failed to load font data: " + std::to_string(error));
        return {};
    }

    // 使用HarfBuzz进行真正的字体子集化
    std::vector<std::uint8_t> subset_data = createHarfBuzzSubset(original_data, used_glyphs);

    if (subset_data.empty() || subset_data.size() >= original_data.size()) {
        PDF_WARN("HarfBuzz subsetting failed or not beneficial, using original font");
        return original_data;
    }

    double compression_ratio = 100.0 * subset_data.size() / original_data.size();
    PDF_DEBUG("Font subset created: " + std::to_string(subset_data.size()) +
              " bytes (" + std::to_string(compression_ratio) + "% of original)");
    PDF_DEBUG("Used glyphs: " + std::to_string(used_glyphs.size()) +
              " out of " + std::to_string(face->num_glyphs) + " total glyphs");

    return subset_data;
}

std::vector<std::uint8_t> FontSubsetManager::createHarfBuzzSubset(const std::vector<std::uint8_t>& font_data,
                                                                 const std::set<uint16_t>& used_glyphs) {
    PDF_DEBUG("Creating HarfBuzz subset with " + std::to_string(used_glyphs.size()) + " glyphs");

    // 创建HarfBuzz blob
    hb_blob_t* blob = hb_blob_create(reinterpret_cast<const char*>(font_data.data()),
                                    font_data.size(),
                                    HB_MEMORY_MODE_READONLY,
                                    nullptr, nullptr);
    if (!blob) {
        PDF_ERROR("Failed to create HarfBuzz blob");
        return {};
    }

    // 创建HarfBuzz face
    hb_face_t* face = hb_face_create(blob, 0);
    if (!face) {
        PDF_ERROR("Failed to create HarfBuzz face");
        hb_blob_destroy(blob);
        return {};
    }

    // 创建子集化输入
    hb_subset_input_t* input = hb_subset_input_create_or_fail();
    if (!input) {
        PDF_ERROR("Failed to create HarfBuzz subset input");
        hb_face_destroy(face);
        hb_blob_destroy(blob);
        return {};
    }

    // 配置子集化选项
    hb_set_t* glyph_set = hb_subset_input_glyph_set(input);

    // 添加使用的字形ID
    for (uint16_t gid : used_glyphs) {
        hb_set_add(glyph_set, gid);
    }

    // 设置保留GID选项（相当于fonttools的--retain-gids）
    hb_subset_input_set_flags(input, HB_SUBSET_FLAGS_RETAIN_GIDS);

    PDF_DEBUG("HarfBuzz subset configured with " + std::to_string(used_glyphs.size()) + " glyphs, retain-gids enabled");

    // 执行子集化
    hb_face_t* subset_face = hb_subset_or_fail(face, input);
    if (!subset_face) {
        PDF_ERROR("HarfBuzz subsetting failed");
        hb_subset_input_destroy(input);
        hb_face_destroy(face);
        hb_blob_destroy(blob);
        return {};
    }

    // 获取子集化后的数据
    hb_blob_t* subset_blob = hb_face_reference_blob(subset_face);
    if (!subset_blob) {
        PDF_ERROR("Failed to get subset blob");
        hb_face_destroy(subset_face);
        hb_subset_input_destroy(input);
        hb_face_destroy(face);
        hb_blob_destroy(blob);
        return {};
    }

    // 复制数据到vector
    unsigned int subset_size;
    const char* subset_data_ptr = hb_blob_get_data(subset_blob, &subset_size);
    std::vector<std::uint8_t> subset_data(subset_data_ptr, subset_data_ptr + subset_size);

    // 清理资源
    hb_blob_destroy(subset_blob);
    hb_face_destroy(subset_face);
    hb_subset_input_destroy(input);
    hb_face_destroy(face);
    hb_blob_destroy(blob);

    PDF_DEBUG("HarfBuzz subset completed: " + std::to_string(subset_data.size()) + " bytes");
    return subset_data;
}

std::vector<std::uint8_t> FontSubsetManager::createBasicSubset(const std::vector<std::uint8_t>& font_data,
                                                              const std::set<uint32_t>& used_codepoints) {
    // 这个方法现在作为HarfBuzz方法的后备
    PDF_DEBUG("Creating basic subset with " + std::to_string(used_codepoints.size()) + " codepoints");
    return font_data;  // 简单返回原始数据
}

std::set<std::string> FontSubsetManager::getUsedCharacters(const std::string& font_name) const {
    std::set<std::string> characters;

    auto it = font_usages_.find(font_name);
    if (it != font_usages_.end()) {
        const auto& usage = it->second;

        // 从使用的码点重构字符
        for (uint32_t codepoint : usage.used_codepoints) {
            // 将Unicode码点转换为UTF-8字符串
            std::string utf8_char;
            if (codepoint <= 0x7F) {
                // ASCII字符
                utf8_char = static_cast<char>(codepoint);
            } else if (codepoint <= 0x7FF) {
                // 2字节UTF-8
                utf8_char += static_cast<char>(0xC0 | (codepoint >> 6));
                utf8_char += static_cast<char>(0x80 | (codepoint & 0x3F));
            } else if (codepoint <= 0xFFFF) {
                // 3字节UTF-8
                utf8_char += static_cast<char>(0xE0 | (codepoint >> 12));
                utf8_char += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                utf8_char += static_cast<char>(0x80 | (codepoint & 0x3F));
            } else {
                // 4字节UTF-8
                utf8_char += static_cast<char>(0xF0 | (codepoint >> 18));
                utf8_char += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                utf8_char += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                utf8_char += static_cast<char>(0x80 | (codepoint & 0x3F));
            }
            characters.insert(utf8_char);
        }
    }

    return characters;
}

} // namespace tinakit::pdf::core
