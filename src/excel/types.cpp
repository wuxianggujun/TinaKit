/**
 * @file excel/types.cpp
 * @brief Excel-specific types implementation
 * @author TinaKit Team
 * @date 2025-01-09
 */

#include "tinakit/excel/types.hpp"
#include <regex>
#include <sstream>
#include <cctype>
#include <stdexcept>

namespace tinakit::excel {

// Position 实现
Position Position::from_address(const std::string& address) {
    std::regex pattern("^([A-Z]+)([0-9]+)$");
    std::smatch match;
    
    if (!std::regex_match(address, match, pattern)) {
        throw std::invalid_argument("Invalid cell address: " + address);
    }
    
    std::string col_str = match[1];
    std::string row_str = match[2];
    
    // 转换列名为列号
    std::size_t col = 0;
    for (char c : col_str) {
        col = col * 26 + (c - 'A' + 1);
    }
    
    // 转换行号
    std::size_t row = std::stoull(row_str);
    
    return Position(row, col);
}

std::string Position::to_address() const {
    return column_number_to_name(column) + std::to_string(row);
}


// 工具函数实现
std::string column_number_to_name(std::size_t column) {
    if (column == 0) {
        throw std::invalid_argument("Column number must be 1-based");
    }
    
    std::string result;
    while (column > 0) {
        column--; // 转换为 0-based
        result = static_cast<char>('A' + column % 26) + result;
        column /= 26;
    }
    
    return result;
}

std::size_t column_name_to_number(const std::string& column_name) {
    if (column_name.empty()) {
        throw std::invalid_argument("Column name cannot be empty");
    }
    
    std::size_t result = 0;
    for (char c : column_name) {
        if (!std::isalpha(c) || !std::isupper(c)) {
            throw std::invalid_argument("Invalid column name: " + column_name);
        }
        result = result * 26 + (c - 'A' + 1);
    }
    
    return result;
}

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

// ConditionalFormatRule 实现
bool ConditionalFormatRule::operator==(const ConditionalFormatRule& other) const {
    return type == other.type &&
           operator_type == other.operator_type &&
           formulas == other.formulas &&
           text == other.text &&
           font == other.font &&
           fill == other.fill &&
           border == other.border;
}

// ConditionalFormat 实现
bool ConditionalFormat::operator==(const ConditionalFormat& other) const {
    return range == other.range &&
           rules == other.rules &&
           priority == other.priority;
}

} // namespace tinakit::excel