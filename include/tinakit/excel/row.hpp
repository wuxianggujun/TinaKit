/**
 * @file row.hpp
 * @brief Excel 行类定义
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include "cell.hpp"
#include <memory>
#include <iterator>

namespace tinakit {
namespace internal {
    class workbook_impl;
}
}

namespace tinakit::excel {

/**
 * @class Row
 * @brief Excel 行轻量级句柄类
 *
 * 这是一个轻量级的句柄类，本身不持有任何重型数据。
 * 所有实际的数据和操作都委托给 internal::workbook_impl。
 *
 * 核心设计原则：
 * 1. 轻量级：只包含 workbook_impl 指针、工作表ID和行索引
 * 2. 可复制：复制成本极低，安全共享同一行
 * 3. 委托模式：所有操作都委托给 workbook_impl
 * 4. 惰性求值：数据按需加载和保存
 *
 * @note 使用句柄-实现分离模式，提供稳定的 ABI 和优秀的性能
 */
class Row {
public:
    /**
     * @brief 迭代器类型
     */
    class iterator;
    class const_iterator;

public:
    /**
     * @brief 轻量级句柄构造函数
     * @param workbook_impl workbook实现指针
     * @param sheet_id 工作表ID
     * @param row_index 行索引（1-based）
     */
    Row(internal::workbook_impl* workbook_impl,
        std::uint32_t sheet_id,
        std::size_t row_index) noexcept;

    /**
     * @brief 默认构造函数（创建无效句柄）
     */
    Row() = default;

    /**
     * @brief 拷贝构造函数（轻量级）
     */
    Row(const Row& other) = default;

    /**
     * @brief 拷贝赋值运算符（轻量级）
     */
    Row& operator=(const Row& other) = default;

    /**
     * @brief 移动构造函数
     */
    Row(Row&& other) noexcept = default;

    /**
     * @brief 移动赋值运算符
     */
    Row& operator=(Row&& other) noexcept = default;

public:
    /**
     * @brief 按列名获取单元格
     * @param column_name 列名（如 "A", "B", "AA"）
     * @return 单元格句柄
     */
    Cell operator[](const std::string& column_name) const;

    /**
     * @brief 按列索引获取单元格
     * @param column_index 列索引（1-based）
     * @return 单元格句柄
     */
    Cell operator[](std::size_t column_index) const;

public:
    /**
     * @brief 获取行索引
     * @return 行索引（1-based）
     */
    std::size_t index() const noexcept;

    /**
     * @brief 获取行高
     * @return 行高（磅）
     */
    double height() const noexcept;

    /**
     * @brief 设置行高
     * @param height 行高（磅）
     */
    void set_height(double height);

    /**
     * @brief 设置行高（链式调用版本）
     * @param height 行高（磅）
     * @return 自身引用，支持链式调用
     */
    Row& height(double height);

    /**
     * @brief 检查是否为空行
     * @return 如果行为空返回 true
     */
    bool empty() const;

    /**
     * @brief 获取行中单元格数量（最大列索引）
     * @return 单元格数量
     */
    std::size_t size() const;

    /**
     * @brief 检查句柄是否有效
     * @return 如果句柄有效返回 true
     */
    bool valid() const noexcept;

    /**
     * @brief 批量设置行中的值
     * @param values 值列表
     * @param start_column 起始列索引（1-based，默认为1）
     */
    void set_values(const std::vector<Cell::CellValue>& values, std::size_t start_column = 1);

    /**
     * @brief 批量获取行中的值
     * @param start_column 起始列索引（1-based，默认为1）
     * @param count 获取的列数（0表示到行末尾）
     * @return 值列表
     */
    std::vector<Cell::CellValue> get_values(std::size_t start_column = 1, std::size_t count = 0) const;

    /**
     * @brief 清空整行
     */
    void clear();

public:
    /**
     * @brief 迭代器支持
     */
    iterator begin() const;
    iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

public:
    /**
     * @brief 获取指定类型的值（用于 ranges 操作）
     * @tparam T 值类型
     * @param column_index 列索引
     * @return 值
     */
    template<typename T>
    T as(std::size_t column_index) const {
        return (*this)[column_index].as<T>();
    }

private:
    internal::workbook_impl* workbook_impl_ = nullptr;
    std::uint32_t sheet_id_ = 0;
    std::size_t row_index_ = 0;

    friend class Worksheet;
};

/**
 * @class Row::iterator
 * @brief 行迭代器，用于遍历行中的单元格
 */
class Row::iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Cell;
    using difference_type = std::ptrdiff_t;
    using pointer = Cell;  // 返回值类型，不是指针
    using reference = Cell; // 返回值类型，不是引用

public:
    iterator() = default;
    explicit iterator(const Row& row, std::size_t column);

    reference operator*() const;
    pointer operator->() const;
    iterator& operator++();
    iterator operator++(int);

    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;

private:
    Row row_;
    std::size_t column_ = 0;
    std::size_t max_column_ = 0;
};

/**
 * @class Row::const_iterator
 * @brief 常量行迭代器
 */
class Row::const_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Cell;
    using difference_type = std::ptrdiff_t;
    using pointer = Cell;  // 返回值类型，不是指针
    using reference = Cell; // 返回值类型，不是引用

public:
    const_iterator() = default;
    explicit const_iterator(const Row& row, std::size_t column);

    reference operator*() const;
    pointer operator->() const;
    const_iterator& operator++();
    const_iterator operator++(int);

    bool operator==(const const_iterator& other) const;
    bool operator!=(const const_iterator& other) const;

private:
    Row row_;
    std::size_t column_ = 0;
    std::size_t max_column_ = 0;
};

} // namespace tinakit::excel

/**
 * @example row_usage.cpp
 * Row 使用示例：
 * @code
 * #include <tinakit/excel/row.hpp>
 * 
 * void row_example() {
 *     using namespace tinakit::excel;
 *     
 *     auto workbook = Excel::create();
 *     auto sheet = workbook.add_sheet("Example");
 *     
 *     // 获取行
 *     auto& row1 = sheet.row(1);
 *     
 *     // 按列名访问单元格
 *     row1["A"].value("Name");
 *     row1["B"].value("Age");
 *     row1["C"].value("Score");
 *     
 *     // 按列索引访问单元格
 *     row1[1].value("John");
 *     row1[2].value(25);
 *     row1[3].value(95.5);
 *     
 *     // 设置行高
 *     row1.set_height(20.0);
 *     
 *     // 迭代器遍历
 *     for (auto& cell : row1) {
 *         if (!cell.empty()) {
 *             std::cout << cell.address() << ": " << cell.to_string() << std::endl;
 *         }
 *     }
 *     
 *     // STL 算法支持
 *     auto non_empty_count = std::count_if(row1.begin(), row1.end(),
 *         [](const Cell& cell) { return !cell.empty(); });
 * }
 * @endcode
 */
