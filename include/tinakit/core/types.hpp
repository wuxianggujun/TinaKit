/**
 * @file types.hpp
 * @brief TinaKit core type definitions
 * @author TinaKit Team
 * @date 2025-6-16
 * 
 * @section error_handling Error Handling Strategy
 * 
 * TinaKit follows these error handling principles:
 * 
 * 1. **Exceptions for Errors**: Operations that can fail throw exceptions
 *    - File I/O errors throw IOException
 *    - Invalid data throws ParseException
 *    - Type conversions throw TypeConversionException
 * 
 * 2. **Optional for Missing Values**: Operations that may not have a result return std::optional
 *    - Cell::try_as<T>() returns std::optional<T>
 *    - Finding elements that may not exist
 * 
 * 3. **Noexcept for Query Operations**: Simple getters are noexcept
 *    - Getting size, count, or status information
 *    - Accessing already-validated data
 * 
 * 4. **RAII for Resource Management**: All resources are managed automatically
 *    - No manual cleanup required
 *    - Exception-safe by design
 */

#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include <filesystem>
#include <iostream>
#include <cmath>
#include "color.hpp"

namespace tinakit {

/**
 * @brief Configuration class
 */
struct Config {
    /**
     * @brief Default configuration
     * @return Default configuration object
     */
    static Config default_config() {
        return Config{};
    }

    bool enable_async = true;           ///< Enable async processing
    std::size_t thread_pool_size = 4;   ///< Thread pool size
    std::size_t max_memory_usage = 1024 * 1024 * 1024;  ///< Maximum memory usage (bytes)
    bool enable_formula_calculation = true;  ///< Enable formula calculation
    std::string temp_directory = "";    ///< Temporary directory path
};

} // namespace tinakit

// 核心数据结构命名空间
namespace tinakit::core {

// 前向声明
struct Coordinate;
struct Point;
struct Rect;

/**
 * @struct Coordinate
 * @brief 表示 Excel 中的单元格坐标（行列位置）
 *
 * 重要说明：
 * - 这是1-based坐标系统，行和列都从1开始
 * - 这是整个项目中表示单元格位置的唯一底层数据结构
 * - 任何需要坐标信息的地方都必须使用此结构
 *
 * 这是一个轻量级的 POD 结构，用于表示单元格的位置。
 * 使用 1-based 索引以匹配 Excel 的地址约定。
 */
struct Coordinate {
    std::size_t row = 1;     ///< 行号（从1开始）
    std::size_t column = 1;  ///< 列号（从1开始）

    /**
     * @brief 默认构造函数
     */
    Coordinate() = default;

    /**
     * @brief 构造函数
     * @param r 行号
     * @param c 列号
     */
    Coordinate(std::size_t r, std::size_t c) : row(r), column(c) {}

    /**
     * @brief 检查坐标是否有效（行列都大于0）
     */
    bool is_valid() const noexcept {
        return row > 0 && column > 0;
    }

    /**
     * @brief 从 Excel 地址字符串创建坐标
     * @param address Excel 地址（如 "A1", "B5"）
     * @return Coordinate 对象
     */
    static Coordinate from_address(const std::string& address);

    /**
     * @brief 转换为 Excel 地址字符串
     * @return Excel 地址字符串
     */
    std::string to_address() const;

    /**
     * @brief 相等比较
     */
    bool operator==(const Coordinate& other) const noexcept {
        return row == other.row && column == other.column;
    }

    /**
     * @brief 不等比较
     */
    bool operator!=(const Coordinate& other) const noexcept {
        return !(*this == other);
    }

    /**
     * @brief 小于比较（用于排序）
     */
    bool operator<(const Coordinate& other) const noexcept {
        if (row != other.row) return row < other.row;
        return column < other.column;
    }
};

/**
 * @brief 输出操作符，用于调试和测试
 */
inline std::ostream& operator<<(std::ostream& os, const Coordinate& coord) {
    os << "(" << coord.row << "," << coord.column << ")";
    return os;
}

/**
 * @struct Point
 * @brief 表示2D坐标点
 *
 * 用于PDF等模块中的几何计算和定位。
 * 坐标系统：左下角为原点(0,0)，向右为X正方向，向上为Y正方向。
 */
struct Point {
    double x = 0.0;  ///< X坐标
    double y = 0.0;  ///< Y坐标

    /**
     * @brief 默认构造函数
     */
    Point() = default;

    /**
     * @brief 构造函数
     * @param x_val X坐标值
     * @param y_val Y坐标值
     */
    Point(double x_val, double y_val) : x(x_val), y(y_val) {}

    /**
     * @brief 相等比较
     */
    bool operator==(const Point& other) const noexcept {
        return std::abs(x - other.x) < 1e-9 && std::abs(y - other.y) < 1e-9;
    }

    /**
     * @brief 不等比较
     */
    bool operator!=(const Point& other) const noexcept {
        return !(*this == other);
    }

    /**
     * @brief 点加法
     */
    Point operator+(const Point& other) const noexcept {
        return Point(x + other.x, y + other.y);
    }

    /**
     * @brief 点减法
     */
    Point operator-(const Point& other) const noexcept {
        return Point(x - other.x, y - other.y);
    }
};

/**
 * @struct Rect
 * @brief 表示矩形区域
 *
 * 用于PDF等模块中的区域定义和布局计算。
 * 坐标系统：左下角为原点，x和y表示矩形左下角坐标。
 */
struct Rect {
    double x = 0.0;      ///< 左下角X坐标
    double y = 0.0;      ///< 左下角Y坐标
    double width = 0.0;  ///< 宽度
    double height = 0.0; ///< 高度

    /**
     * @brief 默认构造函数
     */
    Rect() = default;

    /**
     * @brief 构造函数
     * @param x_val 左下角X坐标
     * @param y_val 左下角Y坐标
     * @param w 宽度
     * @param h 高度
     */
    Rect(double x_val, double y_val, double w, double h)
        : x(x_val), y(y_val), width(w), height(h) {}

    /**
     * @brief 从两个点构造矩形
     * @param top_left 左上角点
     * @param bottom_right 右下角点
     */
    static Rect from_points(const Point& top_left, const Point& bottom_right) {
        return Rect(top_left.x, bottom_right.y,
                   bottom_right.x - top_left.x,
                   top_left.y - bottom_right.y);
    }

    /**
     * @brief 获取左下角点
     */
    Point bottom_left() const noexcept {
        return Point(x, y);
    }

    /**
     * @brief 获取右上角点
     */
    Point top_right() const noexcept {
        return Point(x + width, y + height);
    }

    /**
     * @brief 获取中心点
     */
    Point center() const noexcept {
        return Point(x + width / 2.0, y + height / 2.0);
    }

    /**
     * @brief 检查点是否在矩形内
     */
    bool contains(const Point& point) const noexcept {
        return point.x >= x && point.x <= x + width &&
               point.y >= y && point.y <= y + height;
    }

    /**
     * @brief 检查矩形是否有效（宽高都大于0）
     */
    bool is_valid() const noexcept {
        return width > 0.0 && height > 0.0;
    }

    /**
     * @brief 相等比较
     */
    bool operator==(const Rect& other) const noexcept {
        return std::abs(x - other.x) < 1e-9 &&
               std::abs(y - other.y) < 1e-9 &&
               std::abs(width - other.width) < 1e-9 &&
               std::abs(height - other.height) < 1e-9;
    }

    /**
     * @brief 不等比较
     */
    bool operator!=(const Rect& other) const noexcept {
        return !(*this == other);
    }
};

/**
 * @struct range_address
 * @brief 表示 Excel 中的范围地址（替代原有的 Range 和 WorksheetRange）
 *
 * 这是一个轻量级的 POD 结构，用于表示矩形范围的地址信息。
 * 它完全替代了原有的 worksheet_range，提供更清晰的职责分离。
 */
struct range_address {
    Coordinate start;  ///< 起始位置
    Coordinate end;    ///< 结束位置

    /**
     * @brief 默认构造函数（创建 A1:A1 范围）
     */
    range_address() = default;

    /**
     * @brief 构造函数
     * @param start_pos 起始位置
     * @param end_pos 结束位置
     */
    range_address(const Coordinate& start_pos, const Coordinate& end_pos)
        : start(start_pos), end(end_pos) {}

    /**
     * @brief 从字符串创建范围地址
     * @param a1_notation A1 记法字符串（如 "A1:C5", "A:C", "5:5"）
     * @return range_address 对象
     */
    static range_address from_string(const std::string& a1_notation);

    /**
     * @brief 从两个位置创建范围地址
     * @param start_pos 起始位置
     * @param end_pos 结束位置
     * @return range_address 对象
     */
    static range_address from_positions(const Coordinate& start_pos, const Coordinate& end_pos);

    /**
     * @brief 从单个单元格创建范围地址
     * @param pos 单元格位置
     * @return range_address 对象
     */
    static range_address from_single_cell(const Coordinate& pos);

    /**
     * @brief 转换为 A1 记法字符串
     * @return A1 记法字符串
     */
    std::string to_string() const;

    /**
     * @brief 检查位置是否在范围内
     * @param pos 要检查的位置
     * @return 如果位置在范围内返回 true
     */
    bool contains(const Coordinate& pos) const;

    /**
     * @brief 获取范围大小
     * @return std::pair<行数, 列数>
     */
    std::pair<std::size_t, std::size_t> size() const;

    /**
     * @brief 检查两个范围是否重叠
     * @param other 另一个范围
     * @return 如果重叠返回 true
     */
    bool overlaps(const range_address& other) const;

    /**
     * @brief 相等比较
     */
    bool operator==(const range_address& other) const noexcept {
        return start == other.start && end == other.end;
    }

    /**
     * @brief 不等比较
     */
    bool operator!=(const range_address& other) const noexcept {
        return !(*this == other);
    }
};

// ========================================
// 坐标转换工具函数
// ========================================

/**
 * @brief 将列号转换为列名
 * @param column 列号（1-based，1=A, 2=B, 等）
 * @return 列名字符串（如 "A", "B", "AA"）
 * @throws std::invalid_argument 如果列号为0
 */
std::string column_number_to_name(std::size_t column);

/**
 * @brief 将列名转换为列号
 * @param column_name 列名字符串（如 "A", "B", "AA"）
 * @return 列号（1-based）
 * @throws std::invalid_argument 如果列名无效
 */
std::size_t column_name_to_number(const std::string& column_name);

// ========================================
// 基础字体类型
// ========================================

/**
 * @brief 基础字体类型
 *
 * 定义所有模块共享的基础字体属性。
 * 各模块可以继承或扩展此类型以添加特有功能。
 */
struct BaseFont {
    std::string family = "Arial";       ///< 字体族名
    double size = 12.0;                 ///< 字体大小（点数）
    bool bold = false;                  ///< 是否粗体
    bool italic = false;                ///< 是否斜体
    bool underline = false;             ///< 是否下划线
    Color color = Color::Black;         ///< 字体颜色

    /**
     * @brief 默认构造函数
     */
    BaseFont() = default;

    /**
     * @brief 构造函数
     * @param family_ 字体族名
     * @param size_ 字体大小
     */
    BaseFont(const std::string& family_, double size_)
        : family(family_), size(size_) {}

    /**
     * @brief 构造函数
     * @param family_ 字体族名
     * @param size_ 字体大小
     * @param color_ 字体颜色
     */
    BaseFont(const std::string& family_, double size_, const Color& color_)
        : family(family_), size(size_), color(color_) {}

    /**
     * @brief 相等比较
     */
    bool operator==(const BaseFont& other) const noexcept {
        return family == other.family &&
               std::abs(size - other.size) < 1e-9 &&
               bold == other.bold &&
               italic == other.italic &&
               underline == other.underline &&
               color == other.color;
    }

    /**
     * @brief 不等比较
     */
    bool operator!=(const BaseFont& other) const noexcept {
        return !(*this == other);
    }
};

} // namespace tinakit::core

namespace tinakit {

/**
 * @brief Document type enumeration
 */
enum class DocumentType {
    Excel,          ///< Excel document
    Word,           ///< Word document
    PowerPoint,     ///< PowerPoint document
    Unknown         ///< Unknown type
};

/**
 * @brief Get document type from file extension
 * @param extension File extension
 * @return Document type
 */
DocumentType get_document_type(const std::string& extension);

/**
 * @brief Check if file extension is supported
 * @param extension File extension
 * @return True if supported
 */
bool is_supported_format(const std::string& extension);

} // namespace tinakit

/**
 * @example types_usage.cpp
 * Type usage example:
 * @code
 * #include <tinakit/core/types.hpp>
 *
 * void types_example() {
 *     using namespace tinakit;
 *
 *     // Color usage
 *     auto red = Color(255, 0, 0);
 *     auto blue = Color("#0000FF");
 *
 *     // Configuration
 *     auto config = Config::default_config();
 *     config.enable_async = false;
 *     
 *     // Document type
 *     auto doc_type = get_document_type(".xlsx");  // DocumentType::Excel
 * }
 * @endcode
 */