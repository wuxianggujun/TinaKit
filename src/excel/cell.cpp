/**
 * @file cell.cpp
 * @brief Excel 单元格类实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/excel/cell.hpp"

#include <iostream>

#include "tinakit/excel/types.hpp"
#include "tinakit/core/exceptions.hpp"
#include "tinakit/excel/style_manager.hpp"
#include "tinakit/excel/worksheet.hpp"
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
    
    // 样式操作
    void set_font(const std::string& name, double size);
    void set_bold(bool bold);
    void set_italic(bool italic);
    void set_color(const Color& color);
    void set_background_color(const Color& color);
    void set_alignment(const Alignment& alignment);
    void set_border(BorderType type, BorderStyle style);
    void set_number_format(const std::string& format_code);

    // 获取和设置样式 ID
    void set_style_id(std::uint32_t style_id) { style_id_ = style_id; has_custom_style_ = true; }
    std::uint32_t get_style_id() const { return style_id_; }
    bool has_custom_style() const { return has_custom_style_; }
    
    // 设置工作表引用（用于访问样式管理器）
    void set_worksheet(Worksheet* ws) { worksheet_ = ws; }
    
    // 应用累积的样式更改
    void apply_style_changes();

    // 获取样式信息（供StyleManager使用）
    bool has_font_changes() const { return font_changed_; }
    bool has_fill_changes() const { return fill_changed_; }
    bool has_border_changes() const { return border_changed_; }
    bool has_number_format_changes() const { return number_format_changed_; }
    bool has_alignment_changes() const { return alignment_changed_; }

    const std::optional<Font>& get_pending_font() const { return pending_font_; }
    const std::optional<Fill>& get_pending_fill() const { return pending_fill_; }
    const std::optional<Border>& get_pending_border() const { return pending_border_; }
    const std::optional<NumberFormat>& get_pending_number_format() const { return pending_number_format_; }
    const std::optional<Alignment>& get_pending_alignment() const { return pending_alignment_; }

private:
    std::size_t row_, column_;
    Cell::CellValue value_ = std::string("");
    std::optional<std::string> formula_;
    std::uint32_t style_id_ = 0;  // 默认样式 ID
    bool has_custom_style_ = false;
    
    // 工作表引用（用于访问样式管理器）
    Worksheet* worksheet_ = nullptr;
    
    // 待应用的样式更改
    std::optional<Font> pending_font_;
    std::optional<Fill> pending_fill_;
    std::optional<Border> pending_border_;
    std::optional<NumberFormat> pending_number_format_;
    std::optional<Alignment> pending_alignment_;
    
    // 样式更改标志
    bool font_changed_ = false;
    bool fill_changed_ = false;
    bool border_changed_ = false;
    bool number_format_changed_ = false;
    bool alignment_changed_ = false;
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

// 样式方法实现
void CellImpl::set_font(const std::string& name, double size) {
    if (!pending_font_) {
        // 如果有现有样式，从中复制字体设置
        if (has_custom_style_ && worksheet_) {
            // TODO: 从现有样式获取字体信息
            pending_font_ = Font();
        } else {
            pending_font_ = Font();
        }
    }
    pending_font_->name = name;
    pending_font_->size = size;
    font_changed_ = true;
}

void CellImpl::set_bold(bool bold) {
    if (!pending_font_) {
        pending_font_ = Font();
    }
    pending_font_->bold = bold;
    font_changed_ = true;
}

void CellImpl::set_italic(bool italic) {
    if (!pending_font_) {
        pending_font_ = Font();
    }
    pending_font_->italic = italic;
    font_changed_ = true;
}

void CellImpl::set_color(const Color& color) {
    if (!pending_font_) {
        pending_font_ = Font();
    }
    pending_font_->color = color;
    font_changed_ = true;
}

void CellImpl::set_background_color(const Color& color) {
    if (!pending_fill_) {
        pending_fill_ = Fill();
    }
    pending_fill_->pattern_type = Fill::PatternType::Solid;
    pending_fill_->fg_color = color;
    fill_changed_ = true;
}

void CellImpl::set_alignment(const Alignment& alignment) {
    pending_alignment_ = alignment;
    alignment_changed_ = true;
}

void CellImpl::set_border(BorderType type, BorderStyle style) {
    if (!pending_border_) {
        pending_border_ = Border();
    }

    Border::BorderLine line;
    // 正确映射 BorderStyle 到 Border::Style
    switch (style) {
        case BorderStyle::None:
            line.style = Border::Style::None;
            break;
        case BorderStyle::Thin:
            line.style = Border::Style::Thin;
            break;
        case BorderStyle::Medium:
            line.style = Border::Style::Medium;
            break;
        case BorderStyle::Thick:
            line.style = Border::Style::Thick;
            break;
        case BorderStyle::Double:
            line.style = Border::Style::Double;
            break;
        case BorderStyle::Dotted:
            line.style = Border::Style::Dotted;
            break;
        case BorderStyle::Dashed:
            line.style = Border::Style::Dashed;
            break;
        default:
            line.style = Border::Style::Thin;
            break;
    }
    line.color = Color::Black;  // 默认黑色边框
    
    switch (type) {
        case BorderType::All:
            pending_border_->left = line;
            pending_border_->right = line;
            pending_border_->top = line;
            pending_border_->bottom = line;
            break;
        case BorderType::Left:
            pending_border_->left = line;
            break;
        case BorderType::Right:
            pending_border_->right = line;
            break;
        case BorderType::Top:
            pending_border_->top = line;
            break;
        case BorderType::Bottom:
            pending_border_->bottom = line;
            break;
        case BorderType::None:
            // 清除所有边框
            pending_border_->left.style = Border::Style::None;
            pending_border_->right.style = Border::Style::None;
            pending_border_->top.style = Border::Style::None;
            pending_border_->bottom.style = Border::Style::None;
            break;
    }
    
    border_changed_ = true;
}

void CellImpl::set_number_format(const std::string& format_code) {
    if (!pending_number_format_) {
        pending_number_format_ = NumberFormat();
    }

    // 检查是否是常用的内置格式
    std::uint32_t format_id = 0;
    if (format_code == "0.00%") {
        format_id = 10;  // Excel内置百分比格式
    } else if (format_code == "0%") {
        format_id = 9;   // Excel内置整数百分比格式
    } else if (format_code == "#,##0") {
        format_id = 3;   // Excel内置千分位格式
    } else if (format_code == "#,##0.00") {
        format_id = 4;   // Excel内置千分位小数格式
    } else if (format_code == "yyyy-mm-dd") {
        format_id = 14;  // Excel内置日期格式
    } else if (format_code == "hh:mm:ss") {
        format_id = 21;  // Excel内置时间格式
    } else {
        // 自定义格式，使用164开始的ID
        format_id = 164;
    }

    pending_number_format_->id = format_id;
    pending_number_format_->format_code = format_code;
    number_format_changed_ = true;
}

void CellImpl::apply_style_changes() {
    // 检查是否有任何样式更改
    if (font_changed_ || fill_changed_ || border_changed_ ||
        number_format_changed_ || alignment_changed_) {



        has_custom_style_ = true;

        // 开始应用样式更改

        // 如果有工作表引用，使用StyleManager动态创建样式
        if (worksheet_) {
            auto* style_manager = worksheet_->get_style_manager();


            if (style_manager) {
                // 创建完整的样式定义
                CellStyle cell_style;

                // 处理字体
                if (pending_font_) {
                    std::uint32_t font_id = style_manager->add_font(*pending_font_);
                    cell_style.font_id = font_id;
                    cell_style.apply_font = true;
                }

                // 处理填充
                if (pending_fill_) {
                    std::uint32_t fill_id = style_manager->add_fill(*pending_fill_);
                    cell_style.fill_id = fill_id;
                    cell_style.apply_fill = true;
                }

                // 处理边框
                if (pending_border_) {
                    std::uint32_t border_id = style_manager->add_border(*pending_border_);
                    cell_style.border_id = border_id;
                    cell_style.apply_border = true;
                }

                // 处理数字格式
                if (pending_number_format_) {
                    std::uint32_t format_id = style_manager->add_number_format(*pending_number_format_);
                    cell_style.number_format_id = format_id;
                    cell_style.apply_number_format = true;
                }

                // 处理对齐
                if (pending_alignment_) {
                    cell_style.alignment = *pending_alignment_;
                    cell_style.apply_alignment = true;
                }

                // 添加到StyleManager并获取样式ID
                style_id_ = style_manager->add_cell_style(cell_style);

            } else {
                // 回退到默认样式，而不是粗体样式

                style_id_ = 0;  // 使用默认样式
            }
        } else {
            // 没有工作表引用，使用默认样式

            style_id_ = 0;  // 使用默认样式
        }

        // 不要清除标志，允许增量样式更新
        // 只有在保存文件时才清除这些标志
        // font_changed_ = false;
        // fill_changed_ = false;
        // border_changed_ = false;
        // number_format_changed_ = false;
        // alignment_changed_ = false;
    }
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
    // 设置值后应用累积的样式更改（如果有的话）
    impl_->apply_style_changes();
    return *this;
}

template<>
Cell& Cell::value<int>(const int& value) {
    impl_->set_value(value);
    // 设置值后应用累积的样式更改（如果有的话）
    impl_->apply_style_changes();
    return *this;
}

template<>
Cell& Cell::value<double>(const double& value) {
    impl_->set_value(value);
    // 设置值后应用累积的样式更改（如果有的话）
    impl_->apply_style_changes();
    return *this;
}

template<>
Cell& Cell::value<bool>(const bool& value) {
    impl_->set_value(value);
    // 设置值后应用累积的样式更改（如果有的话）
    impl_->apply_style_changes();
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

// 样式操作 - 立即应用
Cell& Cell::font(const std::string& font_name, double size) {
    impl_->set_font(font_name, size);
    impl_->apply_style_changes();  // 立即应用样式
    return *this;
}

Cell& Cell::bold(bool bold) {
    impl_->set_bold(bold);
    impl_->apply_style_changes();  // 立即应用样式
    return *this;
}

Cell& Cell::italic(bool italic) {
    impl_->set_italic(italic);
    impl_->apply_style_changes();  // 立即应用样式
    return *this;
}

Cell& Cell::color(const Color& color) {
    impl_->set_color(color);
    impl_->apply_style_changes();  // 立即应用样式
    return *this;
}

Cell& Cell::background_color(const Color& color) {
    impl_->set_background_color(color);
    impl_->apply_style_changes();  // 立即应用样式
    return *this;
}

Cell& Cell::align(const Alignment& alignment) {
    impl_->set_alignment(alignment);
    impl_->apply_style_changes();  // 立即应用样式
    return *this;
}

Cell& Cell::border(BorderType border_type, BorderStyle style) {
    impl_->set_border(border_type, style);
    impl_->apply_style_changes();  // 立即应用样式
    return *this;
}

Cell& Cell::number_format(const std::string& format_code) {
    impl_->set_number_format(format_code);
    impl_->apply_style_changes();  // 立即应用样式
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

const Cell::CellValue& Cell::raw_value() const {
    return impl_->get_value();
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

void Cell::set_worksheet(Worksheet* ws) {
    impl_->set_worksheet(ws);
}

void Cell::apply_style_changes() {
    impl_->apply_style_changes();
}

// 获取样式信息（供StyleManager使用）
bool Cell::has_custom_style() const {
    return impl_->has_custom_style();
}

bool Cell::has_font_changes() const {
    return impl_->has_font_changes();
}

bool Cell::has_fill_changes() const {
    return impl_->has_fill_changes();
}

bool Cell::has_border_changes() const {
    return impl_->has_border_changes();
}

bool Cell::has_number_format_changes() const {
    return impl_->has_number_format_changes();
}

bool Cell::has_alignment_changes() const {
    return impl_->has_alignment_changes();
}

const std::optional<Font>& Cell::get_pending_font() const {
    return impl_->get_pending_font();
}

const std::optional<Fill>& Cell::get_pending_fill() const {
    return impl_->get_pending_fill();
}

const std::optional<Border>& Cell::get_pending_border() const {
    return impl_->get_pending_border();
}

const std::optional<NumberFormat>& Cell::get_pending_number_format() const {
    return impl_->get_pending_number_format();
}

const std::optional<Alignment>& Cell::get_pending_alignment() const {
    return impl_->get_pending_alignment();
}

} // namespace tinakit::excel
