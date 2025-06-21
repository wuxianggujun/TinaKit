/**
 * @file row.cpp
 * @brief Excel 行句柄类实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/excel/row.hpp"
#include "tinakit/excel/cell.hpp"
#include "tinakit/excel/types.hpp"
#include "tinakit/internal/workbook_impl.hpp"
#include "tinakit/internal/worksheet_impl.hpp"
#include "tinakit/core/exceptions.hpp"
#include <algorithm>

namespace tinakit::excel {

// ========================================
// Row 构造函数和基本方法
// ========================================

Row::Row(internal::workbook_impl* workbook_impl,
         std::uint32_t sheet_id,
         std::size_t row_index) noexcept
    : workbook_impl_(workbook_impl)
    , sheet_id_(sheet_id)
    , row_index_(row_index) {
}

// ========================================
// 单元格访问
// ========================================

Cell Row::operator[](const std::string& column_name) const {
    auto column_index = column_name_to_number(column_name);
    return (*this)[column_index];
}

Cell Row::operator[](std::size_t column_index) const {
    if (!valid()) {
        throw std::runtime_error("Invalid Row handle");
    }
    return Cell(workbook_impl_, sheet_id_, row_index_, column_index);
}

// ========================================
// 属性访问
// ========================================

std::size_t Row::index() const noexcept {
    return row_index_;
}

double Row::height() const noexcept {
    if (!valid()) {
        return 15.0; // 默认行高
    }

    try {
        auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);
        return worksheet_impl.get_row_height(row_index_);
    } catch (...) {
        return 15.0; // 出错时返回默认值
    }
}

void Row::set_height(double height) {
    if (!valid()) {
        throw std::runtime_error("Invalid Row handle");
    }

    auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
    auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);
    worksheet_impl.set_row_height(row_index_, height);
}

Row& Row::height(double height) {
    set_height(height);
    return *this;
}

bool Row::empty() const {
    if (!valid()) {
        return true;
    }

    try {
        // 检查这一行是否有任何非空单元格
        auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);

        // 获取工作表的最大列数
        auto max_col = worksheet_impl.max_column();
        if (max_col == 0) {
            return true;
        }

        // 检查这一行的所有单元格
        for (std::size_t col = 1; col <= max_col; ++col) {
            core::Coordinate pos(row_index_, col);
            if (worksheet_impl.has_cell_data(pos)) {
                auto cell_data = worksheet_impl.get_cell_data(pos);
                // 检查单元格是否有值
                if (std::holds_alternative<std::string>(cell_data.value)) {
                    if (!std::get<std::string>(cell_data.value).empty()) {
                        return false;
                    }
                } else {
                    return false; // 非字符串值认为非空
                }
            }
        }

        return true;
    } catch (...) {
        return true; // 出错时认为是空行
    }
}

std::size_t Row::size() const {
    if (!valid()) {
        return 0;
    }

    try {
        auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);

        // 找到这一行最后一个非空单元格的列索引
        std::size_t max_col = 0;
        auto worksheet_max_col = worksheet_impl.max_column();

        for (std::size_t col = 1; col <= worksheet_max_col; ++col) {
            core::Coordinate pos(row_index_, col);
            if (worksheet_impl.has_cell_data(pos)) {
                auto cell_data = worksheet_impl.get_cell_data(pos);
                // 检查单元格是否有值
                bool has_value = false;
                if (std::holds_alternative<std::string>(cell_data.value)) {
                    has_value = !std::get<std::string>(cell_data.value).empty();
                } else {
                    has_value = true; // 非字符串值认为有值
                }

                if (has_value) {
                    max_col = col;
                }
            }
        }

        return max_col;
    } catch (...) {
        return 0; // 出错时返回0
    }
}

bool Row::valid() const noexcept {
    return workbook_impl_ != nullptr && sheet_id_ != 0 && row_index_ != 0;
}

// ========================================
// 批量操作
// ========================================

void Row::set_values(const std::vector<Cell::CellValue>& values, std::size_t start_column) {
    if (!valid()) {
        throw std::runtime_error("Invalid Row handle");
    }

    for (std::size_t i = 0; i < values.size(); ++i) {
        auto cell = (*this)[start_column + i];
        std::visit([&cell](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                // 空值，清除单元格数据
                auto sheet_name = cell.workbook_impl_->get_sheet_name(cell.sheet_id_);
                auto& worksheet_impl = cell.workbook_impl_->get_worksheet_impl_public(sheet_name);
                core::Coordinate pos(cell.row_, cell.column_);
                worksheet_impl.clear_cell_data(pos);
            } else {
                cell.value(value);
            }
        }, values[i]);
    }
}

std::vector<Cell::CellValue> Row::get_values(std::size_t start_column, std::size_t count) const {
    if (!valid()) {
        throw std::runtime_error("Invalid Row handle");
    }

    std::size_t end_column;
    if (count == 0) {
        end_column = size();
    } else {
        end_column = start_column + count - 1;
    }

    std::vector<Cell::CellValue> values;
    values.reserve(end_column - start_column + 1);

    for (std::size_t col = start_column; col <= end_column; ++col) {
        auto cell = (*this)[col];
        values.push_back(cell.raw_value());
    }

    return values;
}

void Row::clear() {
    if (!valid()) {
        throw std::runtime_error("Invalid Row handle");
    }

    try {
        auto sheet_name = workbook_impl_->get_sheet_name(sheet_id_);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);

        // 收集这一行中所有有数据的单元格位置
        std::vector<core::Coordinate> cells_to_clear;
        auto max_col = worksheet_impl.max_column();

        for (std::size_t col = 1; col <= max_col; ++col) {
            core::Coordinate pos(row_index_, col);
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
// 迭代器支持
// ========================================

Row::iterator Row::begin() const {
    if (!valid()) {
        return iterator();
    }
    return iterator(*this, 1);
}

Row::iterator Row::end() const {
    if (!valid()) {
        return iterator();
    }
    return iterator(*this, size() + 1);
}

Row::const_iterator Row::cbegin() const {
    if (!valid()) {
        return const_iterator();
    }
    return const_iterator(*this, 1);
}

Row::const_iterator Row::cend() const {
    if (!valid()) {
        return const_iterator();
    }
    return const_iterator(*this, size() + 1);
}

// ========================================
// Row::iterator 实现
// ========================================

Row::iterator::iterator(const Row& row, std::size_t column)
    : row_(row), column_(column), max_column_(row.size()) {
}

Row::iterator::reference Row::iterator::operator*() const {
    return row_[column_];
}

Row::iterator::pointer Row::iterator::operator->() const {
    return row_[column_];
}

Row::iterator& Row::iterator::operator++() {
    ++column_;
    return *this;
}

Row::iterator Row::iterator::operator++(int) {
    auto temp = *this;
    ++column_;
    return temp;
}

bool Row::iterator::operator==(const iterator& other) const {
    return row_.workbook_impl_ == other.row_.workbook_impl_ &&
           row_.sheet_id_ == other.row_.sheet_id_ &&
           row_.row_index_ == other.row_.row_index_ &&
           column_ == other.column_;
}

bool Row::iterator::operator!=(const iterator& other) const {
    return !(*this == other);
}

// ========================================
// Row::const_iterator 实现
// ========================================

Row::const_iterator::const_iterator(const Row& row, std::size_t column)
    : row_(row), column_(column), max_column_(row.size()) {
}

Row::const_iterator::reference Row::const_iterator::operator*() const {
    return row_[column_];
}

Row::const_iterator::pointer Row::const_iterator::operator->() const {
    return row_[column_];
}

Row::const_iterator& Row::const_iterator::operator++() {
    ++column_;
    return *this;
}

Row::const_iterator Row::const_iterator::operator++(int) {
    auto temp = *this;
    ++column_;
    return temp;
}

bool Row::const_iterator::operator==(const const_iterator& other) const {
    return row_.workbook_impl_ == other.row_.workbook_impl_ &&
           row_.sheet_id_ == other.row_.sheet_id_ &&
           row_.row_index_ == other.row_.row_index_ &&
           column_ == other.column_;
}

bool Row::const_iterator::operator!=(const const_iterator& other) const {
    return !(*this == other);
}

} // namespace tinakit::excel
