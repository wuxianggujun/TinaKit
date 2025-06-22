/**
 * @file style.hpp
 * @brief PDF样式系统
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include <string>
#include "tinakit/pdf/types.hpp"

namespace tinakit::pdf {

// 使用types.hpp中定义的类型
// Color, TextAlignment, VerticalAlignment 等已在types.hpp中定义

/**
 * @brief PDF字体样式
 */
struct FontStyle {
    std::string family = "Arial";       ///< 字体族
    double size = 12.0;                 ///< 字体大小（点）
    bool bold = false;                  ///< 是否粗体
    bool italic = false;                ///< 是否斜体
    bool underline = false;             ///< 是否下划线
    Color color = Color::Black;         ///< 字体颜色
    
    /**
     * @brief 默认构造函数
     */
    FontStyle() = default;
    
    /**
     * @brief 构造函数
     */
    FontStyle(const std::string& font_family, double font_size, const Color& font_color = Color::Black)
        : family(font_family), size(font_size), color(font_color) {}
    
    /**
     * @brief 设置为粗体
     */
    FontStyle& set_bold(bool is_bold = true) {
        bold = is_bold;
        return *this;
    }
    
    /**
     * @brief 设置为斜体
     */
    FontStyle& set_italic(bool is_italic = true) {
        italic = is_italic;
        return *this;
    }
    
    /**
     * @brief 设置下划线
     */
    FontStyle& set_underline(bool has_underline = true) {
        underline = has_underline;
        return *this;
    }
    
    /**
     * @brief 设置颜色
     */
    FontStyle& set_color(const Color& font_color) {
        color = font_color;
        return *this;
    }
};

/**
 * @brief PDF边框样式
 */
enum class BorderStyle {
    None,       ///< 无边框
    Solid,      ///< 实线
    Dashed,     ///< 虚线
    Dotted      ///< 点线
};

/**
 * @brief PDF边框定义
 */
struct Border {
    BorderStyle style = BorderStyle::None;  ///< 边框样式
    double width = 1.0;                     ///< 边框宽度
    Color color = Color::Black;             ///< 边框颜色
    
    /**
     * @brief 默认构造函数（无边框）
     */
    Border() = default;
    
    /**
     * @brief 构造函数
     */
    Border(BorderStyle border_style, double border_width = 1.0, const Color& border_color = Color::Black)
        : style(border_style), width(border_width), color(border_color) {}
    
    /**
     * @brief 检查是否有边框
     */
    bool has_border() const {
        return style != BorderStyle::None && width > 0.0;
    }
};

/**
 * @brief PDF单元格样式
 */
struct CellStyle {
    FontStyle font;                         ///< 字体样式
    Color background_color = Color::White;  ///< 背景颜色
    Border border;                          ///< 边框
    TextAlignment alignment = TextAlignment::Left;  ///< 文本对齐
    double padding = 2.0;                   ///< 内边距（点）
    
    /**
     * @brief 默认构造函数
     */
    CellStyle() = default;
    
    /**
     * @brief 构造函数
     */
    CellStyle(const FontStyle& cell_font, const Color& bg_color = Color::White)
        : font(cell_font), background_color(bg_color) {}
    
    /**
     * @brief 设置字体
     */
    CellStyle& set_font(const FontStyle& cell_font) {
        font = cell_font;
        return *this;
    }
    
    /**
     * @brief 设置背景色
     */
    CellStyle& set_background(const Color& bg_color) {
        background_color = bg_color;
        return *this;
    }
    
    /**
     * @brief 设置边框
     */
    CellStyle& set_border(const Border& cell_border) {
        border = cell_border;
        return *this;
    }
    
    /**
     * @brief 设置对齐方式
     */
    CellStyle& set_alignment(TextAlignment text_alignment) {
        alignment = text_alignment;
        return *this;
    }
    
    /**
     * @brief 设置内边距
     */
    CellStyle& set_padding(double cell_padding) {
        padding = cell_padding;
        return *this;
    }
};

/**
 * @brief PDF表格样式
 */
struct TableStyle {
    CellStyle header_style;                 ///< 表头样式
    CellStyle data_style;                   ///< 数据样式
    CellStyle alternate_style;              ///< 交替行样式
    bool use_alternate_rows = false;        ///< 是否使用交替行样式
    double row_height = 20.0;               ///< 行高
    double column_spacing = 0.0;            ///< 列间距
    double row_spacing = 0.0;               ///< 行间距
    
    /**
     * @brief 默认构造函数
     */
    TableStyle() {
        // 设置默认的表头样式
        header_style.font.set_bold(true).set_color(Color::White);
        header_style.set_background(Color::Blue);
        header_style.set_alignment(TextAlignment::Center);
        
        // 设置默认的数据样式
        data_style.font.set_color(Color::Black);
        data_style.set_background(Color::White);
        
        // 设置默认的交替行样式
        alternate_style = data_style;
        alternate_style.set_background(Color::LightGray);
    }
    
    /**
     * @brief 设置表头样式
     */
    TableStyle& set_header_style(const CellStyle& style) {
        header_style = style;
        return *this;
    }
    
    /**
     * @brief 设置数据样式
     */
    TableStyle& set_data_style(const CellStyle& style) {
        data_style = style;
        return *this;
    }
    
    /**
     * @brief 启用交替行样式
     */
    TableStyle& enable_alternate_rows(const CellStyle& style) {
        alternate_style = style;
        use_alternate_rows = true;
        return *this;
    }
    
    /**
     * @brief 设置行高
     */
    TableStyle& set_row_height(double height) {
        row_height = height;
        return *this;
    }
};

/**
 * @brief 预定义的PDF样式模板
 */
namespace StyleTemplates {

/**
 * @brief 标题字体样式
 */
inline FontStyle title_font() {
    return FontStyle("Arial", 18, Color::Black).set_bold(true);
}

/**
 * @brief 副标题字体样式
 */
inline FontStyle subtitle_font() {
    return FontStyle("Arial", 14, Color::Black).set_bold(true);
}

/**
 * @brief 正文字体样式
 */
inline FontStyle body_font() {
    return FontStyle("Arial", 12, Color::Black);
}

/**
 * @brief 小字体样式
 */
inline FontStyle small_font() {
    return FontStyle("Arial", 10, Color::Black);
}

/**
 * @brief 表头单元格样式
 */
inline CellStyle header_cell() {
    CellStyle style;
    style.font = FontStyle("Arial", 12, Color::White).set_bold(true);
    style.background_color = Color::Blue;
    style.alignment = TextAlignment::Center;
    style.border = Border(BorderStyle::Solid, 1.0, Color::Black);
    return style;
}

/**
 * @brief 数据单元格样式
 */
inline CellStyle data_cell() {
    CellStyle style;
    style.font = FontStyle("Arial", 10, Color::Black);
    style.background_color = Color::White;
    style.alignment = TextAlignment::Left;
    style.border = Border(BorderStyle::Solid, 0.5, Color::Black);
    return style;
}

/**
 * @brief 专业表格样式
 */
inline TableStyle professional_table() {
    TableStyle style;
    style.set_header_style(header_cell());
    style.set_data_style(data_cell());
    
    // 交替行样式
    auto alt_style = data_cell();
    alt_style.set_background(Color::White);
    style.enable_alternate_rows(alt_style);
    
    style.set_row_height(25.0);
    return style;
}

/**
 * @brief 简洁表格样式
 */
inline TableStyle simple_table() {
    TableStyle style;
    
    // 简洁表头
    auto header = CellStyle();
    header.font.set_bold(true);
    header.set_alignment(TextAlignment::Center);
    style.set_header_style(header);
    
    // 简洁数据
    auto data = CellStyle();
    data.font = body_font();
    style.set_data_style(data);
    
    return style;
}

} // namespace StyleTemplates

} // namespace tinakit::pdf
