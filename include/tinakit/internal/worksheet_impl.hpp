/**
 * @file worksheet_impl.hpp
 * @brief 工作表内部实现类定义
 * @author TinaKit Team
 * @date 2025-6-20
 */

#pragma once

#include "tinakit/core/types.hpp"
#include "tinakit/core/xml_parser.hpp"
#include "workbook_impl.hpp"
#include <map>
#include <string>
#include <memory>
#include <optional>

namespace tinakit::core {
class OpenXmlArchiver;
} // namespace tinakit::core

namespace tinakit::internal {

/**
 * @enum LoadState
 * @brief 工作表加载状态
 */
enum class LoadState {
    NotLoaded,      ///< 未加载
    PartialLoaded,  ///< 部分加载
    FullyLoaded     ///< 完全加载
};

/**
 * @class worksheet_impl
 * @brief 工作表的内部实现类
 * 
 * 负责管理单个工作表的数据，支持惰性加载和流式处理。
 * 
 * 核心职责：
 * 1. 管理单元格数据的存储和访问
 * 2. 实现惰性加载机制
 * 3. 维护工作表的元数据（名称、尺寸等）
 * 4. 处理工作表级别的操作（合并单元格、条件格式等）
 */
class worksheet_impl {
public:
    /**
     * @brief 构造函数
     * @param name 工作表名称
     * @param workbook_impl 父工作簿实现的引用
     */
    worksheet_impl(const std::string& name, workbook_impl& workbook);
    
    /**
     * @brief 析构函数
     */
    ~worksheet_impl();
    
    // 禁用拷贝和移动
    worksheet_impl(const worksheet_impl&) = delete;
    worksheet_impl& operator=(const worksheet_impl&) = delete;
    worksheet_impl(worksheet_impl&&) = delete;
    worksheet_impl& operator=(worksheet_impl&&) = delete;
    
    // ========================================
    // 基本属性
    // ========================================
    
    /**
     * @brief 获取工作表名称
     */
    const std::string& name() const;
    
    /**
     * @brief 设置工作表名称
     */
    void set_name(const std::string& new_name);
    
    /**
     * @brief 获取最大行数
     */
    std::size_t max_row() const;
    
    /**
     * @brief 获取最大列数
     */
    std::size_t max_column() const;

    /**
     * @brief 获取使用范围
     */
    excel::Range get_used_range() const;

    /**
     * @brief 获取加载状态
     */
    LoadState load_state() const;
    
    // ========================================
    // 单元格数据访问
    // ========================================
    
    /**
     * @brief 获取单元格数据
     */
    cell_data get_cell_data(const core::Coordinate& pos);

    /**
     * @brief 获取单元格数据（const版本）
     */
    cell_data get_cell_data(const core::Coordinate& pos) const;

    /**
     * @brief 设置单元格数据
     */
    void set_cell_data(const core::Coordinate& pos, const cell_data& data);

    /**
     * @brief 检查单元格是否存在数据
     */
    bool has_cell_data(const core::Coordinate& pos) const;

    /**
     * @brief 删除单元格数据
     */
    void remove_cell_data(const core::Coordinate& pos);

    /**
     * @brief 获取范围内的所有单元格数据
     */
    std::map<core::Coordinate, cell_data> get_range_data(const core::range_address& range);
    
    /**
     * @brief 批量设置范围数据
     */
    void set_range_data(const core::range_address& range, 
                       const std::vector<std::vector<cell_data::CellValue>>& values);
    
    // ========================================
    // 惰性加载管理
    // ========================================
    
    /**
     * @brief 确保指定位置的数据已加载
     */
    void ensure_loaded(const core::Coordinate& pos);
    
    /**
     * @brief 确保指定范围的数据已加载
     */
    void ensure_range_loaded(const core::range_address& range);
    
    /**
     * @brief 强制加载所有数据
     */
    void load_all();
    
    /**
     * @brief 卸载数据以释放内存
     */
    void unload();
    
    // ========================================
    // 工作表级别操作
    // ========================================
    
    /**
     * @brief 插入行
     */
    void insert_rows(std::size_t row, std::size_t count);
    
    /**
     * @brief 删除行
     */
    void delete_rows(std::size_t row, std::size_t count);
    
    /**
     * @brief 插入列
     */
    void insert_columns(std::size_t column, std::size_t count);
    
    /**
     * @brief 删除列
     */
    void delete_columns(std::size_t column, std::size_t count);
    
    /**
     * @brief 设置列宽
     */
    void set_column_width(std::size_t column, double width);
    
    /**
     * @brief 获取列宽
     */
    double get_column_width(std::size_t column) const;
    
    /**
     * @brief 设置行高
     */
    void set_row_height(std::size_t row, double height);
    
    /**
     * @brief 获取行高
     */
    double get_row_height(std::size_t row) const;
    
    // ========================================
    // 内部状态管理
    // ========================================
    
    /**
     * @brief 标记为已修改
     */
    void mark_dirty();
    
    /**
     * @brief 检查是否已修改
     */
    bool is_dirty() const;
    
    /**
     * @brief 清除修改标志
     */
    void clear_dirty();

    /**
     * @brief 保存到归档器
     */
    void save_to_archiver(core::OpenXmlArchiver& archiver);

    /**
     * @brief 获取单元格数量
     */
    std::size_t cell_count() const;

    // ========================================
    // 条件格式
    // ========================================

    /**
     * @brief 添加条件格式
     * @param format 条件格式
     */
    void add_conditional_format(const excel::ConditionalFormat& format);

    /**
     * @brief 获取所有条件格式
     * @return 条件格式列表
     */
    const std::vector<excel::ConditionalFormat>& get_conditional_formats() const;

    // ========================================
    // 合并单元格
    // ========================================

    /**
     * @brief 添加合并单元格范围
     * @param range 要合并的范围
     */
    void add_merged_range(const core::range_address& range);

    /**
     * @brief 移除合并单元格范围
     * @param range 要移除的范围
     */
    void remove_merged_range(const core::range_address& range);

    /**
     * @brief 获取所有合并单元格范围
     * @return 合并单元格范围列表
     */
    const std::vector<core::range_address>& get_merged_ranges() const;

    /**
     * @brief 检查指定范围是否已合并
     * @param range 要检查的范围
     * @return true 如果范围已合并
     */
    bool is_merged_range(const core::range_address& range) const;

    /**
     * @brief 应用条件格式到单元格
     * @param pos 单元格位置
     * @return 应用的样式ID（如果有）
     */
    std::optional<std::uint32_t> apply_conditional_format(const core::Coordinate& pos);

private:
    /**
     * @brief 判断字符串是否应该使用内联存储
     * @param str 字符串
     * @return true 如果应该使用内联存储
     */
    bool should_use_inline_string(const std::string& str) const;
    // 基本属性
    std::string name_;
    workbook_impl& workbook_;
    
    // 加载状态
    LoadState load_state_ = LoadState::NotLoaded;
    bool is_dirty_ = false;
    
    // 单元格数据存储
    std::map<core::Coordinate, cell_data> cells_;
    
    // 工作表尺寸
    std::size_t max_row_ = 0;
    std::size_t max_column_ = 0;
    
    // 列宽和行高
    std::map<std::size_t, double> column_widths_;
    std::map<std::size_t, double> row_heights_;
    
    // 合并单元格范围
    std::vector<core::range_address> merged_ranges_;

    // 条件格式
    std::vector<excel::ConditionalFormat> conditional_formats_;
    
    // 内部方法
    void load_from_xml();
    void update_dimensions(const core::Coordinate& pos);
    void parse_cell_data(const std::string& xml_content);
    std::string generate_worksheet_xml();

    // 优化的解析方法
    void parse_single_cell(core::XmlParser::iterator& it, core::XmlParser& parser);
    void parse_conditional_formatting(core::XmlParser::iterator& it, core::XmlParser& parser);
    void parse_merged_cells(core::XmlParser::iterator& it, core::XmlParser& parser);

    // 辅助方法
    bool ranges_overlap(const core::range_address& range1, const core::range_address& range2) const;

    // 条件格式辅助方法
    bool is_cell_in_range(const core::Coordinate& pos, const std::string& range_str) const;
    bool evaluate_conditional_rule(const core::Coordinate& pos, const excel::ConditionalFormatRule& rule) const;
    double get_cell_value_for_condition(const core::Coordinate& pos) const;
    std::string get_cell_text_for_condition(const core::Coordinate& pos) const;
    bool evaluate_cell_value_condition(double cell_value, const excel::ConditionalFormatRule& rule) const;
    bool evaluate_text_condition(double cell_value, const excel::ConditionalFormatRule& rule) const;
    bool evaluate_expression_condition(const core::Coordinate& pos, const excel::ConditionalFormatRule& rule) const;
    bool evaluate_duplicate_values_condition(const core::Coordinate& pos, const excel::ConditionalFormatRule& rule) const;
    bool evaluate_unique_values_condition(const core::Coordinate& pos, const excel::ConditionalFormatRule& rule) const;
    std::string conditional_format_type_to_string(excel::ConditionalFormatType type) const;
    std::string conditional_format_operator_to_string(excel::ConditionalFormatOperator op) const;
    excel::ConditionalFormatType string_to_conditional_format_type(const std::string& str) const;
    excel::ConditionalFormatOperator string_to_conditional_format_operator(const std::string& str) const;
};

} // namespace tinakit::internal
