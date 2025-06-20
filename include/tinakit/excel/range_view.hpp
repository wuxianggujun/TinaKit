/**
 * @file range_view.hpp
 * @brief Excel 范围视图/迭代器类 - 重构后的worksheet_range
 * @author TinaKit Team
 * @date 2025-6-20
 */

#pragma once

#include "tinakit/excel/types.hpp"
#include "tinakit/core/types.hpp"
#include <memory>
#include <iterator>

namespace tinakit::internal {
class workbook_impl;
} // namespace tinakit::internal

namespace tinakit::excel {

// 前向声明
class Cell;

/**
 * @class RangeView
 * @brief Excel 范围视图/迭代器类
 *
 * 这个类的职责是作为 excel::Range 的迭代器实现，
 * 让用户可以用 for-each 循环遍历区域内的所有单元格。
 *
 * 设计原则：
 * 1. 标准C++迭代器：符合STL迭代器规范
 * 2. 轻量级视图：不持有数据，只提供访问接口
 * 3. 单一职责：只负责迭代，不负责批量操作
 * 4. 高性能：优化的迭代实现
 *
 * @example
 * ```cpp
 * auto range = sheet.range("A1:C3");
 * 
 * // 使用range_view进行迭代
 * for (auto& cell : range) {
 *     cell.value("数据");
 * }
 * ```
 *
 * @note 这是重构后的worksheet_range，专注于迭代功能
 */
class RangeView {
public:
    /**
     * @brief 构造函数
     * @param workbook_impl 工作簿实现的共享指针
     * @param sheet_name 工作表名称
     * @param range_addr 范围地址
     */
    RangeView(std::shared_ptr<internal::workbook_impl> workbook_impl,
              std::string sheet_name,
              core::range_address range_addr);

    /**
     * @brief 拷贝构造函数（轻量级）
     */
    RangeView(const RangeView& other) = default;

    /**
     * @brief 移动构造函数
     */
    RangeView(RangeView&& other) noexcept = default;

    /**
     * @brief 拷贝赋值运算符
     */
    RangeView& operator=(const RangeView& other) = default;

    /**
     * @brief 移动赋值运算符
     */
    RangeView& operator=(RangeView&& other) noexcept = default;

    /**
     * @brief 析构函数
     */
    ~RangeView() = default;

    // ========================================
    // 范围信息（只读）
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
    // 标准C++迭代器接口
    // ========================================

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Cell;
        using difference_type = std::ptrdiff_t;
        using pointer = Cell*;
        using reference = Cell&;

        iterator(RangeView& range, std::size_t index);

        reference operator*();
        pointer operator->();
        iterator& operator++();
        iterator operator++(int);

        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;

    private:
        RangeView& range_;
        std::size_t index_;
    };

    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const Cell;
        using difference_type = std::ptrdiff_t;
        using pointer = const Cell*;
        using reference = const Cell&;

        const_iterator(const RangeView& range, std::size_t index);

        reference operator*() const;
        pointer operator->() const;
        const_iterator& operator++();
        const_iterator operator++(int);

        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;

    private:
        const RangeView& range_;
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
    // 轻量级视图：只包含必要的引用信息
    std::shared_ptr<internal::workbook_impl> workbook_impl_;
    std::string sheet_name_;
    core::range_address range_addr_;
};

} // namespace tinakit::excel
