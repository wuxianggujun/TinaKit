/**
 * @file style_manager.cpp
 * @brief Excel 样式管理器实现
 * @author TinaKit Team
 * @date 2025-01-08
 */

#include "tinakit/excel/style_manager.hpp"
#include "tinakit/core/xml_parser.hpp"
#include <sstream>
#include <stdexcept>
#include <functional>

namespace tinakit::excel {

// Font 实现
bool Font::operator==(const Font& other) const {
    return name == other.name &&
           size == other.size &&
           bold == other.bold &&
           italic == other.italic &&
           underline == other.underline &&
           strike == other.strike &&
           color == other.color;
}

// Fill 实现
bool Fill::operator==(const Fill& other) const {
    return pattern_type == other.pattern_type &&
           fg_color == other.fg_color &&
           bg_color == other.bg_color;
}

// BorderLine 实现
bool Border::BorderLine::operator==(const BorderLine& other) const {
    return style == other.style && color == other.color;
}

// Border 实现
bool Border::operator==(const Border& other) const {
    return left == other.left &&
           right == other.right &&
           top == other.top &&
           bottom == other.bottom &&
           diagonal == other.diagonal &&
           diagonal_up == other.diagonal_up &&
           diagonal_down == other.diagonal_down;
}

// Alignment 实现
bool Alignment::operator==(const Alignment& other) const {
    return horizontal == other.horizontal &&
           vertical == other.vertical &&
           text_rotation == other.text_rotation &&
           wrap_text == other.wrap_text &&
           shrink_to_fit == other.shrink_to_fit &&
           indent == other.indent;
}

// NumberFormat 实现
bool NumberFormat::operator==(const NumberFormat& other) const {
    return id == other.id && format_code == other.format_code;
}

// CellStyle 实现
bool CellStyle::operator==(const CellStyle& other) const {
    return font_id == other.font_id &&
           fill_id == other.fill_id &&
           border_id == other.border_id &&
           number_format_id == other.number_format_id &&
           alignment == other.alignment &&
           apply_font == other.apply_font &&
           apply_fill == other.apply_fill &&
           apply_border == other.apply_border &&
           apply_number_format == other.apply_number_format &&
           apply_alignment == other.apply_alignment;
}

// StyleManager 实现
StyleManager::StyleManager() {
    initialize_defaults();
}

void StyleManager::initialize_defaults() {
    // 清空所有内容
    clear();
    
    // 添加默认字体 (索引 0)
    Font default_font;
    add_font(default_font);
    
    // 添加默认填充 (索引 0 和 1)
    Fill none_fill;
    none_fill.pattern_type = Fill::PatternType::None;
    add_fill(none_fill);
    
    Fill gray125_fill;
    gray125_fill.pattern_type = Fill::PatternType::Gray125;
    add_fill(gray125_fill);
    
    // 添加默认边框 (索引 0)
    Border default_border;
    add_border(default_border);
    
    // 添加默认单元格样式 (索引 0)
    CellStyle default_style;
    default_style.font_id = 0;
    default_style.fill_id = 0;
    default_style.border_id = 0;
    default_style.number_format_id = 0;
    add_cell_style(default_style);
}

std::uint32_t StyleManager::add_font(const Font& font) {
    // 计算哈希值
    std::size_t hash = hash_font(font);
    
    // 检查是否已存在
    auto it = font_cache_.find(hash);
    if (it != font_cache_.end()) {
        // 验证是否真的相同
        if (fonts_[it->second] == font) {
            return it->second;
        }
    }
    
    // 添加新字体
    std::uint32_t id = static_cast<std::uint32_t>(fonts_.size());
    fonts_.push_back(font);
    font_cache_[hash] = id;
    
    return id;
}

const Font& StyleManager::get_font(std::uint32_t id) const {
    if (id >= fonts_.size()) {
        throw std::out_of_range("Font ID out of range: " + std::to_string(id));
    }
    return fonts_[id];
}

std::uint32_t StyleManager::add_fill(const Fill& fill) {
    std::size_t hash = hash_fill(fill);
    
    auto it = fill_cache_.find(hash);
    if (it != fill_cache_.end()) {
        if (fills_[it->second] == fill) {
            return it->second;
        }
    }
    
    std::uint32_t id = static_cast<std::uint32_t>(fills_.size());
    fills_.push_back(fill);
    fill_cache_[hash] = id;
    
    return id;
}

const Fill& StyleManager::get_fill(std::uint32_t id) const {
    if (id >= fills_.size()) {
        throw std::out_of_range("Fill ID out of range: " + std::to_string(id));
    }
    return fills_[id];
}

std::uint32_t StyleManager::add_border(const Border& border) {
    std::size_t hash = hash_border(border);
    
    auto it = border_cache_.find(hash);
    if (it != border_cache_.end()) {
        if (borders_[it->second] == border) {
            return it->second;
        }
    }
    
    std::uint32_t id = static_cast<std::uint32_t>(borders_.size());
    borders_.push_back(border);
    border_cache_[hash] = id;
    
    return id;
}

const Border& StyleManager::get_border(std::uint32_t id) const {
    if (id >= borders_.size()) {
        throw std::out_of_range("Border ID out of range: " + std::to_string(id));
    }
    return borders_[id];
}

std::uint32_t StyleManager::add_number_format(const NumberFormat& format) {
    auto it = number_format_cache_.find(format.format_code);
    if (it != number_format_cache_.end()) {
        return it->second;
    }
    
    std::uint32_t id = static_cast<std::uint32_t>(number_formats_.size());
    number_formats_.push_back(format);
    number_format_cache_[format.format_code] = id;
    
    return id;
}

const NumberFormat& StyleManager::get_number_format(std::uint32_t id) const {
    if (id >= number_formats_.size()) {
        throw std::out_of_range("Number format ID out of range: " + std::to_string(id));
    }
    return number_formats_[id];
}

std::uint32_t StyleManager::add_cell_style(const CellStyle& style) {
    // 对于单元格样式，我们不去重，因为它们可能有细微差别
    std::uint32_t id = static_cast<std::uint32_t>(cell_styles_.size());
    cell_styles_.push_back(style);
    return id;
}

const CellStyle& StyleManager::get_cell_style(std::uint32_t id) const {
    if (id >= cell_styles_.size()) {
        throw std::out_of_range("Cell style ID out of range: " + std::to_string(id));
    }
    return cell_styles_[id];
}

void StyleManager::clear() {
    fonts_.clear();
    fills_.clear();
    borders_.clear();
    number_formats_.clear();
    cell_styles_.clear();
    
    font_cache_.clear();
    fill_cache_.clear();
    border_cache_.clear();
    number_format_cache_.clear();
}

std::string StyleManager::generate_xml() const {
    std::ostringstream xml;
    
    // XML 声明
    xml << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)" << '\n';
    
    // 样式表根元素
    xml << R"(<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">)" << '\n';
    
    // 数字格式
    if (!number_formats_.empty()) {
        xml << R"(  <numFmts count=")" << number_formats_.size() << R"(">)" << '\n';
        for (const auto& fmt : number_formats_) {
            xml << R"(    <numFmt numFmtId=")" << fmt.id << R"(" formatCode=")" << fmt.format_code << R"("/>)" << '\n';
        }
        xml << "  </numFmts>\n";
    }
    
    // 字体
    xml << R"(  <fonts count=")" << fonts_.size() << R"(">)" << '\n';
    for (const auto& font : fonts_) {
        xml << "    <font>\n";
        if (font.bold) xml << "      <b/>\n";
        if (font.italic) xml << "      <i/>\n";
        if (font.underline) xml << "      <u/>\n";
        if (font.strike) xml << "      <strike/>\n";
        xml << R"(      <sz val=")" << font.size << R"("/>)" << '\n';
        if (font.color) {
            xml << R"(      <color rgb=")" << font.color->to_hex() << R"("/>)" << '\n';
        }
        xml << R"(      <name val=")" << font.name << R"("/>)" << '\n';
        xml << "    </font>\n";
    }
    xml << "  </fonts>\n";
    
    // 填充
    xml << R"(  <fills count=")" << fills_.size() << R"(">)" << '\n';
    for (const auto& fill : fills_) {
        xml << "    <fill>\n";
        xml << "      <patternFill patternType=\"";
        
        // 转换模式类型为字符串
        switch (fill.pattern_type) {
            case Fill::PatternType::None: xml << "none"; break;
            case Fill::PatternType::Solid: xml << "solid"; break;
            case Fill::PatternType::Gray125: xml << "gray125"; break;
            // ... 其他模式类型
            default: xml << "none"; break;
        }
        
        xml << "\"";
        
        if (fill.fg_color) {
            xml << ">\n";
            xml << R"(        <fgColor rgb=")" << fill.fg_color->to_hex() << R"("/>)" << '\n';
            if (fill.bg_color) {
                xml << R"(        <bgColor rgb=")" << fill.bg_color->to_hex() << R"("/>)" << '\n';
            }
            xml << "      </patternFill>\n";
        } else {
            xml << "/>\n";
        }
        
        xml << "    </fill>\n";
    }
    xml << "  </fills>\n";
    
    // 边框
    xml << R"(  <borders count=")" << borders_.size() << R"(">)" << '\n';
    for (const auto& border : borders_) {
        xml << "    <border>\n";
        
        // 生成各边框线
        auto generate_border_line = [&xml](const char* name, const Border::BorderLine& line) {
            if (line.style != Border::Style::None) {
                xml << "      <" << name << " style=\"";
                switch (line.style) {
                    case Border::Style::Thin: xml << "thin"; break;
                    case Border::Style::Medium: xml << "medium"; break;
                    case Border::Style::Thick: xml << "thick"; break;
                    // ... 其他样式
                    default: xml << "thin"; break;
                }
                xml << "\"";
                
                if (line.color) {
                    xml << ">\n";
                    xml << R"(        <color rgb=")" << line.color->to_hex() << R"("/>)" << '\n';
                    xml << "      </" << name << ">\n";
                } else {
                    xml << "/>\n";
                }
            }
        };
        
        generate_border_line("left", border.left);
        generate_border_line("right", border.right);
        generate_border_line("top", border.top);
        generate_border_line("bottom", border.bottom);
        generate_border_line("diagonal", border.diagonal);
        
        xml << "    </border>\n";
    }
    xml << "  </borders>\n";
    
    // 单元格样式格式（XF）
    xml << R"(  <cellXfs count=")" << cell_styles_.size() << R"(">)" << '\n';
    for (const auto& style : cell_styles_) {
        xml << "    <xf";
        
        if (style.number_format_id) {
            xml << R"( numFmtId=")" << *style.number_format_id << R"(" applyNumberFormat="1")";
        }
        
        if (style.font_id) {
            xml << R"( fontId=")" << *style.font_id << R"(" applyFont="1")";
        }
        
        if (style.fill_id) {
            xml << R"( fillId=")" << *style.fill_id << R"(" applyFill="1")";
        }
        
        if (style.border_id) {
            xml << R"( borderId=")" << *style.border_id << R"(" applyBorder="1")";
        }
        
        if (style.alignment) {
            xml << R"( applyAlignment="1">";
            xml << "\n      <alignment";
            
            // 水平对齐
            switch (style.alignment->horizontal) {
                case Alignment::Horizontal::Left: xml << R"( horizontal="left")"; break;
                case Alignment::Horizontal::Center: xml << R"( horizontal="center")"; break;
                case Alignment::Horizontal::Right: xml << R"( horizontal="right")"; break;
                // ... 其他对齐方式
                default: break;
            }
            
            // 垂直对齐
            switch (style.alignment->vertical) {
                case Alignment::Vertical::Top: xml << R"( vertical="top")"; break;
                case Alignment::Vertical::Center: xml << R"( vertical="center")"; break;
                case Alignment::Vertical::Bottom: xml << R"( vertical="bottom")"; break;
                // ... 其他对齐方式
                default: break;
            }
            
            if (style.alignment->wrap_text) {
                xml << R"( wrapText="1")";
            }
            
            xml << "/>\n    </xf>\n";
        } else {
            xml << "/>\n";
        }
    }
    xml << "  </cellXfs>\n";
    
    xml << "</styleSheet>\n";
    
    return xml.str();
}

void StyleManager::load_from_xml(const std::string& xml_data) {
    // TODO: 实现从 XML 加载样式
    // 这里需要解析 styles.xml 文件的内容
}

std::size_t StyleManager::hash_font(const Font& font) const {
    std::size_t h = 0;
    h ^= std::hash<std::string>{}(font.name);
    h ^= std::hash<double>{}(font.size);
    h ^= std::hash<bool>{}(font.bold);
    h ^= std::hash<bool>{}(font.italic);
    h ^= std::hash<bool>{}(font.underline);
    h ^= std::hash<bool>{}(font.strike);
    if (font.color) {
        h ^= std::hash<std::uint32_t>{}(font.color->to_rgba());
    }
    return h;
}

std::size_t StyleManager::hash_fill(const Fill& fill) const {
    std::size_t h = 0;
    h ^= std::hash<int>{}(static_cast<int>(fill.pattern_type));
    if (fill.fg_color) {
        h ^= std::hash<std::uint32_t>{}(fill.fg_color->to_rgba());
    }
    if (fill.bg_color) {
        h ^= std::hash<std::uint32_t>{}(fill.bg_color->to_rgba());
    }
    return h;
}

std::size_t StyleManager::hash_border(const Border& border) const {
    std::size_t h = 0;
    
    auto hash_border_line = [](const Border::BorderLine& line) {
        std::size_t lh = std::hash<int>{}(static_cast<int>(line.style));
        if (line.color) {
            lh ^= std::hash<std::uint32_t>{}(line.color->to_rgba());
        }
        return lh;
    };
    
    h ^= hash_border_line(border.left);
    h ^= hash_border_line(border.right) << 1;
    h ^= hash_border_line(border.top) << 2;
    h ^= hash_border_line(border.bottom) << 3;
    h ^= hash_border_line(border.diagonal) << 4;
    h ^= std::hash<bool>{}(border.diagonal_up) << 5;
    h ^= std::hash<bool>{}(border.diagonal_down) << 6;
    
    return h;
}

} // namespace tinakit::excel 