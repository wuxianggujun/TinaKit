/**
 * @file worksheet.hpp
 * @brief Excel 工作表类定义
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include "tinakit/core/types.hpp"
#include "tinakit/core/async.hpp"
#include "cell.hpp"
#include "row.hpp"
#include "range.hpp"
#include <filesystem>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <ranges>

namespace tinakit::excel {

    class Cell;
    class Row;
    class Range;

/**
 * @class Worksheet
 * @brief Excel 工作表类
 * 
 * 代表 Excel 工作簿中的一个工作表，包含行、列和单元格的管理。
 * 支持现代 C++ 的迭代器和范围操作。
 * 
 * @note 使用 RAII 原则管理资源，对象析构时自动释放相关资源
 */
class Worksheet {
    // 前向声明
    class Impl;

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
     * @brief 创建新的工作表（工厂方法）
     * @param name 工作表名称
     * @return 工作表智能指针
     */
    static std::shared_ptr<Worksheet> create(const std::string& name);

    /**
     * @brief 构造函数（由 Workbook 内部创建）
     */
    explicit Worksheet(std::unique_ptr<Impl> impl);
    
    /**
     * @brief 析构函数
     */
    ~Worksheet();
    
    /**
     * @brief 移动构造函数
     */
    Worksheet(Worksheet&& other) noexcept;
    
    /**
     * @brief 移动赋值运算符
     */
    Worksheet& operator=(Worksheet&& other) noexcept;
    
    // 禁止拷贝
    Worksheet(const Worksheet&) = delete;
    Worksheet& operator=(const Worksheet&) = delete;

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
     * @return 行引用
     */
    Row& row(std::size_t index);
    
    /**
     * @brief 获取行（只读）
     * @param index 行号（1-based）
     * @return 行常量引用
     */
    const Row& row(std::size_t index) const;
    
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
     * @brief 获取范围
     * @param range_str 范围字符串（如 "A1:C10"）
     * @return 范围对象
     */
    Range range(const std::string& range_str);

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
     * @brief 检查是否为空
     * @return 如果工作表为空返回 true
     */
    bool empty() const noexcept;

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

private:
    std::unique_ptr<Impl> impl_;  ///< 实现细节（PIMPL 模式）
    friend class Workbook;
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
