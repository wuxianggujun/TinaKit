/**
 * @file range_view.cpp
 * @brief Excel 范围视图/迭代器类实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/excel/range_view.hpp"
#include "tinakit/excel/cell.hpp"
#include "tinakit/internal/workbook_impl.hpp"
#include "tinakit/internal/coordinate_utils.hpp"
#include <unordered_map>

namespace tinakit::excel {

// ========================================
// RangeView 构造函数
// ========================================

RangeView::RangeView(std::shared_ptr<internal::workbook_impl> workbook_impl,
                     std::string sheet_name,
                     core::range_address range_addr)
    : workbook_impl_(std::move(workbook_impl))
    , sheet_name_(std::move(sheet_name))
    , range_addr_(range_addr) {
}

// ========================================
// 范围信息
// ========================================

std::string RangeView::address() const {
    return internal::utils::CoordinateUtils::range_address_to_string(range_addr_);
}

std::size_t RangeView::start_row() const {
    return range_addr_.start.row;
}

std::size_t RangeView::start_column() const {
    return range_addr_.start.column;
}

std::size_t RangeView::end_row() const {
    return range_addr_.end.row;
}

std::size_t RangeView::end_column() const {
    return range_addr_.end.column;
}

std::size_t RangeView::row_count() const {
    return range_addr_.end.row - range_addr_.start.row + 1;
}

std::size_t RangeView::column_count() const {
    return range_addr_.end.column - range_addr_.start.column + 1;
}

std::size_t RangeView::cell_count() const {
    return row_count() * column_count();
}

// ========================================
// 单元格访问
// ========================================

Cell& RangeView::cell(std::size_t row, std::size_t col) {
    // 转换相对坐标为绝对坐标
    std::size_t abs_row = range_addr_.start.row + row;
    std::size_t abs_col = range_addr_.start.column + col;
    
    // 边界检查
    if (abs_row > range_addr_.end.row || abs_col > range_addr_.end.column) {
        throw std::out_of_range("Cell position out of range");
    }
    
    // 委托给workbook_impl获取Cell
    // 注意：这里需要实现一个缓存机制来避免重复创建Cell对象
    static thread_local std::unordered_map<std::uint64_t, Cell> cell_cache;
    
    std::uint64_t key = (static_cast<std::uint64_t>(abs_row) << 32) | abs_col;
    auto it = cell_cache.find(key);
    if (it != cell_cache.end()) {
        return it->second;
    }
    
    // 创建新的Cell对象并缓存
    auto sheet_id = workbook_impl_->get_sheet_id(sheet_name_);
    auto [inserted_it, success] = cell_cache.emplace(
        key, Cell(workbook_impl_.get(), sheet_id, abs_row, abs_col)
    );
    return inserted_it->second;
}

const Cell& RangeView::cell(std::size_t row, std::size_t col) const {
    // const版本委托给非const版本
    return const_cast<RangeView*>(this)->cell(row, col);
}

// ========================================
// 迭代器实现
// ========================================

// iterator 实现
RangeView::iterator::iterator(RangeView& range, std::size_t index)
    : range_(range), index_(index) {
}

RangeView::iterator::reference RangeView::iterator::operator*() {
    std::size_t row = index_ / range_.column_count();
    std::size_t col = index_ % range_.column_count();
    return range_.cell(row, col);
}

RangeView::iterator::pointer RangeView::iterator::operator->() {
    return &(operator*());
}

RangeView::iterator& RangeView::iterator::operator++() {
    ++index_;
    return *this;
}

RangeView::iterator RangeView::iterator::operator++(int) {
    iterator temp = *this;
    ++index_;
    return temp;
}

bool RangeView::iterator::operator==(const iterator& other) const {
    return &range_ == &other.range_ && index_ == other.index_;
}

bool RangeView::iterator::operator!=(const iterator& other) const {
    return !(*this == other);
}

// const_iterator 实现
RangeView::const_iterator::const_iterator(const RangeView& range, std::size_t index)
    : range_(range), index_(index) {
}

RangeView::const_iterator::reference RangeView::const_iterator::operator*() const {
    std::size_t row = index_ / range_.column_count();
    std::size_t col = index_ % range_.column_count();
    return range_.cell(row, col);
}

RangeView::const_iterator::pointer RangeView::const_iterator::operator->() const {
    return &(operator*());
}

RangeView::const_iterator& RangeView::const_iterator::operator++() {
    ++index_;
    return *this;
}

RangeView::const_iterator RangeView::const_iterator::operator++(int) {
    const_iterator temp = *this;
    ++index_;
    return temp;
}

bool RangeView::const_iterator::operator==(const const_iterator& other) const {
    return &range_ == &other.range_ && index_ == other.index_;
}

bool RangeView::const_iterator::operator!=(const const_iterator& other) const {
    return !(*this == other);
}

// 迭代器获取方法
RangeView::iterator RangeView::begin() {
    return iterator(*this, 0);
}

RangeView::iterator RangeView::end() {
    return iterator(*this, cell_count());
}

RangeView::const_iterator RangeView::begin() const {
    return const_iterator(*this, 0);
}

RangeView::const_iterator RangeView::end() const {
    return const_iterator(*this, cell_count());
}

RangeView::const_iterator RangeView::cbegin() const {
    return begin();
}

RangeView::const_iterator RangeView::cend() const {
    return end();
}

} // namespace tinakit::excel
