/**
 * @file column.hpp
 * @brief Excel 列操作接口
 * @author TinaKit Team
 * @date 2025-6-21
 */

#pragma once

#include "tinakit/excel/cell.hpp"
#include <memory>
#include <vector>
#include <iterator>

// 前向声明
namespace tinakit::internal {
    class workbook_impl;
}

namespace tinakit::excel {

/**
 * @class Column
 * @brief Excel 列的抽象
 * 
 * 提供对Excel工作表中单个列的操作接口，包括：
 * - 列宽设置和获取
 * - 单元格访问
 * - 批量操作
 * - 迭代器支持
 * - 样式设置
 */
class Column {
public:
    /**
     * @brief 默认构造函数（创建无效句柄）
     */
    Column() = default;

    /**
     * @brief 构造函数
     * @param workbook_impl 工作簿实现指针
     * @param sheet_id 工作表ID
     * @param column_index 列索引（1-based）
     */
    Column(std::shared_ptr<tinakit::internal::workbook_impl> workbook_impl,
           std::uint32_t sheet_id,
           std::size_t column_index);

    /**
     * @brief 拷贝构造函数
     */
    Column(const Column& other) = default;

    /**
     * @brief 移动构造函数
     */
    Column(Column&& other) noexcept = default;

    /**
     * @brief 拷贝赋值运算符
     */
    Column& operator=(const Column& other) = default;

    /**
     * @brief 移动赋值运算符
     */
    Column& operator=(Column&& other) noexcept = default;

    // ========================================
    // 单元格访问
    // ========================================

    /**
     * @brief 通过行索引访问单元格
     * @param row_index 行索引（1-based）
     * @return 单元格对象
     */
    Cell operator[](std::size_t row_index) const;

    /**
     * @brief 通过行索引访问单元格（别名）
     * @param row_index 行索引（1-based）
     * @return 单元格对象
     */
    Cell cell(std::size_t row_index) const;

    // ========================================
    // 列属性
    // ========================================

    /**
     * @brief 获取列索引
     * @return 列索引（1-based）
     */
    std::size_t index() const noexcept;

    /**
     * @brief 获取列宽
     * @return 列宽（字符数）
     */
    double width() const noexcept;

    /**
     * @brief 设置列宽
     * @param width 列宽（字符数）
     */
    void set_width(double width);

    /**
     * @brief 设置列宽（链式调用版本）
     * @param width 列宽（字符数）
     * @return 当前列对象的引用
     */
    Column& width(double width);

    /**
     * @brief 检查列是否隐藏
     * @return 如果列隐藏返回 true
     */
    bool hidden() const noexcept;

    /**
     * @brief 设置列的隐藏状态
     * @param hidden 是否隐藏
     */
    void set_hidden(bool hidden);

    /**
     * @brief 设置列的隐藏状态（链式调用版本）
     * @param hidden 是否隐藏
     * @return 当前列对象的引用
     */
    Column& hidden(bool hidden);

    // ========================================
    // 状态查询
    // ========================================

    /**
     * @brief 检查列是否为空
     * @return 如果列中没有任何数据返回 true
     */
    bool empty() const;

    /**
     * @brief 获取列中最大行索引
     * @return 最大行索引，如果列为空返回 0
     */
    std::size_t size() const;

    /**
     * @brief 检查句柄是否有效
     * @return 如果句柄有效返回 true
     */
    bool valid() const noexcept;

    // ========================================
    // 批量操作
    // ========================================

    /**
     * @brief 批量设置列中的值
     * @param values 值列表
     * @param start_row 起始行索引（1-based，默认为1）
     */
    void set_values(const std::vector<Cell::CellValue>& values, std::size_t start_row = 1);

    /**
     * @brief 批量获取列中的值
     * @param start_row 起始行索引（1-based，默认为1）
     * @param count 获取的行数（0表示到列末尾）
     * @return 值列表
     */
    std::vector<Cell::CellValue> get_values(std::size_t start_row = 1, std::size_t count = 0) const;

    /**
     * @brief 清空整列
     */
    void clear();

    // ========================================
    // 迭代器支持
    // ========================================

    /**
     * @brief 列迭代器
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Cell;
        using difference_type = std::ptrdiff_t;
        using pointer = Cell*;
        using reference = Cell&;

        iterator() = default;
        iterator(const Column& column, std::size_t row_index);

        Cell operator*() const;
        iterator& operator++();
        iterator operator++(int);
        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;

    private:
        const Column* column_ = nullptr;
        std::size_t row_index_ = 0;
    };

    using const_iterator = iterator;

    /**
     * @brief 获取开始迭代器
     */
    iterator begin() const;

    /**
     * @brief 获取结束迭代器
     */
    iterator end() const;

    /**
     * @brief 获取常量开始迭代器
     */
    const_iterator cbegin() const;

    /**
     * @brief 获取常量结束迭代器
     */
    const_iterator cend() const;

private:
    std::shared_ptr<tinakit::internal::workbook_impl> workbook_impl_;
    std::uint32_t sheet_id_ = 0;
    std::size_t column_index_ = 0;

    friend class Worksheet;
};

} // namespace tinakit::excel
