/**
 * @file coordinate_utils.cpp
 * @brief 坐标转换工具实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/internal/coordinate_utils.hpp"
#include <stdexcept>
#include <regex>
#include <algorithm>
#include <cctype>

namespace tinakit::internal::utils {

// ========================================
// 单个坐标转换实现
// ========================================

core::Coordinate CoordinateUtils::string_to_coordinate(std::string_view str) {
    if (str.empty()) {
        throw std::invalid_argument("Empty coordinate string");
    }
    
    auto [col_part, row_part] = split_coordinate_string(str);
    
    if (col_part.empty() || row_part.empty()) {
        throw std::invalid_argument("Invalid coordinate format: " + std::string(str));
    }
    
    // 转换列字母为数字
    std::size_t column = column_letters_to_number(col_part);
    
    // 转换行号
    std::size_t row = 0;
    try {
        row = std::stoull(std::string(row_part));
    } catch (const std::exception&) {
        throw std::invalid_argument("Invalid row number in coordinate: " + std::string(str));
    }
    
    if (row == 0) {
        throw std::invalid_argument("Row number must be greater than 0");
    }
    
    return core::Coordinate(row, column);
}

std::string CoordinateUtils::coordinate_to_string(const core::Coordinate& coord) {
    if (!coord.is_valid()) {
        throw std::invalid_argument("Invalid coordinate: row and column must be greater than 0");
    }
    
    std::string col_letters = column_number_to_letters(coord.column);
    return col_letters + std::to_string(coord.row);
}

// ========================================
// 范围地址转换实现
// ========================================

core::range_address CoordinateUtils::string_to_range_address(std::string_view str) {
    if (str.empty()) {
        throw std::invalid_argument("Empty range string");
    }
    
    auto [start_part, end_part] = split_range_string(str);
    
    if (start_part.empty()) {
        throw std::invalid_argument("Invalid range format: " + std::string(str));
    }
    
    core::Coordinate start = string_to_coordinate(start_part);
    core::Coordinate end = end_part.empty() ? start : string_to_coordinate(end_part);
    
    return core::range_address(start, end);
}

std::string CoordinateUtils::range_address_to_string(const core::range_address& addr) {
    if (!addr.start.is_valid() || !addr.end.is_valid()) {
        throw std::invalid_argument("Invalid range address");
    }
    
    std::string start_str = coordinate_to_string(addr.start);
    
    if (addr.start == addr.end) {
        return start_str;  // 单个单元格
    }
    
    std::string end_str = coordinate_to_string(addr.end);
    return start_str + ":" + end_str;
}

// ========================================
// 列号转换辅助函数实现
// ========================================

std::size_t CoordinateUtils::column_letters_to_number(std::string_view column_letters) {
    if (column_letters.empty()) {
        throw std::invalid_argument("Empty column letters");
    }
    
    std::size_t result = 0;
    for (char c : column_letters) {
        if (c < 'A' || c > 'Z') {
            throw std::invalid_argument("Invalid column letter: " + std::string(1, c));
        }
        result = result * 26 + (c - 'A' + 1);
    }
    
    return result;
}

std::string CoordinateUtils::column_number_to_letters(std::size_t column_number) {
    if (column_number == 0) {
        throw std::invalid_argument("Column number must be greater than 0");
    }
    
    std::string result;
    while (column_number > 0) {
        column_number--;  // 转换为0-based
        result = char('A' + (column_number % 26)) + result;
        column_number /= 26;
    }
    
    return result;
}

// ========================================
// 验证函数实现
// ========================================

bool CoordinateUtils::is_valid_coordinate_string(std::string_view str) {
    if (str.empty()) {
        return false;
    }
    
    try {
        string_to_coordinate(str);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool CoordinateUtils::is_valid_range_string(std::string_view str) {
    if (str.empty()) {
        return false;
    }
    
    try {
        string_to_range_address(str);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool CoordinateUtils::is_valid_column_letters(std::string_view letters) {
    if (letters.empty()) {
        return false;
    }
    
    return std::all_of(letters.begin(), letters.end(), [](char c) {
        return c >= 'A' && c <= 'Z';
    });
}

// ========================================
// 私有辅助函数实现
// ========================================

std::pair<std::string_view, std::string_view> 
CoordinateUtils::split_coordinate_string(std::string_view str) {
    // 找到第一个数字的位置
    auto digit_pos = std::find_if(str.begin(), str.end(), [](char c) {
        return std::isdigit(c);
    });
    
    if (digit_pos == str.end()) {
        return {str, {}};  // 没有找到数字
    }
    
    std::size_t pos = std::distance(str.begin(), digit_pos);
    return {str.substr(0, pos), str.substr(pos)};
}

std::pair<std::string_view, std::string_view> 
CoordinateUtils::split_range_string(std::string_view str) {
    std::size_t colon_pos = str.find(':');
    
    if (colon_pos == std::string_view::npos) {
        return {str, {}};  // 单个单元格
    }
    
    return {str.substr(0, colon_pos), str.substr(colon_pos + 1)};
}

} // namespace tinakit::internal::utils
