/**
 * @file types.cpp
 * @brief TinaKit 核心类型实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/core/types.hpp"
#include "tinakit/internal/coordinate_utils.hpp"
#include "tinakit/core/exceptions.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>

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

// Coordinate 实现 - 委托给 CoordinateUtils
Coordinate Coordinate::from_address(const std::string& address) {
    return internal::utils::CoordinateUtils::string_to_coordinate(address);
}

std::string Coordinate::to_address() const {
    return internal::utils::CoordinateUtils::coordinate_to_string(*this);
}

// range_address 实现 - 委托给 CoordinateUtils
range_address range_address::from_string(const std::string& a1_notation) {
    return internal::utils::CoordinateUtils::string_to_range_address(a1_notation);
}

range_address range_address::from_positions(const Coordinate& start_pos, const Coordinate& end_pos) {
    return range_address(start_pos, end_pos);
}

range_address range_address::from_single_cell(const Coordinate& pos) {
    return range_address(pos, pos);
}

std::string range_address::to_string() const {
    return internal::utils::CoordinateUtils::range_address_to_string(*this);
}

bool range_address::contains(const Coordinate& pos) const {
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

// ========================================
// 坐标转换工具函数实现（委托给CoordinateUtils）
// ========================================

std::string column_number_to_name(std::size_t column) {
    return internal::utils::CoordinateUtils::column_number_to_letters(column);
}

std::size_t column_name_to_number(const std::string& column_name) {
    return internal::utils::CoordinateUtils::column_letters_to_number(column_name);
}

} // namespace tinakit::core