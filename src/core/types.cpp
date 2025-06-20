/**
 * @file types.cpp
 * @brief TinaKit 核心类型实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/core/types.hpp"
#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace tinakit {

// Document type 实现
DocumentType get_document_type(const std::string& extension) {
    std::string ext = extension;
    if (!ext.empty() && ext[0] != '.') {
        ext = "." + ext;
    }
    
    // 转换为小写
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".xlsx" || ext == ".xlsm" || ext == ".xls") {
        return DocumentType::Excel;
    } else if (ext == ".docx" || ext == ".doc") {
        return DocumentType::Word;
    } else if (ext == ".pptx" || ext == ".ppt") {
        return DocumentType::PowerPoint;
    }
    
    return DocumentType::Unknown;
}

bool is_supported_format(const std::string& extension) {
    return get_document_type(extension) != DocumentType::Unknown;
}

} // namespace tinakit

// 核心数据结构实现
namespace tinakit::core {

// 工具函数：列号转列名
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

// 工具函数：列名转列号
std::size_t column_name_to_number(const std::string& column_name) {
    std::size_t result = 0;
    for (char c : column_name) {
        if (c < 'A' || c > 'Z') {
            throw std::invalid_argument("Invalid column name: " + column_name);
        }
        result = result * 26 + (c - 'A' + 1);
    }
    return result;
}

// Position 实现
Position Position::from_address(const std::string& address) {
    std::regex pattern("^([A-Z]+)([0-9]+)$");
    std::smatch match;

    if (!std::regex_match(address, match, pattern)) {
        throw std::invalid_argument("Invalid cell address: " + address);
    }

    std::string col_str = match[1];
    std::string row_str = match[2];

    std::size_t col = column_name_to_number(col_str);
    std::size_t row = std::stoull(row_str);

    return Position(row, col);
}

std::string Position::to_address() const {
    return column_number_to_name(column) + std::to_string(row);
}

// range_address 实现
range_address range_address::from_string(const std::string& a1_notation) {
    // 处理不同的范围格式
    auto colon_pos = a1_notation.find(':');

    if (colon_pos == std::string::npos) {
        // 单个单元格，如 "A1"
        auto pos = Position::from_address(a1_notation);
        return range_address(pos, pos);
    }

    std::string start_str = a1_notation.substr(0, colon_pos);
    std::string end_str = a1_notation.substr(colon_pos + 1);

    // 处理整行或整列的情况
    if (start_str.find_first_of("0123456789") == std::string::npos) {
        // 整列，如 "A:C"
        std::size_t start_col = column_name_to_number(start_str);
        std::size_t end_col = column_name_to_number(end_str);
        return range_address(Position(1, start_col), Position(1048576, end_col)); // Excel 最大行数
    }

    if (start_str.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == std::string::npos) {
        // 整行，如 "5:5"
        std::size_t start_row = std::stoull(start_str);
        std::size_t end_row = std::stoull(end_str);
        return range_address(Position(start_row, 1), Position(end_row, 16384)); // Excel 最大列数
    }

    // 普通范围，如 "A1:C5"
    auto start_pos = Position::from_address(start_str);
    auto end_pos = Position::from_address(end_str);
    return range_address(start_pos, end_pos);
}

range_address range_address::from_positions(const Position& start_pos, const Position& end_pos) {
    return range_address(start_pos, end_pos);
}

range_address range_address::from_single_cell(const Position& pos) {
    return range_address(pos, pos);
}

std::string range_address::to_string() const {
    if (start == end) {
        return start.to_address();
    } else {
        return start.to_address() + ":" + end.to_address();
    }
}

bool range_address::contains(const Position& pos) const {
    return pos.row >= start.row && pos.row <= end.row &&
           pos.column >= start.column && pos.column <= end.column;
}

std::pair<std::size_t, std::size_t> range_address::size() const {
    return {
        end.row - start.row + 1,
        end.column - start.column + 1
    };
}

bool range_address::overlaps(const range_address& other) const {
    return start.column <= other.end.column &&
           end.column >= other.start.column &&
           start.row <= other.end.row &&
           end.row >= other.start.row;
}

} // namespace tinakit::core