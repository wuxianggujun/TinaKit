/**
 * @file cell.hpp
 * @brief Excel 单元格类定义
 * @author TinaKit Team
 * @date 2024-12-16
 */

#pragma once

#include "../core/types.hpp"
#include "../formatting/style.hpp"
#include <string>
#include <optional>
#include <variant>
#include <chrono>

namespace tinakit {

/**
 * @class Cell
 * @brief Excel 单元格类
 * 
 * 代表 Excel 工作表中的一个单元格，支持读取和设置值、格式、公式等。
 * 支持多种数据类型：字符串、数字、日期、布尔值等。
 */
class Cell {
public:
    /**
     * @brief 单元格值类型
     */
    using CellValue = std::variant<
        std::monostate,  // 空值
        std::string,     // 文本
        double,          // 数字
        bool,            // 布尔值
        std::chrono::system_clock::time_point  // 日期时间
    >;

public:
    /**
     * @brief 构造函数
     * @param impl 实现细节指针
     */
    explicit Cell(std::unique_ptr<CellImpl> impl);
    
    /**
     * @brief 析构函数
     */
    ~Cell();
    
    /**
     * @brief 移动构造函数
     */
    Cell(Cell&& other) noexcept;
    
    /**
     * @brief 移动赋值运算符
     */
    Cell& operator=(Cell&& other) noexcept;
    
    // 禁止拷贝
    Cell(const Cell&) = delete;
    Cell& operator=(const Cell&) = delete;

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
     * @tparam T 期望的值类型
     * @return 转换后的值
     * @throws TypeConversionException 类型转换失败
     */
    template<typename T>
    T as() const;
    
    /**
     * @brief 尝试获取单元格值
     * @tparam T 期望的值类型
     * @return 如果转换成功返回值，否则返回空
     */
    template<typename T>
    std::optional<T> try_as() const noexcept;
    
    /**
     * @brief 获取原始单元格值
     * @return 单元格值的 variant
     */
    const CellValue& raw_value() const noexcept;

public:
    /**
     * @brief 设置公式
     * @param formula 公式字符串（不包含等号）
     * @return 自身引用，支持链式调用
     */
    Cell& formula(const std::string& formula);
    
    /**
     * @brief 获取公式
     * @return 公式字符串，如果不是公式则返回空
     */
    std::optional<std::string> formula() const;
    
    /**
     * @brief 检查是否为公式单元格
     * @return 如果是公式返回 true
     */
    bool is_formula() const noexcept;

public:
    /**
     * @brief 检查单元格是否为空
     * @return 如果为空返回 true
     */
    bool empty() const noexcept;
    
    /**
     * @brief 清空单元格内容
     * @return 自身引用，支持链式调用
     */
    Cell& clear();
    
    /**
     * @brief 获取单元格地址
     * @return 单元格地址字符串（如 "A1"）
     */
    std::string address() const;
    
    /**
     * @brief 获取行号
     * @return 行号（从 1 开始）
     */
    std::size_t row() const noexcept;
    
    /**
     * @brief 获取列号
     * @return 列号（从 1 开始）
     */
    std::size_t column() const noexcept;

public:
    // 样式设置方法（链式调用）
    
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
     * @brief 设置字体大小
     * @param size 字体大小
     * @return 自身引用，支持链式调用
     */
    Cell& font_size(double size);
    
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
    Cell& align(Alignment alignment);
    
    /**
     * @brief 设置边框
     * @param border_type 边框类型
     * @param style 边框样式
     * @return 自身引用，支持链式调用
     */
    Cell& border(BorderType border_type, BorderStyle style);
    
    /**
     * @brief 设置数字格式
     * @param format 格式字符串
     * @return 自身引用，支持链式调用
     */
    Cell& number_format(const std::string& format);

public:
    /**
     * @brief 获取样式
     * @return 单元格样式
     */
    const CellStyle& style() const;
    
    /**
     * @brief 设置样式
     * @param style 单元格样式
     * @return 自身引用，支持链式调用
     */
    Cell& style(const CellStyle& style);

public:
    /**
     * @brief 转换为字符串（用于输出）
     * @return 单元格值的字符串表示
     */
    std::string to_string() const;
    
    /**
     * @brief 流输出运算符
     */
    friend std::ostream& operator<<(std::ostream& os, const Cell& cell);

private:
    std::unique_ptr<CellImpl> impl_;  ///< 实现细节（PIMPL 模式）
};

// 模板方法的特化声明
template<> Cell& Cell::value<std::string>(const std::string& value);
template<> Cell& Cell::value<const char*>(const char* const& value);
template<> Cell& Cell::value<double>(const double& value);
template<> Cell& Cell::value<int>(const int& value);
template<> Cell& Cell::value<bool>(const bool& value);
template<> Cell& Cell::value<std::chrono::system_clock::time_point>(
    const std::chrono::system_clock::time_point& value);

template<> std::string Cell::as<std::string>() const;
template<> double Cell::as<double>() const;
template<> int Cell::as<int>() const;
template<> bool Cell::as<bool>() const;
template<> std::chrono::system_clock::time_point Cell::as<std::chrono::system_clock::time_point>() const;

} // namespace tinakit

/**
 * @example cell_usage.cpp
 * Cell 使用示例：
 * @code
 * #include <tinakit/excel/cell.hpp>
 * 
 * void cell_example() {
 *     using namespace tinakit;
 *     
 *     auto workbook = Excel::create();
 *     auto sheet = workbook.add_sheet("示例");
 *     
 *     // 设置值和样式（链式调用）
 *     sheet["A1"]
 *         .value("标题")
 *         .font("Arial", 14)
 *         .bold()
 *         .color(Color::Blue)
 *         .background_color(Color::LightGray)
 *         .align(Alignment::Center);
 *     
 *     // 设置数字
 *     sheet["B1"].value(123.45).number_format("¥#,##0.00");
 *     
 *     // 设置公式
 *     sheet["C1"].formula("SUM(A1:B1)");
 *     
 *     // 类型安全的值获取
 *     auto text = sheet["A1"].as<std::string>();
 *     auto number = sheet["B1"].try_as<double>();
 *     
 *     if (number) {
 *         std::cout << "数值: " << *number << std::endl;
 *     }
 * }
 * @endcode
 */
