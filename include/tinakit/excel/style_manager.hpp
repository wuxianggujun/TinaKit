/**
 * @file style_manager.hpp
 * @brief Excel 样式管理器
 * @author TinaKit Team
 * @date 2025-01-08
 */

#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>

namespace tinakit::excel {

/**
 * @class StyleManager
 * @brief Excel 样式管理器
 * 
 * 管理 Excel 文件中的所有样式信息，包括字体、填充、边框、数字格式等。
 */
class StyleManager {
public:
    /**
     * @brief 构造函数
     */
    StyleManager();
    
    /**
     * @brief 析构函数
     */
    ~StyleManager() = default;
    
    // 字体管理
    std::uint32_t add_font(const Font& font);
    const Font& get_font(std::uint32_t id) const;
    std::size_t font_count() const { return fonts_.size(); }
    
    // 填充管理
    std::uint32_t add_fill(const Fill& fill);
    const Fill& get_fill(std::uint32_t id) const;
    std::size_t fill_count() const { return fills_.size(); }
    
    // 边框管理
    std::uint32_t add_border(const Border& border);
    const Border& get_border(std::uint32_t id) const;
    std::size_t border_count() const { return borders_.size(); }
    
    // 数字格式管理
    std::uint32_t add_number_format(const NumberFormat& format);
    const NumberFormat& get_number_format(std::uint32_t id) const;
    std::size_t number_format_count() const { return number_formats_.size(); }
    
    // 单元格样式管理
    std::uint32_t add_cell_style(const CellStyle& style);
    const CellStyle& get_cell_style(std::uint32_t id) const;
    std::size_t cell_style_count() const { return cell_styles_.size(); }
    
    /**
     * @brief 获取默认样式 ID
     * @return 默认样式 ID
     */
    std::uint32_t default_style_id() const { return 0; }
    
    /**
     * @brief 清空所有样式
     */
    void clear();
    
    /**
     * @brief 生成样式 XML
     * @return 样式 XML 内容
     */
    std::string generate_xml() const;
    
    /**
     * @brief 从 XML 数据加载样式
     * @param xml_data XML 数据
     */
    void load_from_xml(const std::string& xml_data);
    
    /**
     * @brief 初始化默认样式
     */
    void initialize_defaults();

    /**
     * @brief 添加条件格式差异样式
     * @param font 字体样式（可选）
     * @param fill 填充样式（可选）
     * @return dxf ID（用于条件格式引用）
     */
    std::uint32_t add_dxf(const Font* font, const Fill* fill);

    /**
     * @brief 获取dxf数量
     * @return dxf数量
     */
    std::size_t get_dxf_count() const;

private:
    std::vector<Font> fonts_;
    std::vector<Fill> fills_;
    std::vector<Border> borders_;
    std::vector<NumberFormat> number_formats_;
    std::vector<CellStyle> cell_styles_;

    // 条件格式差异样式存储
    struct Dxf {
        std::optional<Font> font;
        std::optional<Fill> fill;
        std::optional<Border> border;
    };
    std::vector<Dxf> dxfs_;
    
    // 用于快速查找重复样式
    std::unordered_map<std::size_t, std::uint32_t> font_cache_;
    std::unordered_map<std::size_t, std::uint32_t> fill_cache_;
    std::unordered_map<std::size_t, std::uint32_t> border_cache_;
    std::unordered_map<std::string, std::uint32_t> number_format_cache_;
    
    // 辅助函数
    std::size_t hash_font(const Font& font) const;
    std::size_t hash_fill(const Fill& fill) const;
    std::size_t hash_border(const Border& border) const;
};

} // namespace tinakit::excel