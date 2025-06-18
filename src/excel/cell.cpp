/**
 * @file cell.cpp
 * @brief Excel 单元格类实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/excel/cell.hpp"
#include "tinakit/core/types.hpp"
#include "tinakit/core/exceptions.hpp"
#include <sstream>

namespace tinakit::excel {

/**
 * @class CellImpl
 * @brief Cell 类的实现细节（PIMPL 模式）
 */
class CellImpl {
public:
    CellImpl(std::size_t row, std::size_t column);
    ~CellImpl() = default;

    // 基本属性
    std::size_t row() const noexcept { return row_; }
    std::size_t column() const noexcept { return column_; }
    std::string address() const;
    
    // 值操作
    void set_value(const Cell::CellValue& value);
    const Cell::CellValue& get_value() const;
    bool is_empty() const;
    
    // 公式操作
    void set_formula(const std::string& formula);
    const std::optional<std::string>& get_formula() const;
    
    // 样式操作（TODO: 待实现）
    void set_font(const std::string& name, double size);
    void set_bold(bool bold);
    void set_italic(bool italic);
    void set_color(const Color& color);
    void set_background_color(const Color& color);
    void set_alignment(Alignment alignment);
    void set_border(BorderType type, BorderStyle style);

    // 获取和设置样式 ID
    void set_style_id(std::uint32_t style_id) { style_id_ = style_id; }
    std::uint32_t get_style_id() const { return style_id_; }

private:
    std::size_t row_, column_;
    Cell::CellValue value_ = std::string("");
    std::optional<std::string> formula_;
    std::uint32_t style_id_ = 0;  // 默认样式 ID
    
    // TODO: 详细的样式属性（字体、颜色等）
};

// =============================================================================
// CellImpl 实现
// =============================================================================

CellImpl::CellImpl(std::size_t row, std::size_t column) 
    : row_(row), column_(column) {
}

std::string CellImpl::address() const {
    return column_number_to_name(column_) + std::to_string(row_);
}

void CellImpl::set_value(const Cell::CellValue& value) {
    value_ = value;
    formula_.reset(); // 设置值会清除公式
}

const Cell::CellValue& CellImpl::get_value() const {
    return value_;
}

bool CellImpl::is_empty() const {
    return std::holds_alternative<std::string>(value_) && 
           std::get<std::string>(value_).empty();
}

void CellImpl::set_formula(const std::string& formula) {
    formula_ = formula;
    // TODO: 计算公式结果
}

const std::optional<std::string>& CellImpl::get_formula() const {
    return formula_;
}

// 样式方法的占位符实现
void CellImpl::set_font(const std::string& name, double size) {
    // TODO: 实现字体设置
}

void CellImpl::set_bold(bool bold) {
    // TODO: 实现粗体设置
}

void CellImpl::set_italic(bool italic) {
    // TODO: 实现斜体设置
}

void CellImpl::set_color(const Color& color) {
    // TODO: 实现字体颜色设置
}

void CellImpl::set_background_color(const Color& color) {
    // TODO: 实现背景颜色设置
}

void CellImpl::set_alignment(Alignment alignment) {
    // TODO: 实现对齐设置
}

void CellImpl::set_border(BorderType type, BorderStyle style) {
    // TODO: 实现边框设置
}

// =============================================================================
// Cell 公共接口实现
// =============================================================================

Cell::Cell(std::unique_ptr<CellImpl> impl) 
    : impl_(std::move(impl)) {
}

Cell::~Cell() = default;

std::unique_ptr<Cell> Cell::create(std::size_t row, std::size_t column) {
    auto impl = std::make_unique<CellImpl>(row, column);
    return std::unique_ptr<Cell>(new Cell(std::move(impl)));
}

Cell::Cell(Cell&& other) noexcept
    : impl_(std::move(other.impl_)) {
}

Cell& Cell::operator=(Cell&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

// 模板特化实现
template<>
Cell& Cell::value<std::string>(const std::string& value) {
    impl_->set_value(value);
    return *this;
}

template<>
Cell& Cell::value<int>(const int& value) {
    impl_->set_value(value);
    return *this;
}

template<>
Cell& Cell::value<double>(const double& value) {
    impl_->set_value(value);
    return *this;
}

template<>
Cell& Cell::value<bool>(const bool& value) {
    impl_->set_value(value);
    return *this;
}

template<>
std::string Cell::as<std::string>() const {
    const auto& value = impl_->get_value();
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    } else if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    } else if (std::holds_alternative<double>(value)) {
        return std::to_string(std::get<double>(value));
    } else if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "TRUE" : "FALSE";
    }
    return "";
}

template<>
int Cell::as<int>() const {
    const auto& value = impl_->get_value();
    if (std::holds_alternative<int>(value)) {
        return std::get<int>(value);
    } else if (std::holds_alternative<double>(value)) {
        return static_cast<int>(std::get<double>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        try {
            return std::stoi(std::get<std::string>(value));
        } catch (const std::exception&) {
            throw TypeConversionException("string", "int", std::get<std::string>(value));
        }
    }
    throw TypeConversionException("unknown", "int");
}

template<>
double Cell::as<double>() const {
    const auto& value = impl_->get_value();
    if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
    } else if (std::holds_alternative<int>(value)) {
        return static_cast<double>(std::get<int>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        try {
            return std::stod(std::get<std::string>(value));
        } catch (const std::exception&) {
            throw TypeConversionException("string", "double", std::get<std::string>(value));
        }
    }
    throw TypeConversionException("unknown", "double");
}

template<>
bool Cell::as<bool>() const {
    const auto& value = impl_->get_value();
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value);
    } else if (std::holds_alternative<int>(value)) {
        return std::get<int>(value) != 0;
    } else if (std::holds_alternative<double>(value)) {
        return std::get<double>(value) != 0.0;
    } else if (std::holds_alternative<std::string>(value)) {
        const auto& str = std::get<std::string>(value);
        return str == "TRUE" || str == "true" || str == "1";
    }
    return false;
}

template<>
std::optional<std::string> Cell::try_as<std::string>() const noexcept {
    try {
        return as<std::string>();
    } catch (...) {
        return std::nullopt;
    }
}

template<>
std::optional<int> Cell::try_as<int>() const noexcept {
    try {
        return as<int>();
    } catch (...) {
        return std::nullopt;
    }
}

template<>
std::optional<double> Cell::try_as<double>() const noexcept {
    try {
        return as<double>();
    } catch (...) {
        return std::nullopt;
    }
}

template<>
std::optional<bool> Cell::try_as<bool>() const noexcept {
    try {
        return as<bool>();
    } catch (...) {
        return std::nullopt;
    }
}

// 公式操作
Cell& Cell::formula(const std::string& formula) {
    impl_->set_formula(formula);
    return *this;
}

std::optional<std::string> Cell::formula() const {
    return impl_->get_formula();
}

// 样式操作
Cell& Cell::font(const std::string& font_name, double size) {
    impl_->set_font(font_name, size);
    return *this;
}

Cell& Cell::bold(bool bold) {
    impl_->set_bold(bold);
    return *this;
}

Cell& Cell::italic(bool italic) {
    impl_->set_italic(italic);
    return *this;
}

Cell& Cell::color(const Color& color) {
    impl_->set_color(color);
    return *this;
}

Cell& Cell::background_color(const Color& color) {
    impl_->set_background_color(color);
    return *this;
}

Cell& Cell::align(Alignment alignment) {
    impl_->set_alignment(alignment);
    return *this;
}

Cell& Cell::border(BorderType border_type, BorderStyle style) {
    impl_->set_border(border_type, style);
    return *this;
}

// 属性访问
std::string Cell::address() const {
    return impl_->address();
}

std::size_t Cell::row() const noexcept {
    return impl_->row();
}

std::size_t Cell::column() const noexcept {
    return impl_->column();
}

bool Cell::empty() const noexcept {
    return impl_->is_empty();
}

std::string Cell::to_string() const {
    return as<std::string>();
}

Cell& Cell::style_id(std::uint32_t style_id) {
    impl_->set_style_id(style_id);
    return *this;
}

std::uint32_t Cell::style_id() const {
    return impl_->get_style_id();
}

std::ostream& operator<<(std::ostream& os, const Cell& cell) {
    os << cell.to_string();
    return os;
}

} // namespace tinakit::excel 