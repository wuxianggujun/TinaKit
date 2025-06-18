/**
 * @file range.hpp
 * @brief Excel 范围类定义
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include "tinakit/core/types.hpp"
#include <string>

namespace tinakit::excel {

/**
 * @class Range
 * @brief Excel 范围类
 * 
 * 代表 Excel 工作表中的一个单元格范围（如 A1:C10）。
 */
class Range {
public:
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

private:
    Position start_;
    Position end_;
};

} // namespace tinakit::excel