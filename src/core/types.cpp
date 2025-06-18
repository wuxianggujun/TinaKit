/**
 * @file types.cpp
 * @brief TinaKit 核心类型实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/core/types.hpp"
#include <stdexcept>
#include <cctype>

namespace tinakit {

// Position 实现
Position Position::from_address(const std::string& address) {
    std::size_t col = 0;
    std::size_t row = 0;
    std::size_t i = 0;
    
    // 解析列字母（A-Z, AA-ZZ, etc.）
    while (i < address.size() && std::isalpha(address[i])) {
        col = col * 26 + (std::toupper(address[i]) - 'A' + 1);
        i++;
    }
    
    // 解析行号
    while (i < address.size() && std::isdigit(address[i])) {
        row = row * 10 + (address[i] - '0');
        i++;
    }
    
    if (col == 0 || row == 0) {
        throw std::invalid_argument("Invalid cell address: " + address);
    }
    
    return Position(row, col);
}

std::string Position::to_address() const {
    return column_number_to_name(column) + std::to_string(row);
}

Position Position::after_paragraph(std::size_t paragraph_index) {
    // TODO: 实现段落定位逻辑
    return Position(paragraph_index + 1, 1);
}

// Range 实现
Range Range::from_string(const std::string& range_str) {
    auto colon_pos = range_str.find(':');
    if (colon_pos == std::string::npos) {
        // 单个单元格
        auto pos = Position::from_address(range_str);
        return Range(pos, pos);
    }
    
    auto start_str = range_str.substr(0, colon_pos);
    auto end_str = range_str.substr(colon_pos + 1);
    
    return Range(Position::from_address(start_str), 
                 Position::from_address(end_str));
}

std::string Range::to_string() const {
    if (start.row == end.row && start.column == end.column) {
        return start.to_address();
    }
    return start.to_address() + ":" + end.to_address();
}

bool Range::contains(const Position& pos) const {
    return pos.row >= start.row && pos.row <= end.row &&
           pos.column >= start.column && pos.column <= end.column;
}

std::pair<std::size_t, std::size_t> Range::size() const {
    return {end.row - start.row + 1, end.column - start.column + 1};
}

// 辅助函数实现
std::string column_number_to_name(std::size_t column) {
    if (column == 0) {
        throw std::invalid_argument("Column number must be >= 1");
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
    std::size_t result = 0;
    for (char c : column_name) {
        if (!std::isalpha(c)) {
            throw std::invalid_argument("Invalid column name: " + column_name);
        }
        result = result * 26 + (std::toupper(c) - 'A' + 1);
    }
    return result;
}

// DocumentType 相关函数
DocumentType get_document_type(const std::string& extension) {
    if (extension == ".xlsx" || extension == ".xlsm" || extension == ".xls") {
        return DocumentType::Excel;
    } else if (extension == ".docx" || extension == ".doc") {
        return DocumentType::Word;
    } else if (extension == ".pptx" || extension == ".ppt") {
        return DocumentType::PowerPoint;
    }
    return DocumentType::Unknown;
}

bool is_supported_format(const std::string& extension) {
    return get_document_type(extension) != DocumentType::Unknown;
}

// Config 实现
Config Config::default_config() {
    Config config;
    config.enable_async = true;
    config.thread_pool_size = 4;
    config.max_memory_usage = 1024 * 1024 * 1024; // 1GB
    config.enable_formula_calculation = true;
    config.temp_directory = "";
    return config;
}

} // namespace tinakit