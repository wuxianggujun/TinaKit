/**
 * @file worksheet.cpp
 * @brief Excel 工作表句柄类实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/cell.hpp"
#include "tinakit/excel/row.hpp"
#include "tinakit/excel/range.hpp"
#include "tinakit/internal/workbook_impl.hpp"
#include "tinakit/internal/worksheet_impl.hpp"
#include "tinakit/internal/coordinate_utils.hpp"
#include "tinakit/core/exceptions.hpp"
#include "tinakit/core/types.hpp"
#include "tinakit/excel/types.hpp"
#include "tinakit/excel/style_manager.hpp"
#include "tinakit/excel/conditional_format.hpp"
#include <algorithm>

namespace tinakit::excel {

// ========================================
// 构造函数和析构函数
// ========================================
Worksheet::Worksheet(std::shared_ptr<internal::workbook_impl> workbook_impl,
                     std::uint32_t sheet_id,
                     std::string sheet_name)
    : workbook_impl_(std::move(workbook_impl)), sheet_id_(sheet_id), sheet_name_(std::move(sheet_name)) {
}

// ========================================
// 基本属性
// ========================================

const std::string& Worksheet::name() const noexcept {
    return sheet_name_;
}

void Worksheet::set_name(const std::string& name) {
    workbook_impl_->rename_worksheet(sheet_name_, name);
    sheet_name_ = name;
}

std::size_t Worksheet::max_row() const noexcept {
    workbook_impl_->ensure_worksheet_loaded(sheet_name_);
    auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name_);
    return worksheet_impl.max_row();
}

std::size_t Worksheet::max_column() const noexcept {
    workbook_impl_->ensure_worksheet_loaded(sheet_name_);
    auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name_);
    return worksheet_impl.max_column();
}

bool Worksheet::empty() const noexcept {
    auto range = used_range();
    return range == Range(); // 比较是否为空范围
}

// ========================================
// 单元格访问（委托给 workbook_impl）
// ========================================

Cell& Worksheet::operator[](const std::string& address) {
    auto pos = Position::from_address(address);
    return cell(pos.row, pos.column);
}

const Cell& Worksheet::operator[](const std::string& address) const {
    auto pos = Position::from_address(address);
    return cell(pos.row, pos.column);
}

Cell& Worksheet::cell(const std::string& address) {
    auto pos = Position::from_address(address);
    return cell(pos.row, pos.column);
}

const Cell& Worksheet::cell(const std::string& address) const {
    auto pos = Position::from_address(address);
    return cell(pos.row, pos.column);
}

Cell& Worksheet::cell(std::size_t row, std::size_t column) {
    // 性能优化：使用FastPosition和unordered_map
    core::FastPosition pos(static_cast<std::uint32_t>(row), static_cast<std::uint32_t>(column));

    // 使用unordered_map进行O(1)查找
    auto it = fast_cell_cache_.find(pos);
    if (it != fast_cell_cache_.end()) {
        cache_hits_++;
        return it->second;
    }

    cache_misses_++;
    // 创建新的Cell对象并缓存
    auto [inserted_it, success] = fast_cell_cache_.emplace(
        pos, Cell(workbook_impl_.get(), sheet_id_, row, column)
    );
    return inserted_it->second;
}

const Cell& Worksheet::cell(std::size_t row, std::size_t column) const {
    // 性能优化：使用FastPosition和unordered_map
    core::FastPosition pos(static_cast<std::uint32_t>(row), static_cast<std::uint32_t>(column));

    auto it = fast_cell_cache_.find(pos);
    if (it != fast_cell_cache_.end()) {
        cache_hits_++;
        return it->second;
    }

    cache_misses_++;
    // 创建新的Cell对象并缓存
    auto [inserted_it, success] = fast_cell_cache_.emplace(
        pos, Cell(workbook_impl_.get(), sheet_id_, row, column)
    );
    return inserted_it->second;
}

// ========================================
// 行访问（委托给 workbook_impl）
// ========================================

Row Worksheet::row(std::size_t index) {
    return Row(workbook_impl_.get(), sheet_id_, index);
}

Row Worksheet::row(std::size_t index) const {
    return Row(workbook_impl_.get(), sheet_id_, index);
}

// ========================================
// 查找和替换（委托给 workbook_impl）
// ========================================

std::vector<std::string> Worksheet::find(const std::string& value) const {
    if (!workbook_impl_) {
        return {};
    }

    std::vector<std::string> results;
    auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name_);

    // 遍历所有单元格查找匹配的值
    auto max_row = worksheet_impl.max_row();
    auto max_col = worksheet_impl.max_column();

    for (std::size_t row = 1; row <= max_row; ++row) {
        for (std::size_t col = 1; col <= max_col; ++col) {
            core::Coordinate pos(row, col);
            if (worksheet_impl.has_cell_data(pos)) {
                auto cell_data = worksheet_impl.get_cell_data(pos);

                // 检查字符串值
                if (std::holds_alternative<std::string>(cell_data.value)) {
                    const auto& cell_value = std::get<std::string>(cell_data.value);
                    if (cell_value.find(value) != std::string::npos) {
                        // 添加单元格地址
                        auto address = internal::utils::CoordinateUtils::coordinate_to_string(pos);
                        results.push_back(address);
                    }
                }
            }
        }
    }

    return results;
}

std::size_t Worksheet::replace(const std::string& old_value, const std::string& new_value) {
    if (!workbook_impl_) {
        return 0;
    }

    std::size_t replace_count = 0;
    auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name_);

    // 遍历所有单元格进行替换
    auto max_row = worksheet_impl.max_row();
    auto max_col = worksheet_impl.max_column();

    for (std::size_t row = 1; row <= max_row; ++row) {
        for (std::size_t col = 1; col <= max_col; ++col) {
            core::Coordinate pos(row, col);
            if (worksheet_impl.has_cell_data(pos)) {
                auto cell_data = worksheet_impl.get_cell_data(pos);

                // 检查字符串值并替换
                if (std::holds_alternative<std::string>(cell_data.value)) {
                    auto& cell_value = std::get<std::string>(cell_data.value);
                    std::size_t pos_found = 0;
                    bool replaced = false;

                    while ((pos_found = cell_value.find(old_value, pos_found)) != std::string::npos) {
                        cell_value.replace(pos_found, old_value.length(), new_value);
                        pos_found += new_value.length();
                        replaced = true;
                    }

                    if (replaced) {
                        // 更新单元格数据
                        cell_data.value = cell_value;
                        worksheet_impl.set_cell_data(pos, cell_data);
                        replace_count++;
                    }
                }
            }
        }
    }

    return replace_count;
}

// ========================================
// 范围操作（委托给 workbook_impl）
// ========================================

Range Worksheet::used_range() const {
    return workbook_impl_->get_used_range(sheet_name_);
}

Worksheet::RowRange Worksheet::rows(std::size_t start_row, std::size_t end_row) {
    return RowRange(*this, start_row, end_row);
}

Worksheet::RowRange Worksheet::rows() {
    return RowRange(*this, 1, max_row());
}

Range Worksheet::range(const std::string& range_str) {
    auto range_addr = internal::utils::CoordinateUtils::string_to_range_address(range_str);
    return Range(workbook_impl_, sheet_name_, range_addr);
}

Range Worksheet::basic_range(const std::string& range_str) {
    if (!workbook_impl_) {
        throw std::runtime_error("Invalid worksheet handle");
    }

    auto range_addr = internal::utils::CoordinateUtils::string_to_range_address(range_str);
    return Range(workbook_impl_, sheet_name_, range_addr);
}

// ========================================
// 异步处理（委托给 workbook_impl）
// ========================================

async::Task<void> Worksheet::process_rows_async(std::function<async::Task<void>(const Row&)> processor) {
    // 获取使用范围
    auto range = used_range();
    if (range == Range()) { // 检查是否为空范围
        co_return;
    }

    // 逐行处理
    for (std::size_t row_num = range.start_position().row; row_num <= range.end_position().row; ++row_num) {
        auto row = this->row(row_num);
        co_await processor(row);
    }
}

// ========================================
// 样式管理（委托给 workbook_impl）
// ========================================

StyleManager* Worksheet::get_style_manager() const {
    return &workbook_impl_->style_manager();
}

StyleManager& Worksheet::style_manager() {
    return workbook_impl_->style_manager();
}

// ========================================
// 列宽管理（委托给 workbook_impl）
// ========================================

void Worksheet::set_column_width(const std::string& column_name, double width) {
    std::size_t column_index = column_name_to_number(column_name);
    set_column_width(column_index, width);
}

void Worksheet::set_column_width(std::size_t column_index, double width) {
    auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name_);
    worksheet_impl.set_column_width(column_index, width);
}

double Worksheet::get_column_width(const std::string& column_name) const {
    std::size_t column_index = column_name_to_number(column_name);
    return get_column_width(column_index);
}

double Worksheet::get_column_width(std::size_t column_index) const {
    auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name_);
    return worksheet_impl.get_column_width(column_index);
}

// ========================================
// 合并单元格（委托给 workbook_impl）
// ========================================

void Worksheet::merge_cells(const std::string& range_str) {
    if (!workbook_impl_) {
        throw std::runtime_error("Invalid worksheet handle");
    }

    auto range_addr = internal::utils::CoordinateUtils::string_to_range_address(range_str);
    merge_cells(range_addr.start.row, range_addr.start.column,
                range_addr.end.row, range_addr.end.column);
}

void Worksheet::merge_cells(std::size_t start_row, std::size_t start_col,
                           std::size_t end_row, std::size_t end_col) {
    if (!workbook_impl_) {
        throw std::runtime_error("Invalid worksheet handle");
    }

    // 验证范围有效性
    if (start_row > end_row || start_col > end_col) {
        throw std::invalid_argument("Invalid merge range: start position must be before end position");
    }

    if (start_row == end_row && start_col == end_col) {
        // 单个单元格不需要合并
        return;
    }

    auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name_);

    // 创建合并范围
    core::range_address merge_range{
        core::Coordinate(start_row, start_col),
        core::Coordinate(end_row, end_col)
    };

    // 添加到合并范围列表
    // TODO: 实现worksheet_impl的add_merged_range方法
    // worksheet_impl.add_merged_range(merge_range);
    (void)merge_range; // 暂时避免未使用变量警告
}

void Worksheet::unmerge_cells(const std::string& range_str) {
    if (!workbook_impl_) {
        throw std::runtime_error("Invalid worksheet handle");
    }

    auto range_addr = internal::utils::CoordinateUtils::string_to_range_address(range_str);
    unmerge_cells(range_addr.start.row, range_addr.start.column,
                  range_addr.end.row, range_addr.end.column);
}

void Worksheet::unmerge_cells(std::size_t start_row, std::size_t start_col,
                             std::size_t end_row, std::size_t end_col) {
    if (!workbook_impl_) {
        throw std::runtime_error("Invalid worksheet handle");
    }

    auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name_);

    // 创建要移除的合并范围
    core::range_address unmerge_range{
        core::Coordinate(start_row, start_col),
        core::Coordinate(end_row, end_col)
    };

    // 从合并范围列表中移除
    // TODO: 实现worksheet_impl的remove_merged_range方法
    // worksheet_impl.remove_merged_range(unmerge_range);
    (void)unmerge_range; // 暂时避免未使用变量警告
}

const std::vector<Range>& Worksheet::get_merged_ranges() const {
    if (!workbook_impl_) {
        static std::vector<Range> empty_ranges;
        return empty_ranges;
    }

    // 这里需要从worksheet_impl获取合并范围并转换为Range对象
    // 由于这个功能比较复杂，暂时返回空列表
    // TODO: 实现从worksheet_impl获取合并范围的功能
    static std::vector<Range> empty_ranges;
    return empty_ranges;
}

// ========================================
// 条件格式（委托给 workbook_impl）
// ========================================

ConditionalFormatBuilder Worksheet::conditional_format(const std::string& range_str) {
    return ConditionalFormatBuilder(*this, range_str);
}

void Worksheet::add_conditional_format(const ConditionalFormat& format) {
    workbook_impl_->add_conditional_format(sheet_name_, format);
}

const std::vector<ConditionalFormat>& Worksheet::get_conditional_formats() const {
    if (!workbook_impl_) {
        static std::vector<ConditionalFormat> empty_formats;
        return empty_formats;
    }

    // TODO: 实现workbook_impl的get_conditional_formats方法
    // return workbook_impl_->get_conditional_formats(sheet_name_);
    static std::vector<ConditionalFormat> empty_formats;
    return empty_formats;
}

// ========================================
// RowRange 实现
// ========================================

Worksheet::RowRange::RowRange(Worksheet& worksheet, std::size_t start_row, std::size_t end_row)
    : worksheet_(worksheet), start_row_(start_row), end_row_(end_row) {
}

Worksheet::RowRange::iterator Worksheet::RowRange::begin() {
    return iterator(worksheet_, start_row_);
}

Worksheet::RowRange::iterator Worksheet::RowRange::end() {
    return iterator(worksheet_, end_row_ + 1);
}

std::size_t Worksheet::RowRange::size() const noexcept {
    return (end_row_ >= start_row_) ? (end_row_ - start_row_ + 1) : 0;
}

bool Worksheet::RowRange::empty() const noexcept {
    return start_row_ > end_row_;
}

// ========================================
// RowRange::iterator 实现
// ========================================

Worksheet::RowRange::iterator::iterator(Worksheet& worksheet, std::size_t row_index)
    : worksheet_(&worksheet), row_index_(row_index) {
}

Worksheet::RowRange::iterator::reference Worksheet::RowRange::iterator::operator*() const {
    return worksheet_->row(row_index_);
}

Worksheet::RowRange::iterator::pointer Worksheet::RowRange::iterator::operator->() const {
    return worksheet_->row(row_index_);
}

Worksheet::RowRange::iterator& Worksheet::RowRange::iterator::operator++() {
    ++row_index_;
    return *this;
}

Worksheet::RowRange::iterator Worksheet::RowRange::iterator::operator++(int) {
    auto temp = *this;
    ++row_index_;
    return temp;
}

bool Worksheet::RowRange::iterator::operator==(const iterator& other) const {
    return worksheet_ == other.worksheet_ && row_index_ == other.row_index_;
}

bool Worksheet::RowRange::iterator::operator!=(const iterator& other) const {
    return !(*this == other);
}

// ========================================
// 性能优化方法
// ========================================

void Worksheet::batch_set_values(const std::vector<std::tuple<std::string, std::string>>& address_value_pairs) {
    // 转换为Coordinate和值的批量操作
    std::vector<std::tuple<core::Coordinate, internal::cell_data::CellValue>> operations;
    operations.reserve(address_value_pairs.size());

    for (const auto& [address, value] : address_value_pairs) {
        auto pos = Position::from_address(address);
        core::Coordinate core_pos(pos.row, pos.column);
        operations.emplace_back(core_pos, internal::cell_data::CellValue(value));
    }

    // 委托给workbook_impl的高性能批量操作
    workbook_impl_->batch_set_cell_values(sheet_name_, operations);
}

void Worksheet::batch_set_values(const std::vector<std::tuple<std::size_t, std::size_t, std::string>>& row_col_value_tuples) {
    // 转换为Coordinate和值的批量操作
    std::vector<std::tuple<core::Coordinate, internal::cell_data::CellValue>> operations;
    operations.reserve(row_col_value_tuples.size());

    for (const auto& [row, col, value] : row_col_value_tuples) {
        core::Coordinate pos(row, col);
        operations.emplace_back(pos, internal::cell_data::CellValue(value));
    }

    // 委托给workbook_impl的高性能批量操作
    workbook_impl_->batch_set_cell_values(sheet_name_, operations);
}

void Worksheet::clear_cache() {
    fast_cell_cache_.clear();
    cache_hits_.store(0);
    cache_misses_.store(0);
}

} // namespace tinakit::excel
