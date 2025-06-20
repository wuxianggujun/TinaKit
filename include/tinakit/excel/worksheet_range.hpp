/**
 * @file worksheet_range.hpp
 * @brief Excel 工作表范围操作类
 * @author TinaKit Team
 * @date 2025-01-19
 */

#pragma once

#include "tinakit/excel/types.hpp"
#include "tinakit/excel/range.hpp"
#include "tinakit/core/color.hpp"
#include <string>
#include <memory>
#include <vector>

namespace tinakit::internal {
class workbook_impl;
} // namespace tinakit::internal

namespace tinakit::excel {

// 前向声明
class Cell;
class Worksheet;
class StyleTemplate;

/**
 * @class WorksheetRange
 * @brief Excel 工作表范围轻量级句柄类
 *
 * 这是一个轻量级的句柄类，本身不持有任何重型数据。
 * 所有实际的数据和操作都委托给 internal::workbook_impl。
 *
 * 核心设计原则：
 * 1. 轻量级：只包含 workbook_impl 指针、工作表名称和范围信息
 * 2. 可复制：复制成本极低，安全共享同一个范围
 * 3. 委托模式：所有操作都委托给 workbook_impl
 * 4. 批量优化：范围操作在 workbook_impl 中进行批量优化
 *
 * @example
 * ```cpp
 * // 获取范围
 * auto range = sheet.range("A1:C10");
 *
 * // 批量样式设置
 * range.style(StyleTemplates::header());
 *
 * // 批量边框
 * range.border(BorderType::All, BorderStyle::Thin);
 *
 * // 批量背景色
 * range.background_color(Color::LightGray);
 *
 * // 遍历单元格
 * for (auto& cell : range) {
 *     cell.value("数据");
 * }
 * ```
 *
 * @note 使用句柄-实现分离模式，提供稳定的 ABI 和优秀的性能
 */
class WorksheetRange {
public:
    /**
     * @brief 构造函数（由 worksheet 内部创建）
     * @param workbook_impl 工作簿实现的共享指针
     * @param sheet_name 工作表名称
     * @param range 范围定义
     */
    WorksheetRange(std::shared_ptr<internal::workbook_impl> workbook_impl,
                   std::string sheet_name,
                   Range range);

    /**
     * @brief 拷贝构造函数（轻量级，共享同一个实现）
     */
    WorksheetRange(const WorksheetRange& other) = default;

    /**
     * @brief 移动构造函数
     */
    WorksheetRange(WorksheetRange&& other) noexcept = default;

    /**
     * @brief 拷贝赋值运算符
     */
    WorksheetRange& operator=(const WorksheetRange& other) = default;

    /**
     * @brief 移动赋值运算符
     */
    WorksheetRange& operator=(WorksheetRange&& other) noexcept = default;

    /**
     * @brief 析构函数
     */
    ~WorksheetRange() = default;

    // ========================================
    // 范围信息
    // ========================================
    
    /**
     * @brief 获取范围地址
     * @return 范围地址字符串（如 "A1:C10"）
     */
    std::string address() const;
    
    /**
     * @brief 获取起始行
     * @return 起始行号（1-based）
     */
    std::size_t start_row() const;
    
    /**
     * @brief 获取起始列
     * @return 起始列号（1-based）
     */
    std::size_t start_column() const;
    
    /**
     * @brief 获取结束行
     * @return 结束行号（1-based）
     */
    std::size_t end_row() const;
    
    /**
     * @brief 获取结束列
     * @return 结束列号（1-based）
     */
    std::size_t end_column() const;
    
    /**
     * @brief 获取行数
     * @return 行数
     */
    std::size_t row_count() const;
    
    /**
     * @brief 获取列数
     * @return 列数
     */
    std::size_t column_count() const;
    
    /**
     * @brief 获取单元格总数
     * @return 单元格总数
     */
    std::size_t cell_count() const;

    // ========================================
    // 单元格访问
    // ========================================
    
    /**
     * @brief 获取指定位置的单元格
     * @param row 相对行号（0-based）
     * @param col 相对列号（0-based）
     * @return 单元格引用
     */
    Cell& cell(std::size_t row, std::size_t col);
    
    /**
     * @brief 获取指定位置的单元格（const版本）
     * @param row 相对行号（0-based）
     * @param col 相对列号（0-based）
     * @return 单元格引用
     */
    const Cell& cell(std::size_t row, std::size_t col) const;

    // ========================================
    // 批量样式设置
    // ========================================
    
    /**
     * @brief 应用样式模板到整个范围
     * @param style_template 样式模板
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& style(const StyleTemplate& style_template);
    
    /**
     * @brief 设置字体
     * @param font_name 字体名称
     * @param size 字体大小
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& font(const std::string& font_name, double size = 11.0);
    
    /**
     * @brief 设置粗体
     * @param bold 是否粗体
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& bold(bool bold = true);
    
    /**
     * @brief 设置斜体
     * @param italic 是否斜体
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& italic(bool italic = true);
    
    /**
     * @brief 设置字体颜色
     * @param color 字体颜色
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& color(const Color& color);
    
    /**
     * @brief 设置背景颜色
     * @param color 背景颜色
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& background_color(const Color& color);
    
    /**
     * @brief 设置对齐方式
     * @param alignment 对齐方式
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& align(const Alignment& alignment);

    /**
     * @brief 设置水平对齐
     * @param horizontal 水平对齐方式
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& align_horizontal(Alignment::Horizontal horizontal);

    /**
     * @brief 设置垂直对齐
     * @param vertical 垂直对齐方式
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& align_vertical(Alignment::Vertical vertical);
    
    /**
     * @brief 设置边框
     * @param type 边框类型
     * @param style 边框样式
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& border(BorderType type, BorderStyle style);
    
    /**
     * @brief 设置彩色边框
     * @param type 边框类型
     * @param style 边框样式
     * @param color 边框颜色
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& border(BorderType type, BorderStyle style, const Color& color);
    
    /**
     * @brief 设置数字格式
     * @param format_code 格式代码
     * @return 自身引用，支持链式调用
     */
    WorksheetRange& number_format(const std::string& format_code);

    // ========================================
    // 批量值设置
    // ========================================
    
    /**
     * @brief 设置所有单元格的值
     * @tparam T 值类型
     * @param value 要设置的值
     * @return 自身引用，支持链式调用
     */
    template<typename T>
    WorksheetRange& value(const T& value);

    /**
     * @brief 设置所有单元格的值（字符串字面量特化）
     * @tparam N 字符串长度
     * @param value 字符串字面量
     * @return 自身引用，支持链式调用
     */
    template<std::size_t N>
    WorksheetRange& value(const char (&value)[N]);
    
    /**
     * @brief 使用二维数组设置值
     * @tparam T 值类型
     * @param values 二维值数组
     * @return 自身引用，支持链式调用
     */
    template<typename T>
    WorksheetRange& values(const std::vector<std::vector<T>>& values);

    // ========================================
    // 迭代器支持
    // ========================================

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Cell;
        using difference_type = std::ptrdiff_t;
        using pointer = Cell*;
        using reference = Cell&;

        iterator(WorksheetRange& range, std::size_t index);

        reference operator*();
        pointer operator->();
        iterator& operator++();
        iterator operator++(int);

        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;

    private:
        WorksheetRange& range_;
        std::size_t index_;
    };

    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const Cell;
        using difference_type = std::ptrdiff_t;
        using pointer = const Cell*;
        using reference = const Cell&;

        const_iterator(const WorksheetRange& range, std::size_t index);

        reference operator*() const;
        pointer operator->() const;
        const_iterator& operator++();
        const_iterator operator++(int);

        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;

    private:
        const WorksheetRange& range_;
        std::size_t index_;
    };
    
    /**
     * @brief 获取开始迭代器
     * @return 开始迭代器
     */
    iterator begin();
    
    /**
     * @brief 获取结束迭代器
     * @return 结束迭代器
     */
    iterator end();
    
    /**
     * @brief 获取开始迭代器（const版本）
     * @return 开始迭代器
     */
    const_iterator begin() const;
    
    /**
     * @brief 获取结束迭代器（const版本）
     * @return 结束迭代器
     */
    const_iterator end() const;
    
    /**
     * @brief 获取开始迭代器（const版本）
     * @return 开始迭代器
     */
    const_iterator cbegin() const;
    
    /**
     * @brief 获取结束迭代器（const版本）
     * @return 结束迭代器
     */
    const_iterator cend() const;

private:
    // 轻量级句柄：只包含工作簿实现指针、工作表名称和范围信息
    std::shared_ptr<internal::workbook_impl> workbook_impl_;
    std::string sheet_name_;
    Range range_;
};

} // namespace tinakit::excel
