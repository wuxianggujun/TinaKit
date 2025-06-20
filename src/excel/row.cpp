/**
 * @file row.cpp
 * @brief Excel 行类实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/excel/row.hpp"

#include <map>

#include "tinakit/excel/cell.hpp"
#include "tinakit/core/types.hpp"
#include "tinakit/core/exceptions.hpp"

namespace tinakit::excel {

/**
 * @class RowImpl
 * @brief Row 类的实现细节（PIMPL 模式）
 */
class RowImpl {
public:
    explicit RowImpl(std::size_t index);
    ~RowImpl() = default;

    // 基本属性
    std::size_t index() const noexcept { return index_; }
    double height() const noexcept { return height_; }
    void set_height(double height) { height_ = height; }
    
    // 单元格访问
    Cell& get_cell(const std::string& column_name);
    const Cell& get_cell(const std::string& column_name) const;
    Cell& get_cell(std::size_t column_index);
    const Cell& get_cell(std::size_t column_index) const;
    
    // 状态查询
    bool is_empty() const;
    std::size_t size() const;

private:
    std::size_t index_;
    double height_ = 15.0; // 默认行高
    mutable std::map<std::size_t, std::unique_ptr<Cell>> cells_;
    std::size_t max_column_ = 0;
};

// =============================================================================
// RowImpl 实现
// =============================================================================

RowImpl::RowImpl(std::size_t index) : index_(index) {
}

Cell& RowImpl::get_cell(const std::string& column_name) {
    auto column_index = column_name_to_number(column_name);
    return get_cell(column_index);
}

const Cell& RowImpl::get_cell(const std::string& column_name) const {
    auto column_index = column_name_to_number(column_name);
    return get_cell(column_index);
}

Cell& RowImpl::get_cell(std::size_t column_index) {
    // TODO: 重构为句柄模式后重新实现
    (void)column_index;
    throw std::runtime_error("Row::get_cell not implemented yet after Cell refactoring");
}

const Cell& RowImpl::get_cell(std::size_t column_index) const {
    auto it = cells_.find(column_index);
    if (it == cells_.end()) {
        throw std::out_of_range("Cell not found at column " + std::to_string(column_index));
    }
    return *it->second;
}

bool RowImpl::is_empty() const {
    return cells_.empty() || std::all_of(cells_.begin(), cells_.end(),
        [](const auto& pair) { return pair.second->empty(); });
}

std::size_t RowImpl::size() const {
    return max_column_;
}

// =============================================================================
// Row 公共接口实现
// =============================================================================

Row::Row(std::unique_ptr<RowImpl> impl) 
    : impl_(std::move(impl)) {
}

Row::~Row() = default;

std::unique_ptr<Row> Row::create(std::size_t index) {
    auto impl = std::make_unique<RowImpl>(index);
    return std::unique_ptr<Row>(new Row(std::move(impl)));
}

Row::Row(Row&& other) noexcept
    : impl_(std::move(other.impl_)) {
}

Row& Row::operator=(Row&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

// 单元格访问
Cell& Row::operator[](const std::string& column_name) {
    return impl_->get_cell(column_name);
}

const Cell& Row::operator[](const std::string& column_name) const {
    return impl_->get_cell(column_name);
}

Cell& Row::operator[](std::size_t column_index) {
    return impl_->get_cell(column_index);
}

const Cell& Row::operator[](std::size_t column_index) const {
    return impl_->get_cell(column_index);
}

// 属性访问
std::size_t Row::index() const noexcept {
    return impl_->index();
}

double Row::height() const noexcept {
    return impl_->height();
}

void Row::set_height(double height) {
    impl_->set_height(height);
}

Row& Row::height(double height) {
    impl_->set_height(height);
    return *this;
}

bool Row::empty() const {
    return impl_->is_empty();
}

std::size_t Row::size() const {
    return impl_->size();
}

// 迭代器实现（基本版本）
Row::iterator Row::begin() {
    return iterator(this, 1);
}

Row::iterator Row::end() {
    return iterator(this, size() + 1);
}

Row::const_iterator Row::begin() const {
    return const_iterator(this, 1);
}

Row::const_iterator Row::end() const {
    return const_iterator(this, size() + 1);
}

Row::const_iterator Row::cbegin() const {
    return begin();
}

Row::const_iterator Row::cend() const {
    return end();
}

// =============================================================================
// Row::iterator 实现
// =============================================================================

Row::iterator::iterator(Row* row, std::size_t column) 
    : row_(row), column_(column) {
}

Row::iterator::reference Row::iterator::operator*() const {
    return (*row_)[column_];
}

Row::iterator::pointer Row::iterator::operator->() const {
    return &((*row_)[column_]);
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
    return row_ == other.row_ && column_ == other.column_;
}

bool Row::iterator::operator!=(const iterator& other) const {
    return !(*this == other);
}

// =============================================================================
// Row::const_iterator 实现
// =============================================================================

Row::const_iterator::const_iterator(const Row* row, std::size_t column) 
    : row_(row), column_(column) {
}

Row::const_iterator::reference Row::const_iterator::operator*() const {
    return (*row_)[column_];
}

Row::const_iterator::pointer Row::const_iterator::operator->() const {
    return &((*row_)[column_]);
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
    return row_ == other.row_ && column_ == other.column_;
}

bool Row::const_iterator::operator!=(const const_iterator& other) const {
    return !(*this == other);
}

} // namespace tinakit::excel
