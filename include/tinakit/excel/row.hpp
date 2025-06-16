/**
 * @file row.hpp
 * @brief Excel 行类定义
 * @author TinaKit Team
 * @date 2024-12-16
 */

#pragma once

#include "../core/types.hpp"
#include "cell.hpp"
#include <vector>
#include <ranges>

namespace tinakit {

/**
 * @class Row
 * @brief Excel 行类
 * 
 * 代表 Excel 工作表中的一行，提供对该行中所有单元格的访问。
 * 支持现代 C++ 的范围操作和迭代器。
 */
class Row {
public:
    /**
     * @brief 行迭代器类型
     */
    class iterator;
    class const_iterator;

public:
    /**
     * @brief 构造函数
     * @param impl 实现细节指针
     */
    explicit Row(std::unique_ptr<RowImpl> impl);
    
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
     * @param column_name 列名
     * @return 单元格常量引用
     */
    const Cell& operator[](const std::string& column_name) const;
    
    /**
     * @brief 按列索引获取单元格
     * @param column_index 列索引（从 1 开始）
     * @return 单元格引用
     */
    Cell& operator[](std::size_t column_index);
    
    /**
     * @brief 按列索引获取单元格（只读）
     * @param column_index 列索引（从 1 开始）
     * @return 单元格常量引用
     */
    const Cell& operator[](std::size_t column_index) const;

public:
    /**
     * @brief 获取行号
     * @return 行号（从 1 开始）
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
     * @brief 检查行是否为空
     * @return 如果行中所有单元格都为空返回 true
     */
    bool empty() const;
    
    /**
     * @brief 获取行中非空单元格的数量
     * @return 非空单元格数量
     */
    std::size_t cell_count() const;
    
    /**
     * @brief 获取行中最后一个非空单元格的列索引
     * @return 最后一个非空单元格的列索引，如果行为空返回 0
     */
    std::size_t last_column() const;

public:
    /**
     * @brief 获取所有单元格
     * @return 单元格列表
     */
    std::vector<std::reference_wrapper<Cell>> cells();
    
    /**
     * @brief 获取所有单元格（只读）
     * @return 单元格常量列表
     */
    std::vector<std::reference_wrapper<const Cell>> cells() const;
    
    /**
     * @brief 获取指定范围的单元格
     * @param start_column 起始列索引
     * @param end_column 结束列索引
     * @return 单元格列表
     */
    std::vector<std::reference_wrapper<Cell>> cells(std::size_t start_column, 
                                                   std::size_t end_column);

public:
    /**
     * @brief 开始迭代器
     * @return 指向第一个单元格的迭代器
     */
    iterator begin();
    
    /**
     * @brief 结束迭代器
     * @return 指向最后一个单元格之后的迭代器
     */
    iterator end();
    
    /**
     * @brief 开始迭代器（只读）
     * @return 指向第一个单元格的常量迭代器
     */
    const_iterator begin() const;
    
    /**
     * @brief 结束迭代器（只读）
     * @return 指向最后一个单元格之后的常量迭代器
     */
    const_iterator end() const;
    
    /**
     * @brief 常量开始迭代器
     * @return 指向第一个单元格的常量迭代器
     */
    const_iterator cbegin() const;
    
    /**
     * @brief 常量结束迭代器
     * @return 指向最后一个单元格之后的常量迭代器
     */
    const_iterator cend() const;

public:
    /**
     * @brief 清空整行
     */
    void clear();
    
    /**
     * @brief 隐藏行
     * @param hidden 是否隐藏
     */
    void set_hidden(bool hidden);
    
    /**
     * @brief 检查行是否隐藏
     * @return 如果隐藏返回 true
     */
    bool is_hidden() const noexcept;

private:
    std::unique_ptr<RowImpl> impl_;  ///< 实现细节（PIMPL 模式）
};

/**
 * @class Row::iterator
 * @brief 行迭代器类
 */
class Row::iterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = Cell;
    using difference_type = std::ptrdiff_t;
    using pointer = Cell*;
    using reference = Cell&;

public:
    iterator(Row* row, std::size_t column);
    
    reference operator*() const;
    pointer operator->() const;
    
    iterator& operator++();
    iterator operator++(int);
    iterator& operator--();
    iterator operator--(int);
    
    iterator& operator+=(difference_type n);
    iterator& operator-=(difference_type n);
    
    iterator operator+(difference_type n) const;
    iterator operator-(difference_type n) const;
    
    difference_type operator-(const iterator& other) const;
    
    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;
    bool operator<(const iterator& other) const;
    bool operator<=(const iterator& other) const;
    bool operator>(const iterator& other) const;
    bool operator>=(const iterator& other) const;

private:
    Row* row_;
    std::size_t column_;
};

/**
 * @class Row::const_iterator
 * @brief 行常量迭代器类
 */
class Row::const_iterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = const Cell;
    using difference_type = std::ptrdiff_t;
    using pointer = const Cell*;
    using reference = const Cell&;

public:
    const_iterator(const Row* row, std::size_t column);
    const_iterator(const iterator& it);
    
    reference operator*() const;
    pointer operator->() const;
    
    const_iterator& operator++();
    const_iterator operator++(int);
    const_iterator& operator--();
    const_iterator operator--(int);
    
    const_iterator& operator+=(difference_type n);
    const_iterator& operator-=(difference_type n);
    
    const_iterator operator+(difference_type n) const;
    const_iterator operator-(difference_type n) const;
    
    difference_type operator-(const const_iterator& other) const;
    
    bool operator==(const const_iterator& other) const;
    bool operator!=(const const_iterator& other) const;
    bool operator<(const const_iterator& other) const;
    bool operator<=(const const_iterator& other) const;
    bool operator>(const const_iterator& other) const;
    bool operator>=(const const_iterator& other) const;

private:
    const Row* row_;
    std::size_t column_;
};

} // namespace tinakit

// 支持 std::ranges
template<>
inline constexpr bool std::ranges::enable_borrowed_range<tinakit::Row> = true;

/**
 * @example row_usage.cpp
 * Row 使用示例：
 * @code
 * #include <tinakit/excel/row.hpp>
 * #include <ranges>
 * 
 * void row_example() {
 *     using namespace tinakit;
 *     using namespace std::ranges;
 *     
 *     auto workbook = Excel::open("data.xlsx");
 *     auto sheet = workbook[0];
 *     auto row = sheet.row(2);  // 第二行
 *     
 *     // 访问单元格
 *     row["A"].value("姓名");
 *     row["B"].value("年龄");
 *     row[3].value("城市");  // C 列
 *     
 *     // 迭代行中的单元格
 *     for (auto& cell : row) {
 *         if (!cell.empty()) {
 *             std::cout << cell.address() << ": " << cell << std::endl;
 *         }
 *     }
 *     
 *     // 使用 ranges 处理
 *     auto non_empty_cells = row 
 *         | views::filter([](const Cell& cell) { return !cell.empty(); })
 *         | views::transform([](const Cell& cell) { return cell.to_string(); })
 *         | to<std::vector>();
 * }
 * @endcode
 */
