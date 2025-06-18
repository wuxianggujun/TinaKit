/**
 * @file worksheet.cpp
 * @brief Excel 工作表类实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/cell.hpp"
#include "tinakit/excel/row.hpp"
#include "tinakit/core/exceptions.hpp"
#include "tinakit/core/types.hpp"
#include <algorithm>
#include <map>

namespace tinakit::excel {

/**
 * @class WorksheetImpl
 * @brief Worksheet 类的实现细节（PIMPL 模式）
 */
class Worksheet::Impl {
public:
    explicit Impl(const std::string& name);
    ~Impl() = default;

    // 单元格管理
    Cell& get_cell(const std::string& address);
    const Cell& get_cell(const std::string& address) const;
    Cell& get_cell(std::size_t row, std::size_t column);
    const Cell& get_cell(std::size_t row, std::size_t column) const;

    // 行管理
    Row& get_row(std::size_t index);
    const Row& get_row(std::size_t index) const;

    // 属性
    const std::string& get_name() const noexcept;
    void set_name(const std::string& name);
    
    bool is_empty() const noexcept;
    std::size_t get_max_row() const noexcept;
    std::size_t get_max_column() const noexcept;

    // 查找和替换
    std::vector<std::string> find_value(const std::string& value) const;
    std::size_t replace_value(const std::string& old_value, const std::string& new_value);

private:
    std::string name_;
    // TODO: 实际的单元格和行数据结构
    mutable std::map<std::pair<std::size_t, std::size_t>, std::unique_ptr<Cell>> cells_;
    mutable std::map<std::size_t, std::unique_ptr<Row>> rows_;
    
    std::size_t max_row_ = 0;
    std::size_t max_column_ = 0;
};



// =============================================================================
// WorksheetImpl 实现
// =============================================================================

Worksheet::Impl::Impl(const std::string& name) : name_(name) {
}

Cell& Worksheet::Impl::get_cell(const std::string& address) {
    auto pos = Position::from_address(address);
    return get_cell(pos.row, pos.column);
}

const Cell& Worksheet::Impl::get_cell(const std::string& address) const {
    auto pos = Position::from_address(address);
    return get_cell(pos.row, pos.column);
}

Cell& Worksheet::Impl::get_cell(std::size_t row, std::size_t column) {
    auto key = std::make_pair(row, column);
    auto it = cells_.find(key);
    if (it == cells_.end()) {
        // 使用 Cell 的工厂方法创建新的 Cell
        auto cell = Cell::create(row, column);
        auto& cell_ref = *cell;
        cells_[key] = std::move(cell);
        
        max_row_ = std::max(max_row_, row);
        max_column_ = std::max(max_column_, column);
        
        return cell_ref;
    }
    return *it->second;
}

const Cell& Worksheet::Impl::get_cell(std::size_t row, std::size_t column) const {
    auto key = std::make_pair(row, column);
    auto it = cells_.find(key);
    if (it == cells_.end()) {
        throw std::out_of_range("Cell not found at row " + std::to_string(row) + 
                               ", column " + std::to_string(column));
    }
    return *it->second;
}

Row& Worksheet::Impl::get_row(std::size_t index) {
    auto it = rows_.find(index);
    if (it == rows_.end()) {
        // 使用 Row 的工厂方法创建新的 Row
        auto row = Row::create(index);
        auto& row_ref = *row;
        rows_[index] = std::move(row);
        
        max_row_ = std::max(max_row_, index);
        
        return row_ref;
    }
    return *it->second;
}

const Row& Worksheet::Impl::get_row(std::size_t index) const {
    auto it = rows_.find(index);
    if (it == rows_.end()) {
        throw std::out_of_range("Row not found at index " + std::to_string(index));
    }
    return *it->second;
}

const std::string& Worksheet::Impl::get_name() const noexcept {
    return name_;
}

void Worksheet::Impl::set_name(const std::string& name) {
    name_ = name;
}

bool Worksheet::Impl::is_empty() const noexcept {
    return cells_.empty();
}

std::size_t Worksheet::Impl::get_max_row() const noexcept {
    return max_row_;
}

std::size_t Worksheet::Impl::get_max_column() const noexcept {
    return max_column_;
}

std::vector<std::string> Worksheet::Impl::find_value(const std::string& value) const {
    std::vector<std::string> result;
    // TODO: 实现查找逻辑
    return result;
}

std::size_t Worksheet::Impl::replace_value(const std::string& old_value, const std::string& new_value) {
    std::size_t count = 0;
    // TODO: 实现替换逻辑
    return count;
}

// =============================================================================
// Worksheet 公共接口实现
// =============================================================================

std::shared_ptr<Worksheet> Worksheet::create(const std::string& name) {
    auto impl = std::make_unique<Impl>(name);
    return std::make_shared<Worksheet>(std::move(impl));
}

Worksheet::Worksheet(std::unique_ptr<Impl> impl) 
    : impl_(std::move(impl)) {
}

Worksheet::~Worksheet() = default;

Worksheet::Worksheet(Worksheet&& other) noexcept
    : impl_(std::move(other.impl_)) {
}

Worksheet& Worksheet::operator=(Worksheet&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

// 单元格访问
Cell& Worksheet::operator[](const std::string& address) {
    return impl_->get_cell(address);
}

const Cell& Worksheet::operator[](const std::string& address) const {
    return impl_->get_cell(address);
}

Cell& Worksheet::cell(const std::string& address) {
    return impl_->get_cell(address);
}

const Cell& Worksheet::cell(const std::string& address) const {
    return impl_->get_cell(address);
}

Cell& Worksheet::cell(std::size_t row, std::size_t column) {
    return impl_->get_cell(row, column);
}

const Cell& Worksheet::cell(std::size_t row, std::size_t column) const {
    return impl_->get_cell(row, column);
}

// 行访问
Row& Worksheet::row(std::size_t index) {
    return impl_->get_row(index);
}

const Row& Worksheet::row(std::size_t index) const {
    return impl_->get_row(index);
}

// 属性
const std::string& Worksheet::name() const noexcept {
    return impl_->get_name();
}

void Worksheet::set_name(const std::string& name) {
    impl_->set_name(name);
}

bool Worksheet::empty() const noexcept {
    return impl_->is_empty();
}

std::size_t Worksheet::max_row() const noexcept {
    return impl_->get_max_row();
}

std::size_t Worksheet::max_column() const noexcept {
    return impl_->get_max_column();
}

// 查找和替换
std::vector<std::string> Worksheet::find(const std::string& value) const {
    return impl_->find_value(value);
}

std::size_t Worksheet::replace(const std::string& old_value, const std::string& new_value) {
    return impl_->replace_value(old_value, new_value);
}

// 范围操作
Range Worksheet::used_range() const {
    return Range(Position(1, 1), Position(max_row(), max_column()));
}

Worksheet::RowRange Worksheet::rows(std::size_t start_row, std::size_t end_row) {
    return RowRange(*this, start_row, end_row);
}

Worksheet::RowRange Worksheet::rows() {
    return RowRange(*this, 1, max_row());
}

Range Worksheet::range(const std::string& range_str) {
    return Range::from_string(range_str);
}

// 异步处理
async::Task<void> Worksheet::process_rows_async(std::function<async::Task<void>(const Row&)> processor) {
    for (std::size_t i = 1; i <= max_row(); ++i) {
        co_await processor(row(i));
    }
}

// =============================================================================
// RowRange 实现
// =============================================================================

Worksheet::RowRange::RowRange(Worksheet& worksheet, std::size_t start_row, std::size_t end_row)
    : worksheet_(worksheet), start_row_(start_row), end_row_(end_row) {
}

} // namespace tinakit::excel
