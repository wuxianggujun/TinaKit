/**
 * @file cell.cpp
 * @brief Excel 单元格句柄类实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/excel/cell.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/style_manager.hpp"
#include "tinakit/excel/style_template.hpp"
#include "tinakit/internal/workbook_impl.hpp"
#include "tinakit/core/exceptions.hpp"
#include "tinakit/excel/types.hpp"
#include <sstream>

namespace tinakit::excel {

// ========================================
// 构造函数和析构函数
// ========================================

Cell::Cell(std::shared_ptr<internal::workbook_impl> workbook_impl, 
           std::string sheet_name, 
           std::size_t row, 
           std::size_t column)
    : workbook_impl_(std::move(workbook_impl))
    , sheet_name_(std::move(sheet_name))
    , row_(row)
    , column_(column) {
}

// ========================================
// 基本属性
// ========================================

std::string Cell::address() const {
    return column_number_to_name(column_) + std::to_string(row_);
}

std::size_t Cell::row() const noexcept {
    return row_;
}

std::size_t Cell::column() const noexcept {
    return column_;
}

bool Cell::empty() const noexcept {
    auto pos = core::Position(row_, column_);
    auto data = workbook_impl_->get_cell_data(sheet_name_, pos);
    return std::holds_alternative<std::string>(data.value) &&
           std::get<std::string>(data.value).empty();
}

const Cell::CellValue& Cell::raw_value() const {
    auto pos = core::Position(row_, column_);
    auto data = workbook_impl_->get_cell_data(sheet_name_, pos);
    static thread_local CellValue cached_value;
    cached_value = data.value;
    return cached_value;
}

std::string Cell::to_string() const {
    auto pos = core::Position(row_, column_);
    auto data = workbook_impl_->get_cell_data(sheet_name_, pos);

    return std::visit([](const auto& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return value;
        } else if constexpr (std::is_same_v<T, double>) {
            return std::to_string(value);
        } else if constexpr (std::is_same_v<T, int>) {
            return std::to_string(value);
        } else if constexpr (std::is_same_v<T, bool>) {
            return value ? "TRUE" : "FALSE";
        }
        return "";
    }, data.value);
}

// ========================================
// 值操作（委托给 workbook_impl）
// ========================================

template<>
Cell& Cell::value<std::string>(const std::string& value) {
    auto pos = core::Position(row_, column_);
    workbook_impl_->set_cell_value(sheet_name_, pos, value);
    return *this;
}

template<>
Cell& Cell::value<int>(const int& value) {
    auto pos = core::Position(row_, column_);
    workbook_impl_->set_cell_value(sheet_name_, pos, value);
    return *this;
}

template<>
Cell& Cell::value<double>(const double& value) {
    auto pos = core::Position(row_, column_);
    workbook_impl_->set_cell_value(sheet_name_, pos, value);
    return *this;
}

template<>
Cell& Cell::value<bool>(const bool& value) {
    auto pos = core::Position(row_, column_);
    workbook_impl_->set_cell_value(sheet_name_, pos, value);
    return *this;
}

// ========================================
// 类型转换（委托给 workbook_impl）
// ========================================

template<>
std::string Cell::as<std::string>() const {
    auto pos = core::Position(row_, column_);
    auto data = workbook_impl_->get_cell_data(sheet_name_, pos);

    return std::visit([](const auto& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return value;
        } else if constexpr (std::is_same_v<T, double>) {
            return std::to_string(value);
        } else if constexpr (std::is_same_v<T, int>) {
            return std::to_string(value);
        } else if constexpr (std::is_same_v<T, bool>) {
            return value ? "TRUE" : "FALSE";
        }
        return "";
    }, data.value);
}

template<>
int Cell::as<int>() const {
    auto pos = core::Position(row_, column_);
    auto data = workbook_impl_->get_cell_data(sheet_name_, pos);

    return std::visit([](const auto& value) -> int {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int>) {
            return value;
        } else if constexpr (std::is_same_v<T, double>) {
            return static_cast<int>(value);
        } else if constexpr (std::is_same_v<T, bool>) {
            return value ? 1 : 0;
        } else if constexpr (std::is_same_v<T, std::string>) {
            try {
                return std::stoi(value);
            } catch (...) {
                return 0;
            }
        }
        return 0;
    }, data.value);
}

template<>
double Cell::as<double>() const {
    auto pos = core::Position(row_, column_);
    auto data = workbook_impl_->get_cell_data(sheet_name_, pos);

    return std::visit([](const auto& value) -> double {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, double>) {
            return value;
        } else if constexpr (std::is_same_v<T, int>) {
            return static_cast<double>(value);
        } else if constexpr (std::is_same_v<T, bool>) {
            return value ? 1.0 : 0.0;
        } else if constexpr (std::is_same_v<T, std::string>) {
            try {
                return std::stod(value);
            } catch (...) {
                return 0.0;
            }
        }
        return 0.0;
    }, data.value);
}

template<>
bool Cell::as<bool>() const {
    auto pos = core::Position(row_, column_);
    auto data = workbook_impl_->get_cell_data(sheet_name_, pos);

    return std::visit([](const auto& value) -> bool {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, bool>) {
            return value;
        } else if constexpr (std::is_same_v<T, int>) {
            return value != 0;
        } else if constexpr (std::is_same_v<T, double>) {
            return value != 0.0;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return !value.empty() && value != "0" && value != "FALSE" && value != "false";
        }
        return false;
    }, data.value);
}

template<>
std::optional<std::string> Cell::try_as<std::string>() const noexcept {
    // TODO: 委托给 workbook_impl
    return std::nullopt;
}

template<>
std::optional<int> Cell::try_as<int>() const noexcept {
    // TODO: 委托给 workbook_impl
    return std::nullopt;
}

template<>
std::optional<double> Cell::try_as<double>() const noexcept {
    // TODO: 委托给 workbook_impl
    return std::nullopt;
}

template<>
std::optional<bool> Cell::try_as<bool>() const noexcept {
    // TODO: 委托给 workbook_impl
    return std::nullopt;
}

// ========================================
// 公式操作（委托给 workbook_impl）
// ========================================

Cell& Cell::formula(const std::string& formula) {
    auto pos = core::Position(row_, column_);
    workbook_impl_->set_cell_formula(sheet_name_, pos, formula);
    return *this;
}

std::optional<std::string> Cell::formula() const {
    auto pos = core::Position(row_, column_);
    auto data = workbook_impl_->get_cell_data(sheet_name_, pos);
    return data.formula;
}

// ========================================
// 样式操作（委托给 workbook_impl）
// ========================================

Cell& Cell::font(const std::string& font_name, double size) {
    // TODO: 委托给 workbook_impl
    (void)font_name;
    (void)size;
    return *this;
}

Cell& Cell::bold(bool bold) {
    // TODO: 委托给 workbook_impl
    (void)bold;
    return *this;
}

Cell& Cell::italic(bool italic) {
    // TODO: 委托给 workbook_impl
    (void)italic;
    return *this;
}

Cell& Cell::color(const Color& color) {
    // TODO: 委托给 workbook_impl
    (void)color;
    return *this;
}

Cell& Cell::background_color(const Color& color) {
    // TODO: 委托给 workbook_impl
    (void)color;
    return *this;
}

Cell& Cell::align(const Alignment& alignment) {
    // TODO: 委托给 workbook_impl
    (void)alignment;
    return *this;
}

Cell& Cell::border(BorderType border_type, BorderStyle style) {
    // TODO: 委托给 workbook_impl
    (void)border_type;
    (void)style;
    return *this;
}

Cell& Cell::border(BorderType border_type, BorderStyle style, const Color& color) {
    // TODO: 委托给 workbook_impl
    (void)border_type;
    (void)style;
    (void)color;
    return *this;
}

Cell& Cell::number_format(const std::string& format_code) {
    // TODO: 委托给 workbook_impl
    (void)format_code;
    return *this;
}

Cell& Cell::wrap_text(bool wrap) {
    // TODO: 委托给 workbook_impl
    (void)wrap;
    return *this;
}

Cell& Cell::indent(int indent_level) {
    // TODO: 委托给 workbook_impl
    (void)indent_level;
    return *this;
}

Cell& Cell::style(const StyleTemplate& style_template) {
    // TODO: 委托给 workbook_impl
    (void)style_template;
    return *this;
}

Cell& Cell::style_id(std::uint32_t style_id) {
    auto pos = core::Position(row_, column_);
    workbook_impl_->set_cell_style(sheet_name_, pos, style_id);
    return *this;
}

std::uint32_t Cell::style_id() const {
    auto pos = core::Position(row_, column_);
    auto data = workbook_impl_->get_cell_data(sheet_name_, pos);
    return data.style_id;
}

bool Cell::has_custom_style() const {
    auto pos = core::Position(row_, column_);
    auto data = workbook_impl_->get_cell_data(sheet_name_, pos);
    return data.style_id != 0;
}

// ========================================
// 输出流操作符
// ========================================

std::ostream& operator<<(std::ostream& os, const Cell& cell) {
    os << cell.to_string();
    return os;
}

} // namespace tinakit::excel
