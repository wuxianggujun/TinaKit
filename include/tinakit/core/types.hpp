/**
 * @file types.hpp
 * @brief TinaKit 核心类型定义
 * @author TinaKit Team
 * @date 2024-12-16
 */

#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include <filesystem>

namespace tinakit {

// 前向声明
class WorkbookImpl;
class WorksheetImpl;
class CellImpl;
class DocumentImpl;
class RangeImpl;

/**
 * @brief 配置类
 */
struct Config {
    /**
     * @brief 默认配置
     * @return 默认配置对象
     */
    static Config default_config();
    
    bool enable_async = true;           ///< 启用异步处理
    std::size_t thread_pool_size = 4;   ///< 线程池大小
    std::size_t max_memory_usage = 1024 * 1024 * 1024;  ///< 最大内存使用量（字节）
    bool enable_formula_calculation = true;  ///< 启用公式计算
    std::string temp_directory = "";    ///< 临时目录路径
};

/**
 * @brief 颜色类
 */
class Color {
public:
    /**
     * @brief 从 RGB 值构造颜色
     * @param r 红色分量 (0-255)
     * @param g 绿色分量 (0-255)
     * @param b 蓝色分量 (0-255)
     * @param a 透明度分量 (0-255)，默认为 255（不透明）
     */
    Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255);
    
    /**
     * @brief 从十六进制字符串构造颜色
     * @param hex 十六进制颜色字符串（如 "#FF0000" 或 "FF0000"）
     */
    explicit Color(const std::string& hex);
    
    /**
     * @brief 获取红色分量
     */
    std::uint8_t red() const noexcept { return r_; }
    
    /**
     * @brief 获取绿色分量
     */
    std::uint8_t green() const noexcept { return g_; }
    
    /**
     * @brief 获取蓝色分量
     */
    std::uint8_t blue() const noexcept { return b_; }
    
    /**
     * @brief 获取透明度分量
     */
    std::uint8_t alpha() const noexcept { return a_; }
    
    /**
     * @brief 转换为十六进制字符串
     * @return 十六进制颜色字符串
     */
    std::string to_hex() const;

public:
    // 预定义颜色
    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Cyan;
    static const Color Magenta;
    static const Color LightGray;
    static const Color DarkGray;
    static const Color LightBlue;
    static const Color LightGreen;
    static const Color LightRed;

private:
    std::uint8_t r_, g_, b_, a_;
};

/**
 * @brief 对齐方式枚举
 */
enum class Alignment {
    Left,           ///< 左对齐
    Center,         ///< 居中对齐
    Right,          ///< 右对齐
    Justify,        ///< 两端对齐
    Top,            ///< 顶部对齐
    Middle,         ///< 垂直居中
    Bottom          ///< 底部对齐
};

/**
 * @brief 边框类型枚举
 */
enum class BorderType {
    None,           ///< 无边框
    All,            ///< 所有边框
    Top,            ///< 顶部边框
    Bottom,         ///< 底部边框
    Left,           ///< 左边框
    Right,          ///< 右边框
    Outline         ///< 外边框
};

/**
 * @brief 边框样式枚举
 */
enum class BorderStyle {
    None,           ///< 无样式
    Thin,           ///< 细线
    Medium,         ///< 中等线
    Thick,          ///< 粗线
    Double,         ///< 双线
    Dotted,         ///< 点线
    Dashed          ///< 虚线
};

/**
 * @brief 位置类
 */
struct Position {
    std::size_t row;     ///< 行号（从 1 开始）
    std::size_t column;  ///< 列号（从 1 开始）
    
    /**
     * @brief 构造函数
     * @param r 行号
     * @param c 列号
     */
    Position(std::size_t r, std::size_t c) : row(r), column(c) {}
    
    /**
     * @brief 从地址字符串创建位置
     * @param address 地址字符串（如 "A1"）
     * @return 位置对象
     */
    static Position from_address(const std::string& address);
    
    /**
     * @brief 转换为地址字符串
     * @return 地址字符串
     */
    std::string to_address() const;
    
    /**
     * @brief 在段落后插入位置
     * @param paragraph_index 段落索引
     * @return 位置对象
     */
    static Position after_paragraph(std::size_t paragraph_index);
};

/**
 * @brief 范围类
 */
struct Range {
    Position start;  ///< 起始位置
    Position end;    ///< 结束位置
    
    /**
     * @brief 构造函数
     * @param start_pos 起始位置
     * @param end_pos 结束位置
     */
    Range(const Position& start_pos, const Position& end_pos) 
        : start(start_pos), end(end_pos) {}
    
    /**
     * @brief 从范围字符串创建范围
     * @param range_str 范围字符串（如 "A1:C10"）
     * @return 范围对象
     */
    static Range from_string(const std::string& range_str);
    
    /**
     * @brief 转换为范围字符串
     * @return 范围字符串
     */
    std::string to_string() const;
    
    /**
     * @brief 检查位置是否在范围内
     * @param pos 位置
     * @return 如果在范围内返回 true
     */
    bool contains(const Position& pos) const;
    
    /**
     * @brief 获取范围大小
     * @return 行数和列数的对
     */
    std::pair<std::size_t, std::size_t> size() const;
};

/**
 * @brief 文档类型枚举
 */
enum class DocumentType {
    Excel,          ///< Excel 文档
    Word,           ///< Word 文档
    PowerPoint,     ///< PowerPoint 文档
    Unknown         ///< 未知类型
};

/**
 * @brief 从文件扩展名获取文档类型
 * @param extension 文件扩展名
 * @return 文档类型
 */
DocumentType get_document_type(const std::string& extension);

/**
 * @brief 检查文件扩展名是否受支持
 * @param extension 文件扩展名
 * @return 如果支持返回 true
 */
bool is_supported_format(const std::string& extension);

/**
 * @brief 列号转换为列名
 * @param column 列号（从 1 开始）
 * @return 列名（如 "A", "B", "AA"）
 */
std::string column_number_to_name(std::size_t column);

/**
 * @brief 列名转换为列号
 * @param column_name 列名
 * @return 列号（从 1 开始）
 */
std::size_t column_name_to_number(const std::string& column_name);

} // namespace tinakit

/**
 * @example types_usage.cpp
 * 类型使用示例：
 * @code
 * #include <tinakit/core/types.hpp>
 * 
 * void types_example() {
 *     using namespace tinakit;
 *     
 *     // 颜色使用
 *     auto red = Color(255, 0, 0);
 *     auto blue = Color("#0000FF");
 *     
 *     // 位置和范围
 *     auto pos = Position::from_address("A1");
 *     auto range = Range::from_string("A1:C10");
 *     
 *     // 列名转换
 *     auto col_name = column_number_to_name(1);  // "A"
 *     auto col_num = column_name_to_number("AA");  // 27
 * }
 * @endcode
 */
