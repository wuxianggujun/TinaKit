/**
 * @file cell.hpp
 * @brief Excel 单元格类定义
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include "tinakit/core/types.hpp"
#include <string>
#include <memory>
#include <optional>
#include <variant>
#include <type_traits>

namespace tinakit::excel {

    class CellImpl;

/**
 * @class Cell
 * @brief Excel 单元格类
 * 
 * 代表 Excel 工作表中的一个单元格，支持多种数据类型和样式设置。
 * 提供类型安全的值访问和链式样式设置。
 */
class Cell {
public:
    /**
     * @brief 单元格值类型
     */
    using CellValue = std::variant<std::string, double, int, bool>;

public:
    /**
     * @brief 构造函数（内部使用）
     */
    explicit Cell(std::unique_ptr<CellImpl> impl);
    
    /**
     * @brief 静态工厂方法（内部使用）
     * @param row 行号
     * @param column 列号
     * @return 新的 Cell 对象
     */
    static std::unique_ptr<Cell> create(std::size_t row, std::size_t column);
    
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
    Cell& align(Alignment alignment);
    
    /**
     * @brief 设置边框
     * @param border_type 边框类型
     * @param style 边框样式
     * @return 自身引用，支持链式调用
     */
    Cell& border(BorderType border_type, BorderStyle style);

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

private:
    std::unique_ptr<CellImpl> impl_;
    friend class Worksheet;
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

// as() 函数的通用实现
template<typename T>
T Cell::as() const {
    throw TypeConversionException("Cell", typeid(T).name(), "Unsupported type conversion");
}

// try_as() 函数的通用实现
template<typename T>
std::optional<T> Cell::try_as() const noexcept {
    try {
        return as<T>();
    } catch (...) {
        return std::nullopt;
    }
}

// as() 函数的特化声明
template<> std::string Cell::as<std::string>() const;
template<> int Cell::as<int>() const;
template<> double Cell::as<double>() const;
template<> bool Cell::as<bool>() const;



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
