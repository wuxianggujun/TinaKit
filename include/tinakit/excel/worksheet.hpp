/**
 * @file worksheet.hpp
 * @brief Excel 工作表类定义
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include "tinakit/core/types.hpp"
#include "tinakit/core/async.hpp"
#include "tinakit/core/performance_optimizations.hpp"
#include "cell.hpp"
#include "row.hpp"
#include "column.hpp"
#include "range.hpp"
#include <filesystem>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <ranges>
#include <atomic>

namespace tinakit::internal {
class workbook_impl;
} // namespace tinakit::internal

namespace tinakit::excel {

// 前向声明
class StyleManager;
class ConditionalFormatBuilder;

/**
 * @class Worksheet
 * @brief Excel 工作表轻量级句柄类
 *
 * 这是一个轻量级的句柄类，本身不持有任何重型数据。
 * 所有实际的数据和操作都委托给 internal::workbook_impl。
 *
 * 核心设计原则：
 * 1. 轻量级：只包含 workbook_impl 指针和工作表名称
 * 2. 可复制：复制成本极低，安全共享同一个工作表
 * 3. 委托模式：所有操作都委托给 workbook_impl
 * 4. 惰性求值：数据按需加载
 *
 * @note 使用句柄-实现分离模式，提供稳定的 ABI 和优秀的性能
 */
class Worksheet {
public:
    /**
     * @brief 行范围类型，支持 std::ranges
     */
    class RowRange;

    /**
     * @brief 迭代器类型
     */
    class iterator;
    class const_iterator;

public:
    /**
     * @brief 构造函数（由 workbook 内部创建）
     * @param workbook_impl 工作簿实现的共享指针
     * @param sheet_id 工作表ID（性能优化）
     * @param sheet_name 工作表名称
     */
    Worksheet(std::shared_ptr<internal::workbook_impl> workbook_impl,
              std::uint32_t sheet_id,
              std::string sheet_name);

    /**
     * @brief 拷贝构造函数（轻量级，共享同一个实现）
     */
    Worksheet(const Worksheet& other) = default;

    /**
     * @brief 移动构造函数
     */
    Worksheet(Worksheet&& other) noexcept = default;

    /**
     * @brief 拷贝赋值运算符
     */
    Worksheet& operator=(const Worksheet& other) = default;

    /**
     * @brief 移动赋值运算符
     */
    Worksheet& operator=(Worksheet&& other) noexcept = default;

    /**
     * @brief 析构函数
     */
    ~Worksheet() = default;

public:
    /**
     * @brief 按地址获取单元格
     * @param address 单元格地址（如 "A1"）
     * @return 单元格引用
     */
    Cell& operator[](const std::string& address);
    
    /**
     * @brief 按地址获取单元格（只读）
     * @param address 单元格地址（如 "A1"）
     * @return 单元格常量引用
     */
    const Cell& operator[](const std::string& address) const;
    
    /**
     * @brief 获取单元格
     * @param address 单元格地址
     * @return 单元格引用
     */
    Cell& cell(const std::string& address);
    
    /**
     * @brief 获取单元格（只读）
     * @param address 单元格地址
     * @return 单元格常量引用
     */
    const Cell& cell(const std::string& address) const;
    
    /**
     * @brief 获取单元格
     * @param row 行号（1-based）
     * @param column 列号（1-based）
     * @return 单元格引用
     */
    Cell& cell(std::size_t row, std::size_t column);
    
    /**
     * @brief 获取单元格（只读）
     * @param row 行号（1-based）
     * @param column 列号（1-based）
     * @return 单元格常量引用
     */
    const Cell& cell(std::size_t row, std::size_t column) const;

public:
    /**
     * @brief 获取行
     * @param index 行号（1-based）
     * @return 行句柄
     */
    Row row(std::size_t index);

    /**
     * @brief 获取行（只读）
     * @param index 行号（1-based）
     * @return 行句柄
     */
    Row row(std::size_t index) const;

    /**
     * @brief 获取列
     * @param index 列号（1-based）
     * @return 列句柄
     */
    Column column(std::size_t index);

    /**
     * @brief 获取列（只读）
     * @param index 列号（1-based）
     * @return 列句柄
     */
    Column column(std::size_t index) const;

    /**
     * @brief 获取列
     * @param column_name 列名（如 "A", "B", "AA"）
     * @return 列句柄
     */
    Column column(const std::string& column_name);

    /**
     * @brief 获取列（只读）
     * @param column_name 列名（如 "A", "B", "AA"）
     * @return 列句柄
     */
    Column column(const std::string& column_name) const;
    
    /**
     * @brief 获取行范围
     * @param start_row 起始行号（1-based）
     * @param end_row 结束行号（1-based，包含）
     * @return 行范围对象
     */
    RowRange rows(std::size_t start_row, std::size_t end_row);
    
    /**
     * @brief 获取所有行
     * @return 行范围对象
     */
    RowRange rows();
    
    /**
     * @brief 获取工作表范围（支持样式操作）
     * @param range_str 范围字符串（如 "A1:C10"）
     * @return 范围对象
     */
    Range range(const std::string& range_str);

    /**
     * @brief 获取基础范围（仅位置信息）
     * @param range_str 范围字符串（如 "A1:C10"）
     * @return 基础范围对象
     */
    Range basic_range(const std::string& range_str);

public:
    /**
     * @brief 批量写入数据
     * @tparam T 数据类型
     * @param start_address 起始地址
     * @param data 数据容器
     */
    template<typename T>
    void write_data(const std::string& start_address, const std::vector<T>& data);
    
    /**
     * @brief 批量写入二维数据
     * @tparam T 数据类型
     * @param start_address 起始地址
     * @param data 二维数据
     */
    template<typename T>
    void write_data_2d(const std::string& start_address, const std::vector<std::vector<T>>& data);
    
    /**
     * @brief 批量写入元组数据
     * @tparam Tuple 元组类型
     * @param start_address 起始地址
     * @param data 元组数据容器
     */
    template<typename Tuple>
    void write_data_tuple(const std::string& start_address, const std::vector<Tuple>& data);

public:
    /**
     * @brief 异步处理行
     * @param processor 行处理函数
     * @return 处理任务
     */
    async::Task<void> process_rows_async(std::function<async::Task<void>(const Row&)> processor);
    
    /**
     * @brief 查找值
     * @param value 要查找的值
     * @return 包含该值的单元格地址列表
     */
    std::vector<std::string> find(const std::string& value) const;
    
    /**
     * @brief 替换值
     * @param old_value 旧值
     * @param new_value 新值
     * @return 替换的单元格数量
     */
    std::size_t replace(const std::string& old_value, const std::string& new_value);

public:
    /**
     * @brief 获取工作表名称
     * @return 工作表名称
     */
    const std::string& name() const noexcept;
    
    /**
     * @brief 设置工作表名称
     * @param name 新名称
     */
    void set_name(const std::string& name);
    
    /**
     * @brief 获取使用范围
     * @return 使用范围
     */
    Range used_range() const;
    
    /**
     * @brief 获取最大行数
     * @return 最大行数
     */
    std::size_t max_row() const noexcept;
    
    /**
     * @brief 获取最大列数
     * @return 最大列数
     */
    std::size_t max_column() const noexcept;

    /**
     * @brief 设置列宽
     * @param column_name 列名（如 "A", "B", "AA"）
     * @param width 列宽（字符数）
     */
    void set_column_width(const std::string& column_name, double width);

    /**
     * @brief 设置列宽
     * @param column_index 列索引（1-based）
     * @param width 列宽（字符数）
     */
    void set_column_width(std::size_t column_index, double width);

    /**
     * @brief 获取列宽
     * @param column_name 列名（如 "A", "B", "AA"）
     * @return 列宽（字符数）
     */
    double get_column_width(const std::string& column_name) const;

    /**
     * @brief 获取列宽
     * @param column_index 列索引（1-based）
     * @return 列宽（字符数）
     */
    double get_column_width(std::size_t column_index) const;

public:
    /**
     * @brief 合并单元格
     * @param range_str 要合并的范围（如 "A1:C3"）
     */
    void merge_cells(const std::string& range_str);

    /**
     * @brief 合并单元格
     * @param start_row 起始行（1-based）
     * @param start_col 起始列（1-based）
     * @param end_row 结束行（1-based）
     * @param end_col 结束列（1-based）
     */
    void merge_cells(std::size_t start_row, std::size_t start_col,
                     std::size_t end_row, std::size_t end_col);

    /**
     * @brief 取消合并单元格
     * @param range_str 要取消合并的范围（如 "A1:C3"）
     */
    void unmerge_cells(const std::string& range_str);

    /**
     * @brief 取消合并单元格
     * @param start_row 起始行（1-based）
     * @param start_col 起始列（1-based）
     * @param end_row 结束行（1-based）
     * @param end_col 结束列（1-based）
     */
    void unmerge_cells(std::size_t start_row, std::size_t start_col,
                       std::size_t end_row, std::size_t end_col);

public:
    /**
     * @brief 创建条件格式构建器
     * @param range_str 应用范围（如 "A1:A10"）
     * @return 条件格式构建器
     */
    ConditionalFormatBuilder conditional_format(const std::string& range_str);

    /**
     * @brief 添加条件格式（内部使用）
     * @param format 条件格式
     */
    void add_conditional_format(const ConditionalFormat& format);

    /**
     * @brief 获取所有条件格式（内部使用）
     * @return 条件格式列表
     */
    const std::vector<ConditionalFormat>& get_conditional_formats() const;

    /**
     * @brief 获取样式管理器（内部使用）
     * @return 样式管理器引用
     */
    StyleManager& style_manager();

    /**
     * @brief 检查是否为空
     * @return 如果工作表为空返回 true
     */
    bool empty() const noexcept;

    /**
     * @brief 获取缓存命中率
     * @return 缓存命中率（0.0-1.0）
     */
    double cache_hit_ratio() const noexcept {
        auto total = cache_hits_.load() + cache_misses_.load();
        return total > 0 ? static_cast<double>(cache_hits_.load()) / total : 0.0;
    }

    /**
     * @brief 批量设置单元格值（高性能API）
     */
    void batch_set_values(const std::vector<std::tuple<std::string, std::string>>& address_value_pairs);

    /**
     * @brief 批量设置单元格值（使用Position）
     */
    void batch_set_values(const std::vector<std::tuple<std::size_t, std::size_t, std::string>>& row_col_value_tuples);

    /**
     * @brief 清空缓存（用于测试和性能分析）
     */
    void clear_cache();

    /**
     * @brief 清空缓存
     */
    void clear_cache() const {
        fast_cell_cache_.clear();
        cache_hits_ = 0;
        cache_misses_ = 0;
    }

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

    /**
     * @brief 获取样式管理器（内部使用）
     * @return 样式管理器指针
     */
    StyleManager* get_style_manager() const;

    /**
     * @brief 获取合并单元格范围（内部使用）
     * @return 合并单元格范围列表
     */
    const std::vector<Range>& get_merged_ranges() const;

private:
    // 轻量级句柄：只包含工作簿实现指针和工作表ID
    std::shared_ptr<internal::workbook_impl> workbook_impl_;
    std::uint32_t sheet_id_;
    std::string sheet_name_;

    // 高性能Cell对象缓存，使用FastPosition和unordered_map
    mutable std::unordered_map<core::FastPosition, Cell, core::FastPositionHash> fast_cell_cache_;

    // 缓存统计
    mutable std::atomic<std::size_t> cache_hits_{0};
    mutable std::atomic<std::size_t> cache_misses_{0};

    friend class workbook;
};

/**
 * @class RowRange
 * @brief 行范围类，支持现代 C++ 范围操作
 */
class Worksheet::RowRange {
public:
    class iterator;

    /**
     * @brief 构造函数
     */
    RowRange(Worksheet& worksheet, std::size_t start_row, std::size_t end_row);

    /**
     * @brief 迭代器支持
     */
    iterator begin();
    iterator end();

    /**
     * @brief 获取范围大小
     */
    std::size_t size() const noexcept;

    /**
     * @brief 检查是否为空
     */
    bool empty() const noexcept;

    /**
     * @brief 过滤操作
     */
    template<typename Predicate>
    auto filter(Predicate&& pred) {
        return *this | std::views::filter(std::forward<Predicate>(pred));
    }

    /**
     * @brief 转换操作
     */
    template<typename Transform>
    auto transform(Transform&& trans) {
        return *this | std::views::transform(std::forward<Transform>(trans));
    }

private:
    Worksheet& worksheet_;
    std::size_t start_row_;
    std::size_t end_row_;
};

/**
 * @class RowRange::iterator
 * @brief RowRange的迭代器
 */
class Worksheet::RowRange::iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Row;
    using difference_type = std::ptrdiff_t;
    using pointer = Row;
    using reference = Row;

public:
    iterator() = default;
    iterator(Worksheet& worksheet, std::size_t row_index);

    reference operator*() const;
    pointer operator->() const;
    iterator& operator++();
    iterator operator++(int);

    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;

private:
    Worksheet* worksheet_ = nullptr;
    std::size_t row_index_ = 0;
};

} // namespace tinakit::excel

// 启用 std::ranges 支持
template<>
inline constexpr bool std::ranges::enable_borrowed_range<tinakit::excel::Worksheet::RowRange> = true;

/**
 * @example worksheet_usage.cpp
 * Worksheet 使用示例：
 * @code
 * #include <tinakit/excel/worksheet.hpp>
 * 
 * void worksheet_example() {
 *     using namespace tinakit::excel;
 *     
 *     auto workbook = Excel::open("data.xlsx");
 *     auto sheet = workbook["Sheet1"];
 *     
 *     // 单元格操作
 *     sheet["A1"].value("Hello");
 *     sheet.cell(1, 2).value("World");
 *     
 *     // 批量数据写入
 *     std::vector<std::tuple<std::string, int, double>> data = {
 *         {"Apple", 10, 1.5},
 *         {"Banana", 5, 0.8}
 *     };
 *     sheet.write_data("A2", data);
 *     
 *     // 现代 C++ 范围操作
 *     auto high_values = sheet.rows(2, 10)
 *         | std::views::filter([](const Row& row) {
 *             return row[2].as<double>() > 1.0;
 *         });
 * }
 * @endcode
 */
