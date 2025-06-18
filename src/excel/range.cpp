/**
 * @file range.cpp
 * @brief Excel 范围类实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/excel/range.hpp"
#include "tinakit/core/exceptions.hpp"
#include <sstream>

namespace tinakit::excel {

Range::Range(const Position& start, const Position& end) 
    : start_(start), end_(end) {
}

Range Range::from_string(const std::string& range_str) {
    // 简单的范围解析实现
    auto colon_pos = range_str.find(':');
    if (colon_pos == std::string::npos) {
        // 单个单元格
        auto pos = Position::from_address(range_str);
        return Range(pos, pos);
    } else {
        // 范围
        auto start_str = range_str.substr(0, colon_pos);
        auto end_str = range_str.substr(colon_pos + 1);
        auto start_pos = Position::from_address(start_str);
        auto end_pos = Position::from_address(end_str);
        return Range(start_pos, end_pos);
    }
}

const Position& Range::start() const noexcept {
    return start_;
}

const Position& Range::end() const noexcept {
    return end_;
}

std::string Range::to_string() const {
    if (start_ == end_) {
        return start_.to_address();
    } else {
        return start_.to_address() + ":" + end_.to_address();
    }
}

} // namespace tinakit::excel