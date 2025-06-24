/**
 * @file font_config.hpp
 * @brief PDF 字体配置接口
 * @author TinaKit Team
 * @date 2025-6-24
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <optional>

namespace tinakit::pdf::config {

/**
 * @brief 字体嵌入策略
 */
enum class FontEmbeddingStrategy {
    /**
     * @brief 不嵌入字体，使用系统字体引用
     * 优点：文件小，兼容性好（对于标准字体）
     * 缺点：依赖目标系统有相同字体
     */
    NONE,
    
    /**
     * @brief 完整嵌入字体
     * 优点：完全自包含，显示一致
     * 缺点：文件大
     */
    FULL_EMBED,
    
    /**
     * @brief 子集化嵌入（推荐）
     * 优点：文件小且自包含
     * 缺点：处理时间稍长
     */
    SUBSET_EMBED,
    
    /**
     * @brief 自动选择（根据字体类型和使用情况）
     * 系统字体 -> NONE
     * 自定义字体 -> SUBSET_EMBED
     */
    AUTO
};

/**
 * @brief 字体子集化选项
 */
struct FontSubsetOptions {
    /**
     * @brief 是否保留字形ID（GID）
     * true: 保持原始字形ID，兼容性更好
     * false: 重新映射字形ID，文件更小
     */
    bool retain_gids = true;
    
    /**
     * @brief 最小字符数阈值
     * 当使用字符数少于此值时才进行子集化
     */
    size_t min_char_threshold = 1000;
    
    /**
     * @brief 最大字符数阈值
     * 当使用字符数超过此值时使用完整嵌入
     */
    size_t max_char_threshold = 10000;
    
    /**
     * @brief 压缩比阈值
     * 只有当子集化后大小小于原始大小的此比例时才使用子集
     */
    double compression_threshold = 0.8;
    
    /**
     * @brief 是否包含基本字符集
     * true: 自动包含ASCII字符和常用符号
     */
    bool include_basic_charset = true;
};

/**
 * @brief 字体配置类
 */
class FontConfig {
public:
    /**
     * @brief 构造函数
     * @param strategy 嵌入策略
     */
    explicit FontConfig(FontEmbeddingStrategy strategy = FontEmbeddingStrategy::AUTO);
    
    /**
     * @brief 设置嵌入策略
     */
    FontConfig& setEmbeddingStrategy(FontEmbeddingStrategy strategy);
    
    /**
     * @brief 获取嵌入策略
     */
    FontEmbeddingStrategy getEmbeddingStrategy() const;
    
    /**
     * @brief 设置子集化选项
     */
    FontConfig& setSubsetOptions(const FontSubsetOptions& options);
    
    /**
     * @brief 获取子集化选项
     */
    const FontSubsetOptions& getSubsetOptions() const;
    
    /**
     * @brief 添加字体回退链
     * @param primary_font 主字体
     * @param fallback_fonts 回退字体列表
     */
    FontConfig& addFontFallback(const std::string& primary_font, 
                               const std::vector<std::string>& fallback_fonts);
    
    /**
     * @brief 设置是否启用字体回退
     */
    FontConfig& enableFontFallback(bool enable = true);
    
    /**
     * @brief 预加载系统字体
     * 在文档创建时预先扫描和缓存系统字体
     */
    FontConfig& preloadSystemFonts(bool enable = true);
    
    /**
     * @brief 设置字体搜索路径
     */
    FontConfig& addFontSearchPath(const std::string& path);
    
    /**
     * @brief 创建预设配置
     */
    static FontConfig createMinimalSize();    // 最小文件大小
    static FontConfig createMaxCompatibility(); // 最大兼容性
    static FontConfig createBalanced();       // 平衡模式（默认）
    static FontConfig createDevelopment();    // 开发模式（快速）

private:
    FontEmbeddingStrategy strategy_;
    FontSubsetOptions subset_options_;
    bool enable_fallback_;
    bool preload_system_fonts_;
    std::vector<std::string> font_search_paths_;
    // 字体回退映射等其他配置...
};

/**
 * @brief 单个字体的配置
 * 允许为特定字体覆盖全局配置
 */
class IndividualFontConfig {
public:
    /**
     * @brief 构造函数
     * @param font_name 字体名称或路径
     */
    explicit IndividualFontConfig(const std::string& font_name);
    
    /**
     * @brief 覆盖嵌入策略
     */
    IndividualFontConfig& overrideEmbeddingStrategy(FontEmbeddingStrategy strategy);
    
    /**
     * @brief 覆盖子集化选项
     */
    IndividualFontConfig& overrideSubsetOptions(const FontSubsetOptions& options);
    
    /**
     * @brief 设置字体别名
     */
    IndividualFontConfig& setAlias(const std::string& alias);
    
    /**
     * @brief 强制使用特定编码
     */
    IndividualFontConfig& forceEncoding(const std::string& encoding);

    /**
     * @brief 获取字体名称
     */
    const std::string& getFontName() const { return font_name_; }

    /**
     * @brief 检查是否有策略覆盖
     */
    bool hasStrategyOverride() const { return strategy_override_.has_value(); }

    /**
     * @brief 获取策略覆盖
     */
    FontEmbeddingStrategy getStrategyOverride() const {
        return strategy_override_.value_or(FontEmbeddingStrategy::AUTO);
    }

private:
    std::string font_name_;
    std::optional<FontEmbeddingStrategy> strategy_override_;
    std::optional<FontSubsetOptions> subset_override_;
    std::string alias_;
    std::string forced_encoding_;
};

/**
 * @brief 字体配置工厂类
 * 提供常用的预设配置
 */
class FontConfigFactory {
public:
    /**
     * @brief 创建最小文件大小配置
     * 优先使用系统字体，对自定义字体进行激进的子集化
     */
    static FontConfig createMinimalSize() {
        FontConfig config(FontEmbeddingStrategy::AUTO);

        FontSubsetOptions subset_opts;
        subset_opts.retain_gids = false;  // 重新映射GID以获得更小文件
        subset_opts.min_char_threshold = 1;  // 即使只有1个字符也子集化
        subset_opts.max_char_threshold = 50000;  // 很高的阈值
        subset_opts.compression_threshold = 0.95;  // 95%压缩比
        subset_opts.include_basic_charset = false;  // 不包含额外字符

        config.setSubsetOptions(subset_opts);
        config.enableFontFallback(false);  // 禁用回退以减少字体数量

        return config;
    }

    /**
     * @brief 创建最大兼容性配置
     * 优先使用系统字体，确保在所有设备上都能正确显示
     */
    static FontConfig createMaxCompatibility() {
        FontConfig config(FontEmbeddingStrategy::NONE);
        config.enableFontFallback(true);
        config.addFontFallback("Arial", {"Helvetica", "sans-serif"});
        config.addFontFallback("Times", {"Times-Roman", "serif"});
        return config;
    }

    /**
     * @brief 创建平衡配置（推荐）
     * 在文件大小和兼容性之间取得平衡
     */
    static FontConfig createBalanced() {
        FontConfig config(FontEmbeddingStrategy::AUTO);

        FontSubsetOptions subset_opts;
        subset_opts.retain_gids = true;
        subset_opts.min_char_threshold = 100;
        subset_opts.max_char_threshold = 10000;
        subset_opts.compression_threshold = 0.8;
        subset_opts.include_basic_charset = true;

        config.setSubsetOptions(subset_opts);
        config.enableFontFallback(true);

        return config;
    }

    /**
     * @brief 创建开发模式配置
     * 快速生成，不进行优化
     */
    static FontConfig createDevelopment() {
        FontConfig config(FontEmbeddingStrategy::FULL_EMBED);
        config.enableFontFallback(false);
        config.preloadSystemFonts(false);
        return config;
    }
};

} // namespace tinakit::pdf::config
