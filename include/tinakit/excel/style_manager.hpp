/**
 * @file style_manager.hpp
 * @brief Excel 样式管理器
 * @author TinaKit Team
 * @date 2025-01-08
 */

#pragma once

#include "tinakit/core/color.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <optional>

namespace tinakit::excel {

/**
 * @brief 字体样式
 */
struct Font {
    std::string name = "Calibri";      ///< 字体名称
    double size = 11.0;                ///< 字体大小
    bool bold = false;                 ///< 是否粗体
    bool italic = false;               ///< 是否斜体
    bool underline = false;            ///< 是否下划线
    bool strike = false;               ///< 是否删除线
    std::optional<Color> color;        ///< 字体颜色
    
    bool operator==(const Font& other) const;
};

/**
 * @brief 填充样式
 */
struct Fill {
    enum class PatternType {
        None = 0,
        Solid = 1,
        MediumGray = 2,
        DarkGray = 3,
        LightGray = 4,
        DarkHorizontal = 5,
        DarkVertical = 6,
        DarkDown = 7,
        DarkUp = 8,
        DarkGrid = 9,
        DarkTrellis = 10,
        LightHorizontal = 11,
        LightVertical = 12,
        LightDown = 13,
        LightUp = 14,
        LightGrid = 15,
        LightTrellis = 16,
        Gray125 = 17,
        Gray0625 = 18
    };
    
    PatternType pattern_type = PatternType::None;
    std::optional<Color> fg_color;     ///< 前景色
    std::optional<Color> bg_color;     ///< 背景色
    
    bool operator==(const Fill& other) const;
};

/**
 * @brief 边框样式
 */
struct Border {
    enum class Style {
        None = 0,
        Thin = 1,
        Medium = 2,
        Dashed = 3,
        Dotted = 4,
        Thick = 5,
        Double = 6,
        Hair = 7,
        MediumDashed = 8,
        DashDot = 9,
        MediumDashDot = 10,
        DashDotDot = 11,
        MediumDashDotDot = 12,
        SlantDashDot = 13
    };
    
    struct BorderLine {
        Style style = Style::None;
        std::optional<Color> color;
        
        bool operator==(const BorderLine& other) const;
    };
    
    BorderLine left;
    BorderLine right;
    BorderLine top;
    BorderLine bottom;
    BorderLine diagonal;
    bool diagonal_up = false;
    bool diagonal_down = false;
    
    bool operator==(const Border& other) const;
};

/**
 * @brief 对齐方式
 */
struct Alignment {
    enum class Horizontal {
        General = 0,
        Left = 1,
        Center = 2,
        Right = 3,
        Fill = 4,
        Justify = 5,
        CenterContinuous = 6,
        Distributed = 7
    };
    
    enum class Vertical {
        Top = 0,
        Center = 1,
        Bottom = 2,
        Justify = 3,
        Distributed = 4
    };
    
    Horizontal horizontal = Horizontal::General;
    Vertical vertical = Vertical::Bottom;
    int text_rotation = 0;             ///< 文本旋转角度（-90 到 90 或 255）
    bool wrap_text = false;            ///< 是否自动换行
    bool shrink_to_fit = false;        ///< 是否缩小字体以适应
    int indent = 0;                    ///< 缩进级别
    
    bool operator==(const Alignment& other) const;
};

/**
 * @brief 数字格式
 */
struct NumberFormat {
    std::uint32_t id = 0;              ///< 格式 ID
    std::string format_code;           ///< 格式代码
    
    bool operator==(const NumberFormat& other) const;
};

/**
 * @brief 单元格样式（XF）
 */
struct CellStyle {
    std::optional<std::uint32_t> font_id;
    std::optional<std::uint32_t> fill_id;
    std::optional<std::uint32_t> border_id;
    std::optional<std::uint32_t> number_format_id;
    std::optional<Alignment> alignment;
    
    bool apply_font = false;
    bool apply_fill = false;
    bool apply_border = false;
    bool apply_number_format = false;
    bool apply_alignment = false;
    
    bool operator==(const CellStyle& other) const;
};

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

private:
    std::vector<Font> fonts_;
    std::vector<Fill> fills_;
    std::vector<Border> borders_;
    std::vector<NumberFormat> number_formats_;
    std::vector<CellStyle> cell_styles_;
    
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