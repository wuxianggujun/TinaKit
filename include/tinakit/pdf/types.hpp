/**
 * @file types.hpp
 * @brief PDF模块类型定义
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "tinakit/core/types.hpp"
#include "tinakit/core/color.hpp"

namespace tinakit::pdf {

// ========================================
// 使用core模块中的基础类型
// ========================================

using Point = tinakit::core::Point;
using Rect = tinakit::core::Rect;
using Color = tinakit::Color;

// ========================================
// PDF特有的几何类型
// ========================================

/**
 * @brief 尺寸
 */
struct Size {
    double width = 0.0;
    double height = 0.0;

    Size() = default;
    Size(double w_, double h_) : width(w_), height(h_) {}
};

// ========================================
// 字体类型
// ========================================

/**
 * @brief PDF字体定义
 *
 * 继承自core::BaseFont，添加PDF特有的属性
 */
struct Font : public tinakit::core::BaseFont {
    // PDF特有的字体属性可以在这里添加
    // 例如：字体嵌入选项、编码方式等

    /**
     * @brief 默认构造函数
     */
    Font() = default;

    /**
     * @brief 从BaseFont构造
     */
    Font(const tinakit::core::BaseFont& base) : tinakit::core::BaseFont(base) {}

    /**
     * @brief 构造函数
     * @param family_ 字体族名
     * @param size_ 字体大小
     */
    Font(const std::string& family_, double size_)
        : tinakit::core::BaseFont(family_, size_) {}

    /**
     * @brief 构造函数
     * @param family_ 字体族名
     * @param size_ 字体大小
     * @param color_ 字体颜色
     */
    Font(const std::string& family_, double size_, const Color& color_)
        : tinakit::core::BaseFont(family_, size_, color_) {}
};

// ========================================
// 页面类型
// ========================================

/**
 * @brief 页面大小
 */
enum class PageSize {
    A4,         ///< A4 (210 x 297 mm)
    A3,         ///< A3 (297 x 420 mm)
    A5,         ///< A5 (148 x 210 mm)
    Letter,     ///< Letter (8.5 x 11 inch)
    Legal,      ///< Legal (8.5 x 14 inch)
    Tabloid,    ///< Tabloid (11 x 17 inch)
    Custom      ///< 自定义大小
};

/**
 * @brief 页面方向
 */
enum class PageOrientation {
    Portrait,   ///< 纵向
    Landscape   ///< 横向
};

/**
 * @brief 页面边距
 */
struct PageMargins {
    double top = 72.0;      ///< 上边距 (1 inch = 72 points)
    double right = 72.0;    ///< 右边距
    double bottom = 72.0;   ///< 下边距
    double left = 72.0;     ///< 左边距
    
    PageMargins() = default;
    PageMargins(double all) : top(all), right(all), bottom(all), left(all) {}
    PageMargins(double vertical, double horizontal) 
        : top(vertical), right(horizontal), bottom(vertical), left(horizontal) {}
    PageMargins(double top_, double right_, double bottom_, double left_)
        : top(top_), right(right_), bottom(bottom_), left(left_) {}
};

// ========================================
// 文本对齐
// ========================================

/**
 * @brief 文本对齐方式
 */
enum class TextAlignment {
    Left,       ///< 左对齐
    Center,     ///< 居中对齐
    Right,      ///< 右对齐
    Justify     ///< 两端对齐
};

/**
 * @brief 垂直对齐方式
 */
enum class VerticalAlignment {
    Top,        ///< 顶部对齐
    Middle,     ///< 中间对齐
    Bottom      ///< 底部对齐
};

// ========================================
// 文档信息
// ========================================

/**
 * @brief 文档信息
 */
struct DocumentInfo {
    std::string title;          ///< 标题
    std::string author;         ///< 作者
    std::string subject;        ///< 主题
    std::string keywords;       ///< 关键词
    std::string creator;        ///< 创建者
    std::string producer = "TinaKit PDF Library";  ///< 生产者
    std::string creation_date;  ///< 创建日期
    std::string mod_date;       ///< 修改日期
};

// ========================================
// 表格类型
// ========================================

/**
 * @brief 表格单元格
 */
struct TableCell {
    std::string text;
    Font font;
    Color background_color = Color::White;
    TextAlignment alignment = TextAlignment::Left;
    VerticalAlignment vertical_alignment = VerticalAlignment::Middle;
    bool has_border = true;
    Color border_color = Color::Black;
    double border_width = 1.0;
    
    TableCell() = default;
    TableCell(const std::string& text_) : text(text_) {}
    TableCell(const std::string& text_, const Font& font_) : text(text_), font(font_) {}
};

/**
 * @brief 表格行
 */
struct TableRow {
    std::vector<TableCell> cells;
    double height = 0.0;  ///< 行高，0表示自动
    
    TableRow() = default;
    TableRow(const std::vector<std::string>& texts) {
        for (const auto& text : texts) {
            cells.emplace_back(text);
        }
    }
};

/**
 * @brief 表格
 */
struct Table {
    std::vector<TableRow> rows;
    std::vector<double> column_widths;  ///< 列宽，空表示自动
    double total_width = 0.0;           ///< 总宽度，0表示自动
    bool has_header = false;            ///< 是否有表头
    Font header_font;                   ///< 表头字体
    Color header_background = Color::LightGray;  ///< 表头背景色
    
    Table() = default;
    
    /**
     * @brief 添加行
     */
    void add_row(const TableRow& row) {
        rows.push_back(row);
    }
    
    /**
     * @brief 添加行（从字符串列表）
     */
    void add_row(const std::vector<std::string>& texts) {
        rows.emplace_back(texts);
    }
    
    /**
     * @brief 设置列宽
     */
    void set_column_widths(const std::vector<double>& widths) {
        column_widths = widths;
    }
    
    /**
     * @brief 获取行数
     */
    std::size_t row_count() const {
        return rows.size();
    }
    
    /**
     * @brief 获取列数
     */
    std::size_t column_count() const {
        return rows.empty() ? 0 : rows[0].cells.size();
    }
};

// ========================================
// 工具函数
// ========================================

/**
 * @brief 将页面大小转换为点数
 * @param size 页面大小
 * @param orientation 页面方向
 * @return 宽度和高度（点数）
 */
std::pair<double, double> page_size_to_points(PageSize size, PageOrientation orientation = PageOrientation::Portrait);

/**
 * @brief 将毫米转换为点数
 * @param mm 毫米
 * @return 点数
 */
inline double mm_to_points(double mm) {
    return mm * 72.0 / 25.4;  // 1 inch = 25.4 mm = 72 points
}

/**
 * @brief 将英寸转换为点数
 * @param inches 英寸
 * @return 点数
 */
inline double inches_to_points(double inches) {
    return inches * 72.0;  // 1 inch = 72 points
}

/**
 * @brief 将点数转换为毫米
 * @param points 点数
 * @return 毫米
 */
inline double points_to_mm(double points) {
    return points * 25.4 / 72.0;
}

/**
 * @brief 将点数转换为英寸
 * @param points 点数
 * @return 英寸
 */
inline double points_to_inches(double points) {
    return points / 72.0;
}

} // namespace tinakit::pdf
