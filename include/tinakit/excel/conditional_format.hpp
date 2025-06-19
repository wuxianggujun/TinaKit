/**
 * @file conditional_format.hpp
 * @brief 条件格式构建器类定义
 * @author TinaKit Team
 * @date 2025-6-19
 */

#pragma once

#include "types.hpp"
#include <string>
#include <memory>

namespace tinakit::excel {

// 前向声明
class Worksheet;
class Range;

/**
 * @class ConditionalFormatBuilder
 * @brief 条件格式构建器类
 * 
 * 提供链式调用的API来构建条件格式规则
 */
class ConditionalFormatBuilder {
public:
    /**
     * @brief 构造函数
     * @param worksheet 工作表引用
     * @param range_str 应用范围
     */
    ConditionalFormatBuilder(Worksheet& worksheet, const std::string& range_str);
    
    /**
     * @brief 析构函数
     */
    ~ConditionalFormatBuilder();
    
    // 数值比较条件
    ConditionalFormatBuilder& when_greater_than(double value);
    ConditionalFormatBuilder& when_greater_than_or_equal(double value);
    ConditionalFormatBuilder& when_less_than(double value);
    ConditionalFormatBuilder& when_less_than_or_equal(double value);
    ConditionalFormatBuilder& when_equal(double value);
    ConditionalFormatBuilder& when_not_equal(double value);
    ConditionalFormatBuilder& when_between(double min_value, double max_value);
    ConditionalFormatBuilder& when_not_between(double min_value, double max_value);
    
    // 文本条件
    ConditionalFormatBuilder& when_contains(const std::string& text);
    ConditionalFormatBuilder& when_not_contains(const std::string& text);
    ConditionalFormatBuilder& when_begins_with(const std::string& text);
    ConditionalFormatBuilder& when_ends_with(const std::string& text);
    
    // 特殊条件
    ConditionalFormatBuilder& when_duplicate_values();
    ConditionalFormatBuilder& when_unique_values();
    
    // 格式设置
    ConditionalFormatBuilder& background_color(const Color& color);
    ConditionalFormatBuilder& font_color(const Color& color);
    ConditionalFormatBuilder& font(const std::string& font_name, double size = 11.0);
    ConditionalFormatBuilder& bold(bool bold = true);
    ConditionalFormatBuilder& italic(bool italic = true);
    
    // 应用条件格式
    void apply();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @class ConditionalFormatManager
 * @brief 条件格式管理器
 * 
 * 管理工作表中的所有条件格式
 */
class ConditionalFormatManager {
public:
    /**
     * @brief 构造函数
     */
    ConditionalFormatManager();
    
    /**
     * @brief 析构函数
     */
    ~ConditionalFormatManager();
    
    /**
     * @brief 添加条件格式
     * @param format 条件格式
     */
    void add_conditional_format(const ConditionalFormat& format);
    
    /**
     * @brief 移除条件格式
     * @param range_str 范围字符串
     */
    void remove_conditional_format(const std::string& range_str);
    
    /**
     * @brief 获取所有条件格式
     * @return 条件格式列表
     */
    const std::vector<ConditionalFormat>& get_conditional_formats() const;
    
    /**
     * @brief 清除所有条件格式
     */
    void clear();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace tinakit::excel
