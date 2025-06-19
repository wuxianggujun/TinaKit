/**
 * @file worksheet_range.cpp
 * @brief Excel 工作表范围操作类实现
 * @author TinaKit Team
 * @date 2025-01-19
 */

#include "tinakit/excel/worksheet_range.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/cell.hpp"
#include "tinakit/excel/style_template.hpp"
#include <stdexcept>

namespace tinakit::excel {

// ========================================
// WorksheetRange::Impl 实现
// ========================================

class WorksheetRange::Impl {
public:
    Worksheet& worksheet_;
    Range range_;
    
    Impl(Worksheet& worksheet, const Range& range)
        : worksheet_(worksheet), range_(range) {
    }
    
    Impl(const Impl& other)
        : worksheet_(other.worksheet_), range_(other.range_) {
    }
};

// ========================================
// WorksheetRange 实现
// ========================================

WorksheetRange::WorksheetRange(Worksheet& worksheet, const Range& range)
    : impl_(std::make_unique<Impl>(worksheet, range)) {
}

WorksheetRange::WorksheetRange(const WorksheetRange& other)
    : impl_(std::make_unique<Impl>(*other.impl_)) {
}

WorksheetRange::WorksheetRange(WorksheetRange&& other) noexcept
    : impl_(std::move(other.impl_)) {
}

WorksheetRange& WorksheetRange::operator=(const WorksheetRange& other) {
    if (this != &other) {
        impl_ = std::make_unique<Impl>(*other.impl_);
    }
    return *this;
}

WorksheetRange& WorksheetRange::operator=(WorksheetRange&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

WorksheetRange::~WorksheetRange() = default;

// ========================================
// 范围信息
// ========================================

std::string WorksheetRange::address() const {
    return impl_->range_.to_string();
}

std::size_t WorksheetRange::start_row() const {
    return impl_->range_.start().row;
}

std::size_t WorksheetRange::start_column() const {
    return impl_->range_.start().column;
}

std::size_t WorksheetRange::end_row() const {
    return impl_->range_.end().row;
}

std::size_t WorksheetRange::end_column() const {
    return impl_->range_.end().column;
}

std::size_t WorksheetRange::row_count() const {
    return end_row() - start_row() + 1;
}

std::size_t WorksheetRange::column_count() const {
    return end_column() - start_column() + 1;
}

std::size_t WorksheetRange::cell_count() const {
    return row_count() * column_count();
}

// ========================================
// 单元格访问
// ========================================

Cell& WorksheetRange::cell(std::size_t row, std::size_t col) {
    if (row >= row_count() || col >= column_count()) {
        throw std::out_of_range("Cell index out of range");
    }
    
    std::size_t actual_row = start_row() + row;
    std::size_t actual_col = start_column() + col;
    
    return impl_->worksheet_.cell(actual_row, actual_col);
}

const Cell& WorksheetRange::cell(std::size_t row, std::size_t col) const {
    if (row >= row_count() || col >= column_count()) {
        throw std::out_of_range("Cell index out of range");
    }
    
    std::size_t actual_row = start_row() + row;
    std::size_t actual_col = start_column() + col;
    
    return impl_->worksheet_.cell(actual_row, actual_col);
}

// ========================================
// 批量样式设置
// ========================================

WorksheetRange& WorksheetRange::style(const StyleTemplate& style_template) {
    if (style_template.has_any_style()) {
        // 获取样式ID
        auto* style_manager = impl_->worksheet_.get_style_manager();
        if (style_manager) {
            std::uint32_t style_id = style_template.apply_to_style_manager(*style_manager);

            // 应用到所有单元格
            for (std::size_t row = 0; row < row_count(); ++row) {
                for (std::size_t col = 0; col < column_count(); ++col) {
                    auto& cell_ref = cell(row, col);
                    // 直接设置样式ID，避免重复的样式计算
                    // 这里需要访问Cell的内部实现来直接设置样式ID
                    // 为了性能，我们可以批量设置
                    cell_ref.style(style_template);
                }
            }
        }
    }
    return *this;
}

WorksheetRange& WorksheetRange::font(const std::string& font_name, double size) {
    auto style_template = StyleTemplate().font(font_name, size);
    return style(style_template);
}

WorksheetRange& WorksheetRange::bold(bool bold) {
    auto style_template = StyleTemplate().bold(bold);
    return style(style_template);
}

WorksheetRange& WorksheetRange::italic(bool italic) {
    auto style_template = StyleTemplate().italic(italic);
    return style(style_template);
}

WorksheetRange& WorksheetRange::color(const Color& color) {
    auto style_template = StyleTemplate().color(color);
    return style(style_template);
}

WorksheetRange& WorksheetRange::background_color(const Color& color) {
    auto style_template = StyleTemplate().background_color(color);
    return style(style_template);
}

WorksheetRange& WorksheetRange::align(const Alignment& alignment) {
    auto style_template = StyleTemplate().align(alignment);
    return style(style_template);
}

WorksheetRange& WorksheetRange::align_horizontal(Alignment::Horizontal horizontal) {
    auto style_template = StyleTemplate().align_horizontal(horizontal);
    return style(style_template);
}

WorksheetRange& WorksheetRange::align_vertical(Alignment::Vertical vertical) {
    auto style_template = StyleTemplate().align_vertical(vertical);
    return style(style_template);
}

WorksheetRange& WorksheetRange::border(BorderType type, BorderStyle style) {
    auto style_template = StyleTemplate().border(type, style);
    return this->style(style_template);
}

WorksheetRange& WorksheetRange::border(BorderType type, BorderStyle style, const Color& color) {
    auto style_template = StyleTemplate().border(type, style, color);
    return this->style(style_template);
}

WorksheetRange& WorksheetRange::number_format(const std::string& format_code) {
    auto style_template = StyleTemplate().number_format(format_code);
    return style(style_template);
}

// ========================================
// 批量值设置
// ========================================

template<typename T>
WorksheetRange& WorksheetRange::value(const T& value) {
    for (std::size_t row = 0; row < row_count(); ++row) {
        for (std::size_t col = 0; col < column_count(); ++col) {
            cell(row, col).value(value);
        }
    }
    return *this;
}

template<typename T>
WorksheetRange& WorksheetRange::values(const std::vector<std::vector<T>>& values) {
    std::size_t max_rows = std::min(values.size(), row_count());

    for (std::size_t row = 0; row < max_rows; ++row) {
        const auto& row_values = values[row];
        std::size_t max_cols = std::min(row_values.size(), column_count());

        for (std::size_t col = 0; col < max_cols; ++col) {
            cell(row, col).value(row_values[col]);
        }
    }
    return *this;
}

// 字符串字面量特化
template<std::size_t N>
WorksheetRange& WorksheetRange::value(const char (&value)[N]) {
    std::string str_value(value);
    return this->value(str_value);
}

// 显式实例化常用类型
template WorksheetRange& WorksheetRange::value<std::string>(const std::string&);
template WorksheetRange& WorksheetRange::value<int>(const int&);
template WorksheetRange& WorksheetRange::value<double>(const double&);
template WorksheetRange& WorksheetRange::value<bool>(const bool&);

template WorksheetRange& WorksheetRange::values<std::string>(const std::vector<std::vector<std::string>>&);
template WorksheetRange& WorksheetRange::values<int>(const std::vector<std::vector<int>>&);
template WorksheetRange& WorksheetRange::values<double>(const std::vector<std::vector<double>>&);
template WorksheetRange& WorksheetRange::values<bool>(const std::vector<std::vector<bool>>&);

// ========================================
// 迭代器实现
// ========================================

WorksheetRange::iterator::iterator(WorksheetRange& range, std::size_t index)
    : range_(range), index_(index) {
}

WorksheetRange::iterator::reference WorksheetRange::iterator::operator*() {
    std::size_t row = index_ / range_.column_count();
    std::size_t col = index_ % range_.column_count();
    return range_.cell(row, col);
}

WorksheetRange::iterator::pointer WorksheetRange::iterator::operator->() {
    return &(operator*());
}

WorksheetRange::iterator& WorksheetRange::iterator::operator++() {
    ++index_;
    return *this;
}

WorksheetRange::iterator WorksheetRange::iterator::operator++(int) {
    iterator temp = *this;
    ++index_;
    return temp;
}

bool WorksheetRange::iterator::operator==(const iterator& other) const {
    return &range_ == &other.range_ && index_ == other.index_;
}

bool WorksheetRange::iterator::operator!=(const iterator& other) const {
    return !(*this == other);
}

WorksheetRange::const_iterator::const_iterator(const WorksheetRange& range, std::size_t index)
    : range_(range), index_(index) {
}

WorksheetRange::const_iterator::reference WorksheetRange::const_iterator::operator*() const {
    std::size_t row = index_ / range_.column_count();
    std::size_t col = index_ % range_.column_count();
    return range_.cell(row, col);
}

WorksheetRange::const_iterator::pointer WorksheetRange::const_iterator::operator->() const {
    return &(operator*());
}

WorksheetRange::const_iterator& WorksheetRange::const_iterator::operator++() {
    ++index_;
    return *this;
}

WorksheetRange::const_iterator WorksheetRange::const_iterator::operator++(int) {
    const_iterator temp = *this;
    ++index_;
    return temp;
}

bool WorksheetRange::const_iterator::operator==(const const_iterator& other) const {
    return &range_ == &other.range_ && index_ == other.index_;
}

bool WorksheetRange::const_iterator::operator!=(const const_iterator& other) const {
    return !(*this == other);
}

WorksheetRange::iterator WorksheetRange::begin() {
    return iterator(*this, 0);
}

WorksheetRange::iterator WorksheetRange::end() {
    return iterator(*this, cell_count());
}

WorksheetRange::const_iterator WorksheetRange::begin() const {
    return const_iterator(*this, 0);
}

WorksheetRange::const_iterator WorksheetRange::end() const {
    return const_iterator(*this, cell_count());
}

WorksheetRange::const_iterator WorksheetRange::cbegin() const {
    return const_iterator(*this, 0);
}

WorksheetRange::const_iterator WorksheetRange::cend() const {
    return const_iterator(*this, cell_count());
}

} // namespace tinakit::excel
