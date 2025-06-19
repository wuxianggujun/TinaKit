/**
 * @file cell.cpp
 * @brief Excel 单元格类实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/excel/cell.hpp"
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
    
    // 设置工作表引用（用于访问样式管理器）
    void set_worksheet(Worksheet* ws) { worksheet_ = ws; }
    
    // 应用累积的样式更改
    void apply_style_changes();

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
    line.style = static_cast<Border::Style>(static_cast<int>(style));
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
    // TODO: 生成合适的 format ID
    pending_number_format_->id = 164;  // 自定义格式的起始 ID
    pending_number_format_->format_code = format_code;
    number_format_changed_ = true;
}

void CellImpl::apply_style_changes() {
    if (!worksheet_) {
        // 如果没有工作表引用，无法应用样式
        return;
    }
    
    // 如果有任何样式更改，创建新的样式
    if (font_changed_ || fill_changed_ || border_changed_ || 
        number_format_changed_ || alignment_changed_) {
        
        // TODO: 从工作簿获取样式管理器
        // auto& style_manager = worksheet_->workbook().style_manager();
        
        // 暂时注释掉，等待 Worksheet 类实现相关方法
        /*
        CellStyle new_style;
        
        // 设置字体
        if (font_changed_ && pending_font_) {
            new_style.font_id = style_manager.add_font(*pending_font_);
            new_style.apply_font = true;
        }
        
        // 设置填充
        if (fill_changed_ && pending_fill_) {
            new_style.fill_id = style_manager.add_fill(*pending_fill_);
            new_style.apply_fill = true;
        }
        
        // 设置边框
        if (border_changed_ && pending_border_) {
            new_style.border_id = style_manager.add_border(*pending_border_);
            new_style.apply_border = true;
        }
        
        // 设置数字格式
        if (number_format_changed_ && pending_number_format_) {
            new_style.number_format_id = style_manager.add_number_format(*pending_number_format_);
            new_style.apply_number_format = true;
        }
        
        // 设置对齐
        if (alignment_changed_ && pending_alignment_) {
            new_style.alignment = pending_alignment_;
            new_style.apply_alignment = true;
        }
        
        // 创建新样式并获取 ID
        style_id_ = style_manager.add_cell_style(new_style);
        has_custom_style_ = true;
        */
        
        // 清除标志
        font_changed_ = false;
        fill_changed_ = false;
        border_changed_ = false;
        number_format_changed_ = false;
        alignment_changed_ = false;
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

Cell& Cell::align(const Alignment& alignment) {
    impl_->set_alignment(alignment);
    return *this;
}

Cell& Cell::border(BorderType border_type, BorderStyle style) {
    impl_->set_border(border_type, style);
    return *this;
}

Cell& Cell::number_format(const std::string& format_code) {
    impl_->set_number_format(format_code);
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

} // namespace tinakit::excel 