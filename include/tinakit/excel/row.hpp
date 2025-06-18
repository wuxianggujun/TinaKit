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

namespace tinakit::excel {

    class RowImpl;
    class Cell;

/**
 * @class Row
 * @brief Excel 行类
 * 
 * 代表 Excel 工作表中的一行，支持现代 C++ 迭代器和范围操作。
 * 提供对行中单元格的访问和管理功能。
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
     * @brief 构造函数（内部使用）
     */
    explicit Row(std::unique_ptr<RowImpl> impl);
    
    /**
     * @brief 静态工厂方法（内部使用）
     * @param index 行索引
     * @return 新的 Row 对象
     */
    static std::unique_ptr<Row> create(std::size_t index);
    
    /**
     * @brief 析构函数
     */
    ~Row();
    
    /**
     * @brief 移动构造函数
     */
    Row(Row&& other) noexcept;
    
    /**
     * @brief 移动赋值运算符
     */
    Row& operator=(Row&& other) noexcept;
    
    // 禁止拷贝
    Row(const Row&) = delete;
    Row& operator=(const Row&) = delete;

public:
    /**
     * @brief 按列名获取单元格
     * @param column_name 列名（如 "A", "B", "AA"）
     * @return 单元格引用
     */
    Cell& operator[](const std::string& column_name);
    
    /**
     * @brief 按列名获取单元格（只读）
     * @param column_name 列名（如 "A", "B", "AA"）
     * @return 单元格常量引用
     */
    const Cell& operator[](const std::string& column_name) const;
    
    /**
     * @brief 按列索引获取单元格
     * @param column_index 列索引（1-based）
     * @return 单元格引用
     */
    Cell& operator[](std::size_t column_index);
    
    /**
     * @brief 按列索引获取单元格（只读）
     * @param column_index 列索引（1-based）
     * @return 单元格常量引用
     */
    const Cell& operator[](std::size_t column_index) const;

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
     * @brief 检查是否为空行
     * @return 如果行为空返回 true
     */
    bool empty() const;
    
    /**
     * @brief 获取行中单元格数量
     * @return 单元格数量
     */
    std::size_t size() const;

public:
    /**
     * @brief 迭代器支持
     */
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
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
    std::unique_ptr<RowImpl> impl_;
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
    using pointer = Cell*;
    using reference = Cell&;

public:
    iterator() = default;
    explicit iterator(Row* row, std::size_t column);

    reference operator*() const;
    pointer operator->() const;
    iterator& operator++();
    iterator operator++(int);

    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;

private:
    Row* row_ = nullptr;
    std::size_t column_ = 0;
};

/**
 * @class Row::const_iterator
 * @brief 常量行迭代器
 */
class Row::const_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const Cell;
    using difference_type = std::ptrdiff_t;
    using pointer = const Cell*;
    using reference = const Cell&;

public:
    const_iterator() = default;
    explicit const_iterator(const Row* row, std::size_t column);

    reference operator*() const;
    pointer operator->() const;
    const_iterator& operator++();
    const_iterator operator++(int);

    bool operator==(const const_iterator& other) const;
    bool operator!=(const const_iterator& other) const;

private:
    const Row* row_ = nullptr;
    std::size_t column_ = 0;
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
