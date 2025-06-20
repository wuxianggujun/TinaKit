/**
 * @file range.hpp
 * @brief Excel 范围类定义
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include "types.hpp"
#include <string>

namespace tinakit::excel {

// 前向声明
class Worksheet;
class ConditionalFormatBuilder;

/**
 * @class Range
 * @brief Excel 范围类
 * 
 * 代表 Excel 工作表中的一个单元格范围（如 A1:C10）。
 */
class Range {
public:
    /**
     * @brief 默认构造函数（创建空范围）
     */
    Range() = default;

    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     */
    Range(const Position& start, const Position& end);
    
    /**
     * @brief 从字符串创建范围
     * @param range_str 范围字符串（如 "A1:C10"）
     * @return Range 对象
     */
    static Range from_string(const std::string& range_str);
    
    /**
     * @brief 获取起始位置
     * @return 起始位置
     */
    const Position& start() const noexcept;
    
    /**
     * @brief 获取结束位置
     * @return 结束位置
     */
    const Position& end() const noexcept;
    
    /**
     * @brief 获取范围字符串表示
     * @return 范围字符串
     */
    std::string to_string() const;

    /**
     * @brief 检查位置是否在范围内
     * @param pos 位置
     * @return 如果位置在范围内返回true
     */
    bool contains(const Position& pos) const;

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

    /**
     * @brief 创建条件格式构建器（需要工作表上下文）
     * @param worksheet 工作表引用
     * @return 条件格式构建器
     */
    ConditionalFormatBuilder conditional_format(Worksheet& worksheet) const;

private:
    Position start_;
    Position end_;
};

} // namespace tinakit::excel