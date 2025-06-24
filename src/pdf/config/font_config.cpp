/**
 * @file font_config.cpp
 * @brief 字体配置实现
 * @author TinaKit Team
 * @date 2025-6-24
 */

#include "tinakit/pdf/config/font_config.hpp"
#include <optional>

namespace tinakit::pdf::config {

// ========================================
// FontConfig 类实现
// ========================================

FontConfig::FontConfig(FontEmbeddingStrategy strategy)
    : strategy_(strategy), enable_fallback_(false), preload_system_fonts_(false) {
}

FontConfig& FontConfig::setEmbeddingStrategy(FontEmbeddingStrategy strategy) {
    strategy_ = strategy;
    return *this;
}

FontEmbeddingStrategy FontConfig::getEmbeddingStrategy() const {
    return strategy_;
}

FontConfig& FontConfig::setSubsetOptions(const FontSubsetOptions& options) {
    subset_options_ = options;
    return *this;
}

const FontSubsetOptions& FontConfig::getSubsetOptions() const {
    return subset_options_;
}

FontConfig& FontConfig::addFontFallback(const std::string& primary_font, 
                                       const std::vector<std::string>& fallback_fonts) {
    // TODO: 实现字体回退映射
    (void)primary_font;
    (void)fallback_fonts;
    return *this;
}

FontConfig& FontConfig::enableFontFallback(bool enable) {
    enable_fallback_ = enable;
    return *this;
}

FontConfig& FontConfig::preloadSystemFonts(bool enable) {
    preload_system_fonts_ = enable;
    return *this;
}

FontConfig& FontConfig::addFontSearchPath(const std::string& path) {
    font_search_paths_.push_back(path);
    return *this;
}

FontConfig FontConfig::createMinimalSize() {
    return FontConfigFactory::createMinimalSize();
}

FontConfig FontConfig::createMaxCompatibility() {
    return FontConfigFactory::createMaxCompatibility();
}

FontConfig FontConfig::createBalanced() {
    return FontConfigFactory::createBalanced();
}

FontConfig FontConfig::createDevelopment() {
    return FontConfigFactory::createDevelopment();
}

// ========================================
// IndividualFontConfig 类实现
// ========================================

IndividualFontConfig::IndividualFontConfig(const std::string& font_name)
    : font_name_(font_name) {
}

IndividualFontConfig& IndividualFontConfig::overrideEmbeddingStrategy(FontEmbeddingStrategy strategy) {
    strategy_override_ = strategy;
    return *this;
}

IndividualFontConfig& IndividualFontConfig::overrideSubsetOptions(const FontSubsetOptions& options) {
    subset_override_ = options;
    return *this;
}

IndividualFontConfig& IndividualFontConfig::setAlias(const std::string& alias) {
    alias_ = alias;
    return *this;
}

IndividualFontConfig& IndividualFontConfig::forceEncoding(const std::string& encoding) {
    forced_encoding_ = encoding;
    return *this;
}

} // namespace tinakit::pdf::config
