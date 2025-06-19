/**
 * @file excel/types.hpp
 * @brief Excel-specific type definitions
 * @author TinaKit Team
 * @date 2025-01-09
 */

#pragma once

#include "tinakit/core/color.hpp"
#include <string>
#include <cstdint>
#include <stdexcept>
#include <optional>

namespace tinakit::excel {

/**
 * @brief Position class
 * 
 * @note This class uses 1-based indexing to match Excel's addressing convention.
 *       Row 1 is the first row, Column 1 is column A.
 *       Internal implementations should convert to 0-based indexing when necessary.
 */
struct Position {
    std::size_t row;     ///< Row number (1-based, starting from 1)
    std::size_t column;  ///< Column number (1-based, 1=A, 2=B, etc.)

    /**
     * @brief Constructor
     * @param r Row number (must be >= 1)
     * @param c Column number (must be >= 1)
     * @throws std::invalid_argument if r or c is 0
     */
    Position(std::size_t r, std::size_t c) : row(r), column(c) {
        if (r == 0 || c == 0) {
            throw std::invalid_argument("Position indices must be 1-based (row and column must be >= 1)");
        }
    }

    /**
     * @brief Create position from address string
     * @param address Address string (e.g., "A1")
     * @return Position object
     */
    static Position from_address(const std::string& address);

    /**
     * @brief Convert to address string
     * @return Address string
     */
    std::string to_address() const;

    /**
     * @brief Equality operator
     * @param other Other position
     * @return True if positions are equal
     */
    bool operator==(const Position& other) const {
        return row == other.row && column == other.column;
    }

    /**
     * @brief Inequality operator
     * @param other Other position
     * @return True if positions are not equal
     */
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

/**
 * @brief Range class forward declaration
 */
class Range;

/**
 * @brief Border type enumeration (for Cell API)
 */
enum class BorderType {
    None,           ///< No border
    All,            ///< All borders
    Top,            ///< Top border
    Bottom,         ///< Bottom border
    Left,           ///< Left border
    Right,          ///< Right border
    Outline         ///< Outline border
};

/**
 * @brief Border style enumeration (simplified for Cell API)
 */
enum class BorderStyle {
    None,           ///< No style
    Thin,           ///< Thin line
    Medium,         ///< Medium line
    Thick,          ///< Thick line
    Double,         ///< Double line
    Dotted,         ///< Dotted line
    Dashed          ///< Dashed line
};

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
 * @brief 边框样式（完整版本）
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
    Vertical vertical = Vertical::Center;  // 默认垂直居中，像WPS一样
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
 * @brief Convert column number to column name
 * @param column Column number (1-based)
 * @return Column name (e.g., "A", "B", "AA")
 */
std::string column_number_to_name(std::size_t column);

/**
 * @brief Convert column name to column number
 * @param column_name Column name
 * @return Column number (1-based)
 */
std::size_t column_name_to_number(const std::string& column_name);

} // namespace tinakit::excel