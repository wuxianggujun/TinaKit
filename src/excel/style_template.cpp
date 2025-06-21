/**
 * @file style_template.cpp
 * @brief Excel 样式模板系统实现
 * @author TinaKit Team
 * @date 2025-01-19
 */

#include "tinakit/excel/style.hpp"
#include "tinakit/excel/style_manager.hpp"
#include "tinakit/excel/cell.hpp"

namespace tinakit::excel {

// ========================================
// Style::Impl 实现
// ========================================

class Style::Impl {
public:
    // 字体相关
    std::optional<Font> font_;
    
    // 填充相关
    std::optional<Fill> fill_;
    
    // 边框相关
    std::optional<Border> border_;
    
    // 对齐相关
    std::optional<Alignment> alignment_;
    
    // 数字格式相关
    std::optional<std::string> number_format_;
    
    // 标记哪些属性被设置了
    bool has_font = false;
    bool has_fill = false;
    bool has_border = false;
    bool has_alignment = false;
    bool has_number_format = false;
    
    Impl() = default;
    
    Impl(const Impl& other) 
        : font_(other.font_)
        , fill_(other.fill_)
        , border_(other.border_)
        , alignment_(other.alignment_)
        , number_format_(other.number_format_)
        , has_font(other.has_font)
        , has_fill(other.has_fill)
        , has_border(other.has_border)
        , has_alignment(other.has_alignment)
        , has_number_format(other.has_number_format) {
    }
    
    void ensure_font() {
        if (!font_) {
            font_ = Font();
        }
        has_font = true;
    }
    
    void ensure_fill() {
        if (!fill_) {
            fill_ = Fill();
            fill_->pattern_type = Fill::PatternType::Solid;
        }
        has_fill = true;
    }
    
    void ensure_border() {
        if (!border_) {
            border_ = Border();
        }
        has_border = true;
    }
    
    void ensure_alignment() {
        if (!alignment_) {
            alignment_ = Alignment();
        }
        has_alignment = true;
    }
};

// ========================================
// Style 实现
// ========================================

Style::Style() 
    : impl_(std::make_unique<Impl>()) {
}

Style::Style(const Style& other) 
    : impl_(std::make_unique<Impl>(*other.impl_)) {
}

Style::Style(Style&& other) noexcept 
    : impl_(std::move(other.impl_)) {
}

Style& Style::operator=(const Style& other) {
    if (this != &other) {
        impl_ = std::make_unique<Impl>(*other.impl_);
    }
    return *this;
}

Style& Style::operator=(Style&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

Style::~Style() = default;

// ========================================
// 字体设置
// ========================================

Style& Style::font(const std::string& font_name, double size) {
    impl_->ensure_font();
    impl_->font_->name = font_name;
    impl_->font_->size = size;
    return *this;
}

Style& Style::bold(bool bold) {
    impl_->ensure_font();
    impl_->font_->bold = bold;
    return *this;
}

Style& Style::italic(bool italic) {
    impl_->ensure_font();
    impl_->font_->italic = italic;
    return *this;
}

Style& Style::underline(bool underline) {
    impl_->ensure_font();
    impl_->font_->underline = underline;
    return *this;
}

Style& Style::strike(bool strike) {
    impl_->ensure_font();
    impl_->font_->strike = strike;
    return *this;
}

// ========================================
// 颜色设置
// ========================================

Style& Style::color(const Color& color) {
    impl_->ensure_font();
    impl_->font_->color = color;
    return *this;
}

Style& Style::background_color(const Color& color) {
    impl_->ensure_fill();
    impl_->fill_->fg_color = color;
    return *this;
}

// ========================================
// 对齐设置
// ========================================

Style& Style::align(const Alignment& alignment) {
    impl_->ensure_alignment();
    impl_->alignment_ = alignment;
    return *this;
}

Style& Style::align_horizontal(Alignment::Horizontal horizontal) {
    impl_->ensure_alignment();
    impl_->alignment_->horizontal = horizontal;
    return *this;
}

Style& Style::align_vertical(Alignment::Vertical vertical) {
    impl_->ensure_alignment();
    impl_->alignment_->vertical = vertical;
    return *this;
}

// ========================================
// 边框设置
// ========================================

Style& Style::border(BorderType type, BorderStyle style) {
    impl_->ensure_border();
    
    Border::Style border_style = static_cast<Border::Style>(static_cast<int>(style));
    
    switch (type) {
        case BorderType::All:
            impl_->border_->left.style = border_style;
            impl_->border_->right.style = border_style;
            impl_->border_->top.style = border_style;
            impl_->border_->bottom.style = border_style;
            break;
        case BorderType::Top:
            impl_->border_->top.style = border_style;
            break;
        case BorderType::Bottom:
            impl_->border_->bottom.style = border_style;
            break;
        case BorderType::Left:
            impl_->border_->left.style = border_style;
            break;
        case BorderType::Right:
            impl_->border_->right.style = border_style;
            break;
    }
    
    return *this;
}

Style& Style::border(BorderType type, BorderStyle style, const Color& color) {
    border(type, style);
    
    // 设置边框颜色
    switch (type) {
        case BorderType::All:
            impl_->border_->left.color = color;
            impl_->border_->right.color = color;
            impl_->border_->top.color = color;
            impl_->border_->bottom.color = color;
            break;
        case BorderType::Top:
            impl_->border_->top.color = color;
            break;
        case BorderType::Bottom:
            impl_->border_->bottom.color = color;
            break;
        case BorderType::Left:
            impl_->border_->left.color = color;
            break;
        case BorderType::Right:
            impl_->border_->right.color = color;
            break;
    }
    
    return *this;
}

// ========================================
// 数字格式设置
// ========================================

Style& Style::number_format(const std::string& format_code) {
    impl_->number_format_ = format_code;
    impl_->has_number_format = true;
    return *this;
}

// ========================================
// 文本格式设置
// ========================================

Style& Style::wrap_text(bool wrap) {
    impl_->ensure_alignment();
    impl_->alignment_->wrap_text = wrap;
    return *this;
}

Style& Style::indent(int indent_level) {
    impl_->ensure_alignment();
    impl_->alignment_->indent = std::max(0, std::min(15, indent_level));
    return *this;
}

// ========================================
// 内部接口
// ========================================

std::uint32_t Style::apply_to_style_manager(StyleManager& style_manager) const {
    CellStyle cell_style;
    
    // 处理字体
    if (impl_->has_font && impl_->font_) {
        cell_style.font_id = style_manager.add_font(*impl_->font_);
        cell_style.apply_font = true;
    }
    
    // 处理填充
    if (impl_->has_fill && impl_->fill_) {
        cell_style.fill_id = style_manager.add_fill(*impl_->fill_);
        cell_style.apply_fill = true;
    }
    
    // 处理边框
    if (impl_->has_border && impl_->border_) {
        cell_style.border_id = style_manager.add_border(*impl_->border_);
        cell_style.apply_border = true;
    }
    
    // 处理对齐
    if (impl_->has_alignment && impl_->alignment_) {
        cell_style.alignment = *impl_->alignment_;
        cell_style.apply_alignment = true;
    }
    
    // 处理数字格式
    if (impl_->has_number_format && impl_->number_format_) {
        NumberFormat num_format;
        num_format.format_code = *impl_->number_format_;
        cell_style.number_format_id = style_manager.add_number_format(num_format);
        cell_style.apply_number_format = true;
    }
    
    return style_manager.add_cell_style(cell_style);
}

bool Style::has_any_style() const {
    return impl_->has_font || impl_->has_fill || impl_->has_border ||
           impl_->has_alignment || impl_->has_number_format;
}

// ========================================
// 预定义样式模板
// ========================================

namespace StyleTemplates {

Style title(double size) {
    return Style()
        .font("微软雅黑", size)
        .bold()
        .color(Color::White)
        .background_color(Color::Blue)
        .align_horizontal(Alignment::Horizontal::Center)
        .align_vertical(Alignment::Vertical::Center);
}

Style subtitle(double size) {
    return Style()
        .font("微软雅黑", size)
        .bold()
        .color(Color::Black)
        .background_color(Color::LightGray)
        .align_horizontal(Alignment::Horizontal::Center)
        .align_vertical(Alignment::Vertical::Center);
}

Style header() {
    return Style()
        .font("Calibri", 11)
        .bold()
        .color(Color::Black)
        .background_color(Color::LightGray)
        .align_horizontal(Alignment::Horizontal::Center)
        .align_vertical(Alignment::Vertical::Center)
        .border(BorderType::All, BorderStyle::Thin);
}

Style data() {
    return Style()
        .font("Calibri", 11)
        .color(Color::Black)
        .align_vertical(Alignment::Vertical::Center)
        .border(BorderType::All, BorderStyle::Thin);
}

Style highlight(const Color& color) {
    return Style()
        .background_color(color)
        .bold();
}

Style warning() {
    return Style()
        .color(Color::Black)
        .background_color(Color::Yellow)
        .bold();
}

Style error() {
    return Style()
        .color(Color::White)
        .background_color(Color::Red)
        .bold();
}

Style success() {
    return Style()
        .color(Color::White)
        .background_color(Color::Green)
        .bold();
}

} // namespace StyleTemplates

} // namespace tinakit::excel
