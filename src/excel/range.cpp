/**
 * @file range.cpp
 * @brief Excel 范围句柄类实现 - 重构后的唯一用户接口
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/excel/range.hpp"
#include "tinakit/excel/style_template.hpp"
#include "tinakit/internal/workbook_impl.hpp"
#include "tinakit/internal/coordinate_utils.hpp"
#include "tinakit/core/exceptions.hpp"

namespace tinakit::excel {

// ========================================
// Range 构造函数
// ========================================

Range::Range(std::shared_ptr<internal::workbook_impl> workbook_impl,
             std::string sheet_name,
             core::range_address range_addr)
    : workbook_impl_(std::move(workbook_impl))
    , sheet_name_(std::move(sheet_name))
    , range_addr_(range_addr)
    , view_(RangeView(workbook_impl_, sheet_name_, range_addr)) {
}

// ========================================
// 范围信息
// ========================================

std::string Range::address() const {
    return internal::utils::CoordinateUtils::range_address_to_string(range_addr_);
}

std::string Range::to_string() const {
    return address(); // 向后兼容方法
}

core::Coordinate Range::start_position() const {
    return range_addr_.start;
}

core::Coordinate Range::end_position() const {
    return range_addr_.end;
}

bool Range::contains(const core::Coordinate& pos) const {
    return range_addr_.contains(pos);
}

std::pair<std::size_t, std::size_t> Range::size() const {
    return range_addr_.size();
}

bool Range::overlaps(const Range& other) const {
    return range_addr_.overlaps(other.range_addr_);
}

// ========================================
// 批量操作 - 委托给 workbook_impl
// ========================================

template<typename T>
Range& Range::set_value(const T& value) {
    // 委托给workbook_impl进行批量值设置
    workbook_impl_->set_range_values(sheet_name_, range_addr_, {{value}});
    return *this;
}

// 显式实例化常用类型
template Range& Range::set_value<std::string>(const std::string& value);
template Range& Range::set_value<int>(const int& value);
template Range& Range::set_value<double>(const double& value);
template Range& Range::set_value<bool>(const bool& value);

Range& Range::set_style(const StyleTemplate& style_template) {
    // 委托给workbook_impl进行批量样式设置
    // TODO: 实现StyleTemplate到style_id的转换
    auto style_id = 1; // 临时使用默认样式ID
    return set_style(style_id);
}

Range& Range::set_style(std::uint32_t style_id) {
    // 委托给workbook_impl进行批量样式设置
    workbook_impl_->set_range_style(sheet_name_, range_addr_, style_id);
    return *this;
}

Range& Range::clear() {
    // 委托给workbook_impl进行批量清除
    // TODO: 实现clear_range方法
    // workbook_impl_->clear_range(sheet_name_, range_addr_);
    return *this;
}

// ========================================
// 静态工厂方法实现
// ========================================

Range Range::from_string(const std::string& range_str,
                         std::shared_ptr<internal::workbook_impl> workbook_impl,
                         const std::string& sheet_name) {
    auto range_addr = internal::utils::CoordinateUtils::string_to_range_address(range_str);
    return Range(std::move(workbook_impl), sheet_name, range_addr);
}

// ========================================
// 比较运算符实现
// ========================================

bool Range::operator==(const Range& other) const {
    return range_addr_ == other.range_addr_ &&
           sheet_name_ == other.sheet_name_;
}

bool Range::operator!=(const Range& other) const {
    return !(*this == other);
}

} // namespace tinakit::excel