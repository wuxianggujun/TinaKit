/**
 * @file column.cpp
 * @brief Excel 列操作实现
 * @author TinaKit Team
 * @date 2025-6-21
 */

#include "tinakit/excel/column.hpp"
#include "tinakit/internal/workbook_impl.hpp"
#include "tinakit/internal/worksheet_impl.hpp"
#include <stdexcept>
#include <algorithm>

namespace tinakit::excel {

// ========================================
// 构造和基础操作
// ========================================

Column::Column(std::shared_ptr<tinakit::internal::workbook_impl> workbook_impl,
               std::uint32_t sheet_id,
               std::size_t column_index)
    : workbook_impl_(std::move(workbook_impl))
    , sheet_id_(sheet_id)
    , column_index_(column_index) {
}

// ========================================
// 单元格访问
// ========================================

Cell Column::operator[](std::size_t row_index) const {
    return Cell(workbook_impl_.get(), sheet_id_, row_index, column_index_);
}

Cell Column::cell(std::size_t row_index) const {
    return (*this)[row_index];
}

// ========================================
// 列属性
// ========================================

std::size_t Column::index() const noexcept {
    return column_index_;
}

double Column::width() const noexcept {
    if (!valid()) {
        return 0.0;
    }

    try {
        auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);
        return worksheet_impl.get_column_width(column_index_);
    } catch (...) {
        return 0.0;
    }
}

void Column::set_width(double width) {
    if (!valid()) {
        throw std::runtime_error("Invalid Column handle");
    }

    try {
        auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);
        worksheet_impl.set_column_width(column_index_, width);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to set column width: " + std::string(e.what()));
    }
}

Column& Column::width(double width) {
    set_width(width);
    return *this;
}

bool Column::hidden() const noexcept {
    if (!valid()) {
        return false;
    }

    try {
        auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);
        return worksheet_impl.is_column_hidden(column_index_);
    } catch (...) {
        return false;
    }
}

void Column::set_hidden(bool hidden) {
    if (!valid()) {
        throw std::runtime_error("Invalid Column handle");
    }

    try {
        auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);
        worksheet_impl.set_column_hidden(column_index_, hidden);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to set column hidden state: " + std::string(e.what()));
    }
}

Column& Column::hidden(bool hidden) {
    set_hidden(hidden);
    return *this;
}

// ========================================
// 状态查询
// ========================================

bool Column::empty() const {
    if (!valid()) {
        return true;
    }

    try {
        auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);

        // 检查这一列中是否有任何数据
        auto max_row = worksheet_impl.max_row();
        for (std::size_t row = 1; row <= max_row; ++row) {
            core::Coordinate pos(row, column_index_);
            if (worksheet_impl.has_cell_data(pos)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return true;
    }
}

std::size_t Column::size() const {
    if (!valid()) {
        return 0;
    }

    try {
        auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);

        // 找到这一列中最大的行索引
        std::size_t max_row_with_data = 0;
        auto max_row = worksheet_impl.max_row();
        
        for (std::size_t row = 1; row <= max_row; ++row) {
            core::Coordinate pos(row, column_index_);
            if (worksheet_impl.has_cell_data(pos)) {
                max_row_with_data = row;
            }
        }
        
        return max_row_with_data;
    } catch (...) {
        return 0;
    }
}

bool Column::valid() const noexcept {
    return workbook_impl_ != nullptr && sheet_id_ != 0 && column_index_ != 0;
}

// ========================================
// 批量操作
// ========================================

void Column::set_values(const std::vector<Cell::CellValue>& values, std::size_t start_row) {
    if (!valid()) {
        throw std::runtime_error("Invalid Column handle");
    }

    for (std::size_t i = 0; i < values.size(); ++i) {
        auto cell = (*this)[start_row + i];
        std::visit([&cell, this](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                // 空值，清除单元格数据
                auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
                auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);
                core::Coordinate pos(cell.row(), cell.column());
                worksheet_impl.clear_cell_data(pos);
            } else {
                cell.value(value);
            }
        }, values[i]);
    }
}

std::vector<Cell::CellValue> Column::get_values(std::size_t start_row, std::size_t count) const {
    if (!valid()) {
        throw std::runtime_error("Invalid Column handle");
    }

    std::size_t end_row;
    if (count == 0) {
        end_row = size();
    } else {
        end_row = start_row + count - 1;
    }

    std::vector<Cell::CellValue> values;
    values.reserve(end_row - start_row + 1);

    for (std::size_t row = start_row; row <= end_row; ++row) {
        auto cell = (*this)[row];
        values.push_back(cell.raw_value());
    }

    return values;
}

void Column::clear() {
    if (!valid()) {
        throw std::runtime_error("Invalid Column handle");
    }

    try {
        auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);

        // 收集这一列中所有有数据的单元格位置
        std::vector<core::Coordinate> cells_to_clear;
        auto max_row = worksheet_impl.max_row();
        
        for (std::size_t row = 1; row <= max_row; ++row) {
            core::Coordinate pos(row, column_index_);
            if (worksheet_impl.has_cell_data(pos)) {
                cells_to_clear.push_back(pos);
            }
        }

        // 清除所有找到的单元格
        for (const auto& pos : cells_to_clear) {
            worksheet_impl.clear_cell_data(pos);
        }
    } catch (...) {
        // 忽略错误
    }
}

// ========================================
// 迭代器实现
// ========================================

Column::iterator::iterator(const Column& column, std::size_t row_index)
    : column_(&column), row_index_(row_index) {
}

Cell Column::iterator::operator*() const {
    return (*column_)[row_index_];
}

Column::iterator& Column::iterator::operator++() {
    ++row_index_;
    return *this;
}

Column::iterator Column::iterator::operator++(int) {
    iterator temp = *this;
    ++row_index_;
    return temp;
}

bool Column::iterator::operator==(const iterator& other) const {
    return column_ == other.column_ && row_index_ == other.row_index_;
}

bool Column::iterator::operator!=(const iterator& other) const {
    return !(*this == other);
}

Column::iterator Column::begin() const {
    return iterator(*this, 1);
}

Column::iterator Column::end() const {
    return iterator(*this, size() + 1);
}

Column::const_iterator Column::cbegin() const {
    return begin();
}

Column::const_iterator Column::cend() const {
    return end();
}

} // namespace tinakit::excel
