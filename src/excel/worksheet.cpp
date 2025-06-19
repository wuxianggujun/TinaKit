/**
 * @file worksheet.cpp
 * @brief Excel 工作表类实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/cell.hpp"
#include "tinakit/excel/row.hpp"
#include "tinakit/excel/range.hpp"
#include "tinakit/core/exceptions.hpp"
#include "tinakit/excel/types.hpp"
#include "tinakit/excel/style_manager.hpp"
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
    explicit Impl(const std::string& name, StyleManager* style_manager);
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

    // 样式管理
    StyleManager* get_style_manager() const { return style_manager_; }

    // 列宽管理
    void set_column_width(const std::string& column_name, double width);
    void set_column_width(std::size_t column_index, double width);
    double get_column_width(const std::string& column_name) const;
    double get_column_width(std::size_t column_index) const;

    // 合并单元格管理
    void merge_cells(std::size_t start_row, std::size_t start_col,
                     std::size_t end_row, std::size_t end_col);
    void unmerge_cells(std::size_t start_row, std::size_t start_col,
                       std::size_t end_row, std::size_t end_col);
    const std::vector<Range>& get_merged_ranges() const { return merged_ranges_; }

private:
    std::string name_;
    StyleManager* style_manager_ = nullptr;  // 样式管理器引用
    // TODO: 实际的单元格和行数据结构
    mutable std::map<std::pair<std::size_t, std::size_t>, std::unique_ptr<Cell>> cells_;
    mutable std::map<std::size_t, std::unique_ptr<Row>> rows_;

    std::size_t max_row_ = 0;
    std::size_t max_column_ = 0;

    // 列宽存储（列索引 -> 宽度）
    std::map<std::size_t, double> column_widths_;

    // 合并单元格范围存储
    std::vector<Range> merged_ranges_;
};



// =============================================================================
// WorksheetImpl 实现
// =============================================================================

Worksheet::Impl::Impl(const std::string& name) : name_(name) {
}

Worksheet::Impl::Impl(const std::string& name, StyleManager* style_manager)
    : name_(name), style_manager_(style_manager) {
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

        // 设置工作表引用，让 Cell 能够访问样式管理器
        // 注意：这里需要传递包含此 Impl 的 Worksheet 对象
        // 暂时先不设置，稍后在 Worksheet 层面处理

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
    (void)value; // 避免未使用参数警告
    std::vector<std::string> result;
    // TODO: 实现查找逻辑
    return result;
}

std::size_t Worksheet::Impl::replace_value(const std::string& old_value, const std::string& new_value) {
    (void)old_value; // 避免未使用参数警告
    (void)new_value; // 避免未使用参数警告
    std::size_t count = 0;
    // TODO: 实现替换逻辑
    return count;
}

void Worksheet::Impl::set_column_width(const std::string& column_name, double width) {
    auto column_index = column_name_to_number(column_name);
    set_column_width(column_index, width);
}

void Worksheet::Impl::set_column_width(std::size_t column_index, double width) {
    column_widths_[column_index] = width;
}

double Worksheet::Impl::get_column_width(const std::string& column_name) const {
    auto column_index = column_name_to_number(column_name);
    return get_column_width(column_index);
}

double Worksheet::Impl::get_column_width(std::size_t column_index) const {
    auto it = column_widths_.find(column_index);
    if (it != column_widths_.end()) {
        return it->second;
    }
    return 8.43; // Excel默认列宽
}

void Worksheet::Impl::merge_cells(std::size_t start_row, std::size_t start_col,
                                  std::size_t end_row, std::size_t end_col) {
    // 验证参数
    if (start_row > end_row || start_col > end_col) {
        throw std::invalid_argument("Invalid merge range: start position must be <= end position");
    }

    if (start_row == end_row && start_col == end_col) {
        // 单个单元格，无需合并
        return;
    }

    // 创建合并范围
    Range merge_range(Position(start_row, start_col), Position(end_row, end_col));

    // 检查是否与现有合并范围重叠
    for (const auto& existing_range : merged_ranges_) {
        if (merge_range.overlaps(existing_range)) {
            throw std::invalid_argument("Merge range overlaps with existing merged range");
        }
    }

    // 添加到合并范围列表
    merged_ranges_.push_back(merge_range);
}

void Worksheet::Impl::unmerge_cells(std::size_t start_row, std::size_t start_col,
                                    std::size_t end_row, std::size_t end_col) {
    Range target_range(Position(start_row, start_col), Position(end_row, end_col));

    // 查找并移除匹配的合并范围
    auto it = std::find_if(merged_ranges_.begin(), merged_ranges_.end(),
        [&target_range](const Range& range) {
            return range == target_range;
        });

    if (it != merged_ranges_.end()) {
        merged_ranges_.erase(it);
    }
}

// =============================================================================
// Worksheet 公共接口实现
// =============================================================================

std::shared_ptr<Worksheet> Worksheet::create(const std::string& name) {
    auto impl = std::make_unique<Impl>(name);
    return std::make_shared<Worksheet>(std::move(impl));
}

std::shared_ptr<Worksheet> Worksheet::create(const std::string& name, StyleManager* style_manager) {
    auto impl = std::make_unique<Impl>(name, style_manager);
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
    auto& cell = impl_->get_cell(address);
    cell.set_worksheet(this);
    return cell;
}

const Cell& Worksheet::operator[](const std::string& address) const {
    return impl_->get_cell(address);
}

Cell& Worksheet::cell(const std::string& address) {
    auto& cell = impl_->get_cell(address);
    cell.set_worksheet(this);
    return cell;
}

const Cell& Worksheet::cell(const std::string& address) const {
    return impl_->get_cell(address);
}

Cell& Worksheet::cell(std::size_t row, std::size_t column) {
    auto& cell = impl_->get_cell(row, column);
    // 设置工作表引用，让 Cell 能够访问样式管理器
    cell.set_worksheet(this);
    return cell;
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

// 样式管理
StyleManager* Worksheet::get_style_manager() const {
    return impl_->get_style_manager();
}

// 列宽管理
void Worksheet::set_column_width(const std::string& column_name, double width) {
    impl_->set_column_width(column_name, width);
}

void Worksheet::set_column_width(std::size_t column_index, double width) {
    impl_->set_column_width(column_index, width);
}

double Worksheet::get_column_width(const std::string& column_name) const {
    return impl_->get_column_width(column_name);
}

double Worksheet::get_column_width(std::size_t column_index) const {
    return impl_->get_column_width(column_index);
}

// 合并单元格
void Worksheet::merge_cells(const std::string& range_str) {
    auto range = Range::from_string(range_str);
    auto start_pos = range.start();
    auto end_pos = range.end();
    merge_cells(start_pos.row, start_pos.column, end_pos.row, end_pos.column);
}

void Worksheet::merge_cells(std::size_t start_row, std::size_t start_col,
                           std::size_t end_row, std::size_t end_col) {
    impl_->merge_cells(start_row, start_col, end_row, end_col);
}

void Worksheet::unmerge_cells(const std::string& range_str) {
    auto range = Range::from_string(range_str);
    auto start_pos = range.start();
    auto end_pos = range.end();
    unmerge_cells(start_pos.row, start_pos.column, end_pos.row, end_pos.column);
}

void Worksheet::unmerge_cells(std::size_t start_row, std::size_t start_col,
                             std::size_t end_row, std::size_t end_col) {
    impl_->unmerge_cells(start_row, start_col, end_row, end_col);
}

const std::vector<Range>& Worksheet::get_merged_ranges() const {
    return impl_->get_merged_ranges();
}

// =============================================================================
// RowRange 实现
// =============================================================================

Worksheet::RowRange::RowRange(Worksheet& worksheet, std::size_t start_row, std::size_t end_row)
    : worksheet_(worksheet), start_row_(start_row), end_row_(end_row) {
}

} // namespace tinakit::excel
