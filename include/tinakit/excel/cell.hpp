/**
 * @file cell.hpp
 * @brief Excel 单元格类定义
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include "tinakit/core/color.hpp"
#include "types.hpp"
#include <string>
#include <memory>
#include <optional>
#include <variant>
#include <type_traits>
#include <cstdint>

namespace tinakit::internal {
class workbook_impl;
} // namespace tinakit::internal

namespace tinakit::excel {

class Worksheet;
class Style;

/**
 * @class Cell
 * @brief Excel 单元格轻量级句柄类
 *
 * 这是一个轻量级的句柄类，本身不持有任何重型数据。
 * 所有实际的数据和操作都委托给 internal::workbook_impl。
 *
 * 核心设计原则：
 * 1. 轻量级：只包含 workbook_impl 指针、工作表名称和位置信息
 * 2. 可复制：复制成本极低，安全共享同一个单元格
 * 3. 委托模式：所有操作都委托给 workbook_impl
 * 4. 惰性求值：数据按需加载和保存
 *
 * @note 使用句柄-实现分离模式，提供稳定的 ABI 和优秀的性能
 */
class Cell {
public:
    /**
     * @brief 单元格值类型
     *
     * std::monostate 表示空单元格（未设置任何值）
     * std::string 表示文本值
     * double 表示浮点数值
     * int 表示整数值
     * bool 表示布尔值
     */
    using CellValue = std::variant<std::monostate, std::string, double, int, bool>;

    /**
     * @brief 轻量级句柄ID类型
     */
    using HandleId = std::uint64_t;

public:
    /**
     * @brief 构造函数（由 worksheet 内部创建）
     * @param workbook_impl 工作簿实现的原始指针（性能优化）
     * @param sheet_id 工作表ID（避免字符串复制）
     * @param row 行号（1-based）
     * @param column 列号（1-based）
     */
    Cell(internal::workbook_impl* workbook_impl,
         std::uint32_t sheet_id,
         std::size_t row,
         std::size_t column) noexcept;

    /**
     * @brief 拷贝构造函数（轻量级，共享同一个实现）
     */
    Cell(const Cell& other) = default;

    /**
     * @brief 移动构造函数
     */
    Cell(Cell&& other) noexcept = default;

    /**
     * @brief 拷贝赋值运算符
     */
    Cell& operator=(const Cell& other) = default;

    /**
     * @brief 移动赋值运算符
     */
    Cell& operator=(Cell&& other) noexcept = default;

    /**
     * @brief 析构函数
     */
    ~Cell() = default;

public:
    /**
     * @brief 设置单元格值
     * @tparam T 值类型
     * @param value 要设置的值
     * @return 自身引用，支持链式调用
     */
    template<typename T>
    Cell& value(const T& value);
    
    /**
     * @brief 获取单元格值
     * @tparam T 目标类型
     * @return 转换后的值
     * @throws TypeConversionException 类型转换失败
     */
    template<typename T>
    T as() const;
    
    /**
     * @brief 尝试获取单元格值（不抛出异常）
     * @tparam T 目标类型
     * @return 转换后的值，失败时返回 nullopt
     */
    template<typename T>
    std::optional<T> try_as() const noexcept;

public:
    /**
     * @brief 设置公式
     * @param formula 公式字符串
     * @return 自身引用，支持链式调用
     */
    Cell& formula(const std::string& formula);
    
    /**
     * @brief 获取公式
     * @return 公式字符串，如果没有公式返回 nullopt
     */
    std::optional<std::string> formula() const;

public:
    /**
     * @brief 设置字体
     * @param font_name 字体名称
     * @param size 字体大小
     * @return 自身引用，支持链式调用
     */
    Cell& font(const std::string& font_name, double size = 11.0);
    
    /**
     * @brief 设置粗体
     * @param bold 是否粗体
     * @return 自身引用，支持链式调用
     */
    Cell& bold(bool bold = true);
    
    /**
     * @brief 设置斜体
     * @param italic 是否斜体
     * @return 自身引用，支持链式调用
     */
    Cell& italic(bool italic = true);
    
    /**
     * @brief 设置字体颜色
     * @param color 颜色
     * @return 自身引用，支持链式调用
     */
    Cell& color(const Color& color);
    
    /**
     * @brief 设置背景颜色
     * @param color 背景颜色
     * @return 自身引用，支持链式调用
     */
    Cell& background_color(const Color& color);
    
    /**
     * @brief 设置对齐方式
     * @param alignment 对齐方式
     * @return 自身引用，支持链式调用
     */
    Cell& align(const Alignment& alignment);
    
    /**
     * @brief 设置边框
     * @param border_type 边框类型
     * @param style 边框样式
     * @return 自身引用，支持链式调用
     */
    Cell& border(BorderType border_type, BorderStyle style);

    /**
     * @brief 设置边框（带颜色）
     * @param border_type 边框类型
     * @param style 边框样式
     * @param color 边框颜色
     * @return 自身引用，支持链式调用
     */
    Cell& border(BorderType border_type, BorderStyle style, const Color& color);

    /**
     * @brief 设置数字格式
     * @param format_code 格式代码（如 "#,##0.00"）
     * @return 自身引用，支持链式调用
     */
    Cell& number_format(const std::string& format_code);

    /**
     * @brief 设置文本换行
     * @param wrap 是否换行
     * @return 自身引用，支持链式调用
     */
    Cell& wrap_text(bool wrap = true);

    /**
     * @brief 设置缩进
     * @param indent_level 缩进级别（0-15）
     * @return 自身引用，支持链式调用
     */
    Cell& indent(int indent_level);

    /**
     * @brief 应用样式模板
     * @param style_template 样式模板
     * @return 自身引用，支持链式调用
     */
    Cell& style(const Style& style_template);

public:
    /**
     * @brief 获取单元格地址
     * @return 地址字符串（如 "A1"）
     */
    std::string address() const;
    
    /**
     * @brief 获取行号
     * @return 行号（1-based）
     */
    std::size_t row() const noexcept;
    
    /**
     * @brief 获取列号
     * @return 列号（1-based）
     */
    std::size_t column() const noexcept;
    
    /**
     * @brief 检查是否为空
     * @return 如果单元格为空返回 true
     */
    bool empty() const noexcept;
    
    /**
     * @brief 获取单元格的原始值（variant）
     * @return 单元格值的 variant
     */
    const CellValue& raw_value() const;
    
    /**
     * @brief 获取字符串表示
     * @return 单元格值的字符串表示
     */
    std::string to_string() const;
    
    /**
     * @brief 设置样式 ID
     * @param style_id 样式 ID
     * @return 自身引用，支持链式调用
     */
    Cell& style_id(std::uint32_t style_id);
    
    /**
     * @brief 获取样式 ID
     * @return 样式 ID
     */
    std::uint32_t style_id() const;

public:
    /**
     * @brief 输出流操作符
     */
    friend std::ostream& operator<<(std::ostream& os, const Cell& cell);

public:
    /**
     * @brief 检查是否有自定义样式
     * @return 如果有自定义样式返回true
     */
    bool has_custom_style() const;

private:
    // 轻量级句柄：只包含工作簿实现指针、工作表ID和位置信息
    internal::workbook_impl* workbook_impl_;  // 使用原始指针提高性能
    std::uint32_t sheet_id_;                  // 使用ID而不是字符串
    std::size_t row_;
    std::size_t column_;

    friend class Worksheet;
    friend class Row;
};

// 为了支持字符数组，声明 value 函数的所有需要的特化
template<> Cell& Cell::value<std::string>(const std::string& value);
template<> Cell& Cell::value<int>(const int& value);
template<> Cell& Cell::value<double>(const double& value);
template<> Cell& Cell::value<bool>(const bool& value);

// 对于字符数组的通用模板实现
template<typename T>
Cell& Cell::value(const T& val) {
    // 对于字符数组，转换为 string
    if constexpr (std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>) {
        return value(std::string(val));
    } else {
        // 对于其他类型，尝试转换
        return value(static_cast<std::string>(val));
    }
}

// 模板特化声明（实现在 cell.cpp 中）
template<> std::string Cell::as<std::string>() const;
template<> int Cell::as<int>() const;
template<> double Cell::as<double>() const;
template<> bool Cell::as<bool>() const;

template<> std::optional<std::string> Cell::try_as<std::string>() const noexcept;
template<> std::optional<int> Cell::try_as<int>() const noexcept;
template<> std::optional<double> Cell::try_as<double>() const noexcept;
template<> std::optional<bool> Cell::try_as<bool>() const noexcept;



} // namespace tinakit::excel

/**
 * @example cell_usage.cpp
 * Cell 使用示例：
 * @code
 * #include <tinakit/excel/cell.hpp>
 * 
 * void cell_example() {
 *     using namespace tinakit::excel;
 *     
 *     auto workbook = Excel::create();
 *     auto sheet = workbook.add_sheet("Example");
 *     
 *     // 基本值设置
 *     sheet["A1"].value("Hello");
 *     sheet["B1"].value(42);
 *     sheet["C1"].value(3.14);
 *     sheet["D1"].value(true);
 *     
 *     // 链式样式设置
 *     sheet["A1"]
 *         .font("Arial", 14)
 *         .bold()
 *         .color(Color::Blue)
 *         .background_color(Color::LightGray);
 *     
 *     // 公式设置
 *     sheet["E1"].formula("=B1+C1");
 *     
 *     // 类型安全的值获取
 *     auto str_val = sheet["A1"].as<std::string>();
 *     auto int_val = sheet["B1"].try_as<int>();
 *     
 *     if (int_val) {
 *         std::cout << "B1 contains: " << *int_val << std::endl;
 *     }
 * }
 * @endcode
 */
