/**
 * @file range.hpp
 * @brief Excel 范围句柄类 - 重构后的唯一用户接口
 * @author TinaKit Team
 * @date 2025-6-20
 */

#pragma once

#include "types.hpp"
#include "range_view.hpp"
#include "tinakit/core/types.hpp"
#include "tinakit/core/color.hpp"
#include <string>
#include <memory>
#include <optional>

namespace tinakit::internal {
class workbook_impl;
} // namespace tinakit::internal

namespace tinakit::excel {

// 前向声明
class Worksheet;
class ConditionalFormatBuilder;
class StyleTemplate;

/**
 * @class Range
 * @brief Excel 范围句柄类 - 唯一面向用户的范围操作类
 *
 * 这是重构后的唯一面向用户的范围类。它是一个轻量级的句柄，
 * 负责提供所有针对这个区域的操作方法（如 set_value(), set_style(), clear()等），
 * 并且提供迭代器。
 *
 * 设计原则：
 * 1. 单一入口：用户操作范围的唯一接口
 * 2. 轻量级句柄：内部仅包含 worksheet_impl* 和 range_address
 * 3. 批量优化：所有操作都委托给 workbook_impl 进行批量优化
 * 4. 迭代支持：通过调用内部的 RangeView 来实现
 * 5. 链式调用：支持流畅的API设计
 *
 * @example
 * ```cpp
 * // 获取范围对象
 * auto my_range = worksheet.range("B2:D4");
 *
 * // 执行批量操作
 * my_range.set_style(some_style);
 *
 * // 遍历范围内的所有单元格
 * for (tinakit::excel::Cell c : my_range) {
 *     c.set_value("Hello");
 * }
 * ```
 */
class Range {
public:
    /**
     * @brief 默认构造函数（创建空范围）
     */
    Range() = default;

    /**
     * @brief 构造函数（由 worksheet 内部创建）
     * @param workbook_impl 工作簿实现的共享指针
     * @param sheet_name 工作表名称
     * @param range_addr 范围地址
     */
    Range(std::shared_ptr<internal::workbook_impl> workbook_impl,
          std::string sheet_name,
          core::range_address range_addr);

    /**
     * @brief 拷贝构造函数（轻量级）
     */
    Range(const Range& other) = default;

    /**
     * @brief 移动构造函数
     */
    Range(Range&& other) noexcept = default;

    /**
     * @brief 拷贝赋值运算符
     */
    Range& operator=(const Range& other) = default;

    /**
     * @brief 移动赋值运算符
     */
    Range& operator=(Range&& other) noexcept = default;

    /**
     * @brief 析构函数
     */
    ~Range() = default;

    // ========================================
    // 静态工厂方法
    // ========================================

    /**
     * @brief 从字符串创建范围（用于向后兼容）
     * @param range_str 范围字符串（如 "A1:C10"）
     * @param workbook_impl 工作簿实现
     * @param sheet_name 工作表名称
     * @return Range 对象
     */
    static Range from_string(const std::string& range_str,
                           std::shared_ptr<internal::workbook_impl> workbook_impl,
                           const std::string& sheet_name);

    // ========================================
    // 范围信息
    // ========================================

    /**
     * @brief 获取范围地址
     * @return 范围地址字符串（如 "A1:C10"）
     */
    std::string address() const;

    /**
     * @brief 获取范围字符串表示（向后兼容）
     * @return 范围字符串
     */
    std::string to_string() const;

    /**
     * @brief 获取起始位置
     * @return 起始位置
     */
    core::Coordinate start_position() const;

    /**
     * @brief 获取结束位置
     * @return 结束位置
     */
    core::Coordinate end_position() const;

    /**
     * @brief 获取起始位置（向后兼容）
     * @return 起始位置
     */
    core::Coordinate start() const { return start_position(); }

    /**
     * @brief 获取结束位置（向后兼容，但与迭代器end()冲突）
     * @return 结束位置
     * @note 此方法仅用于非迭代上下文
     */
    core::Coordinate end_coord() const { return end_position(); }

    /**
     * @brief 检查位置是否在范围内
     * @param pos 位置
     * @return 如果位置在范围内返回true
     */
    bool contains(const core::Coordinate& pos) const;

    /**
     * @brief 获取范围大小
     * @return 行数和列数的对
     */
    std::pair<std::size_t, std::size_t> size() const;

    /**
     * @brief 检查两个范围是否重叠
     * @param other 另一个范围
     * @return 如果重叠返回true
     */
    bool overlaps(const Range& other) const;

    /**
     * @brief 相等比较运算符
     * @param other 另一个范围
     * @return 如果相等返回true
     */
    bool operator==(const Range& other) const;

    /**
     * @brief 不等比较运算符
     * @param other 另一个范围
     * @return 如果不等返回true
     */
    bool operator!=(const Range& other) const;

    // ========================================
    // 批量操作
    // ========================================

    /**
     * @brief 设置所有单元格的值
     * @tparam T 值类型
     * @param value 要设置的值
     * @return 自身引用，支持链式调用
     */
    template<typename T>
    Range& set_value(const T& value);

    /**
     * @brief 应用样式模板到整个范围
     * @param style_template 样式模板
     * @return 自身引用，支持链式调用
     */
    Range& set_style(const StyleTemplate& style_template);

    /**
     * @brief 设置样式ID到整个范围
     * @param style_id 样式ID
     * @return 自身引用，支持链式调用
     */
    Range& set_style(std::uint32_t style_id);

    /**
     * @brief 清除范围内的所有内容
     * @return 自身引用，支持链式调用
     */
    Range& clear();

    // ========================================
    // 迭代支持 - 委托给 RangeView
    // ========================================

    /**
     * @brief 获取开始迭代器
     * @return 开始迭代器
     */
    auto begin() {
        if (!view_) view_ = RangeView(workbook_impl_, sheet_name_, range_addr_);
        return view_->begin();
    }

    /**
     * @brief 获取结束迭代器
     * @return 结束迭代器
     */
    auto end() {
        if (!view_) view_ = RangeView(workbook_impl_, sheet_name_, range_addr_);
        return view_->end();
    }

    /**
     * @brief 获取开始迭代器（const版本）
     * @return 开始迭代器
     */
    auto begin() const {
        if (!view_) const_cast<Range*>(this)->view_ = RangeView(workbook_impl_, sheet_name_, range_addr_);
        return view_->begin();
    }

    /**
     * @brief 获取结束迭代器（const版本）
     * @return 结束迭代器
     */
    auto end() const {
        if (!view_) const_cast<Range*>(this)->view_ = RangeView(workbook_impl_, sheet_name_, range_addr_);
        return view_->end();
    }

private:
    // 轻量级句柄：只包含必要的引用信息
    std::shared_ptr<internal::workbook_impl> workbook_impl_;
    std::string sheet_name_;
    core::range_address range_addr_;
    std::optional<RangeView> view_; // 视图对象用于迭代（可选，支持默认构造）
};

// 为字符数组提供特化声明
template<> Range& Range::set_value<const char*>(const char* const& value);

} // namespace tinakit::excel