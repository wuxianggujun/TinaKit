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