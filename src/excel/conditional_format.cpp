/**
 * @file conditional_format.cpp
 * @brief 条件格式构建器类实现
 * @author TinaKit Team
 * @date 2025-6-19
 */

#include "tinakit/excel/conditional_format.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/style_manager.hpp"
#include <vector>
#include <algorithm>
#include <iostream>

namespace tinakit::excel {

// 辅助函数：格式化数字为字符串，去掉不必要的小数点
std::string format_number_for_condition(double value) {
    // 检查是否为整数
    if (value == std::floor(value)) {
        return std::to_string(static_cast<long long>(value));
    } else {
        return std::to_string(value);
    }
}

// =============================================================================
// ConditionalFormatBuilder::Impl
// =============================================================================

class ConditionalFormatBuilder::Impl {
public:
    Impl(Worksheet& worksheet, const std::string& range_str)
        : worksheet_(worksheet), range_str_(range_str) {
        format_.range = range_str;
        format_.priority = 1;
    }
    
    void set_condition(ConditionalFormatType type, ConditionalFormatOperator op,
                      const std::vector<std::string>& formulas, const std::string& text = "") {
        ConditionalFormatRule rule;
        rule.type = type;
        rule.operator_type = op;
        rule.formulas = formulas;
        rule.text = text;

        // 替换现有规则或添加新规则
        if (!format_.rules.empty()) {
            format_.rules[0] = rule;
        } else {
            format_.rules.push_back(rule);
        }
        current_rule_ = &format_.rules.back();
    }

    // 辅助函数：从范围字符串获取左上角单元格
    std::string get_top_left_cell(const std::string& range_str) {
        size_t colon_pos = range_str.find(':');
        if (colon_pos != std::string::npos) {
            return range_str.substr(0, colon_pos);  // 返回冒号前的部分，如 "A5:A14" -> "A5"
        }
        return range_str;  // 如果没有冒号，整个字符串就是单元格引用
    }
    
    void apply_format() {
        if (!format_.rules.empty()) {
            // 为每个规则生成dxfId
            for (auto& rule : format_.rules) {
                if (rule.font || rule.fill) {
                    // 将格式添加到StyleManager并获取dxfId
                    std::uint32_t dxf_id = worksheet_.style_manager().add_dxf(
                        rule.font ? &(*rule.font) : nullptr,
                        rule.fill ? &(*rule.fill) : nullptr
                    );
                    rule.dxf_id = dxf_id;

                    // 清除内联格式，因为现在使用dxfId引用
                    rule.font.reset();
                    rule.fill.reset();
                }
            }

            // 将条件格式添加到工作表
            worksheet_.add_conditional_format(format_);
        }
    }
    
    Worksheet& worksheet_;
    std::string range_str_;
    ConditionalFormat format_;
    ConditionalFormatRule* current_rule_ = nullptr;
};

// =============================================================================
// ConditionalFormatBuilder 实现
// =============================================================================

ConditionalFormatBuilder::ConditionalFormatBuilder(Worksheet& worksheet, const std::string& range_str)
    : impl_(std::make_unique<Impl>(worksheet, range_str)) {
}

ConditionalFormatBuilder::~ConditionalFormatBuilder() = default;

ConditionalFormatBuilder& ConditionalFormatBuilder::when_greater_than(int value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::GreaterThan,
                        {std::to_string(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_greater_than(double value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::GreaterThan,
                        {format_number_for_condition(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_greater_than_or_equal(int value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::GreaterThanOrEqual,
                        {std::to_string(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_greater_than_or_equal(double value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::GreaterThanOrEqual,
                        {format_number_for_condition(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_less_than(int value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::LessThan,
                        {std::to_string(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_less_than(double value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::LessThan,
                        {format_number_for_condition(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_less_than_or_equal(int value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::LessThanOrEqual,
                        {std::to_string(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_less_than_or_equal(double value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::LessThanOrEqual,
                        {format_number_for_condition(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_equal(int value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::Equal,
                        {std::to_string(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_equal(double value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::Equal,
                        {format_number_for_condition(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_not_equal(int value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::NotEqual,
                        {std::to_string(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_not_equal(double value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::NotEqual,
                        {format_number_for_condition(value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_between(int min_value, int max_value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::Between,
                        {std::to_string(min_value), std::to_string(max_value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_between(double min_value, double max_value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::Between,
                        {format_number_for_condition(min_value), format_number_for_condition(max_value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_not_between(double min_value, double max_value) {
    impl_->set_condition(ConditionalFormatType::CellValue, ConditionalFormatOperator::NotBetween,
                        {format_number_for_condition(min_value), format_number_for_condition(max_value)});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_contains(const std::string& text) {
    // 获取范围的左上角单元格
    std::string top_left = impl_->get_top_left_cell(impl_->range_str_);
    // 转义文本中的引号
    std::string escaped_text = text;
    size_t pos = 0;
    while ((pos = escaped_text.find("\"", pos)) != std::string::npos) {
        escaped_text.replace(pos, 1, "\"\"");
        pos += 2;
    }
    std::string formula = "NOT(ISERROR(SEARCH(\"" + escaped_text + "\"," + top_left + ")))";
    impl_->set_condition(ConditionalFormatType::ContainsText, ConditionalFormatOperator::ContainsText,
                        {formula}, text);
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_not_contains(const std::string& text) {
    std::string top_left = impl_->get_top_left_cell(impl_->range_str_);
    std::string escaped_text = text;
    size_t pos = 0;
    while ((pos = escaped_text.find("\"", pos)) != std::string::npos) {
        escaped_text.replace(pos, 1, "\"\"");
        pos += 2;
    }
    std::string formula = "ISERROR(SEARCH(\"" + escaped_text + "\"," + top_left + "))";
    impl_->set_condition(ConditionalFormatType::NotContainsText, ConditionalFormatOperator::NotContains,
                        {formula}, text);
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_begins_with(const std::string& text) {
    std::string top_left = impl_->get_top_left_cell(impl_->range_str_);
    std::string escaped_text = text;
    size_t pos = 0;
    while ((pos = escaped_text.find("\"", pos)) != std::string::npos) {
        escaped_text.replace(pos, 1, "\"\"");
        pos += 2;
    }
    std::string formula = "LEFT(" + top_left + "," + std::to_string(text.length()) + ")=\"" + escaped_text + "\"";
    impl_->set_condition(ConditionalFormatType::BeginsWith, ConditionalFormatOperator::BeginsWith,
                        {formula}, text);
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_ends_with(const std::string& text) {
    std::string top_left = impl_->get_top_left_cell(impl_->range_str_);
    std::string escaped_text = text;
    size_t pos = 0;
    while ((pos = escaped_text.find("\"", pos)) != std::string::npos) {
        escaped_text.replace(pos, 1, "\"\"");
        pos += 2;
    }
    std::string formula = "RIGHT(" + top_left + "," + std::to_string(text.length()) + ")=\"" + escaped_text + "\"";
    impl_->set_condition(ConditionalFormatType::EndsWith, ConditionalFormatOperator::EndsWith,
                        {formula}, text);
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_duplicate_values() {
    impl_->set_condition(ConditionalFormatType::DuplicateValues, ConditionalFormatOperator::Equal, {});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::when_unique_values() {
    impl_->set_condition(ConditionalFormatType::UniqueValues, ConditionalFormatOperator::Equal, {});
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::background_color(const Color& color) {
    if (impl_->current_rule_) {
        if (!impl_->current_rule_->fill) {
            impl_->current_rule_->fill = Fill();
        }
        impl_->current_rule_->fill->pattern_type = Fill::PatternType::Solid;
        impl_->current_rule_->fill->fg_color = color;
    }
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::font_color(const Color& color) {
    if (impl_->current_rule_) {
        if (!impl_->current_rule_->font) {
            impl_->current_rule_->font = Font();
        }
        impl_->current_rule_->font->color = color;
    }
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::font(const std::string& font_name, double size) {
    if (impl_->current_rule_) {
        if (!impl_->current_rule_->font) {
            impl_->current_rule_->font = Font();
        }
        impl_->current_rule_->font->family = font_name;
        impl_->current_rule_->font->size = size;
    }
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::bold(bool bold) {
    if (impl_->current_rule_) {
        if (!impl_->current_rule_->font) {
            impl_->current_rule_->font = Font();
        }
        impl_->current_rule_->font->bold = bold;
    }
    return *this;
}

ConditionalFormatBuilder& ConditionalFormatBuilder::italic(bool italic) {
    if (impl_->current_rule_) {
        if (!impl_->current_rule_->font) {
            impl_->current_rule_->font = Font();
        }
        impl_->current_rule_->font->italic = italic;
    }
    return *this;
}

void ConditionalFormatBuilder::apply() {
    impl_->apply_format();
}

// =============================================================================
// ConditionalFormatManager::Impl
// =============================================================================

class ConditionalFormatManager::Impl {
public:
    std::vector<ConditionalFormat> conditional_formats_;
};

// =============================================================================
// ConditionalFormatManager 实现
// =============================================================================

ConditionalFormatManager::ConditionalFormatManager()
    : impl_(std::make_unique<Impl>()) {
}

ConditionalFormatManager::~ConditionalFormatManager() = default;

void ConditionalFormatManager::add_conditional_format(const ConditionalFormat& format) {
    impl_->conditional_formats_.push_back(format);
}

void ConditionalFormatManager::remove_conditional_format(const std::string& range_str) {
    auto it = std::remove_if(impl_->conditional_formats_.begin(), impl_->conditional_formats_.end(),
        [&range_str](const ConditionalFormat& format) {
            return format.range == range_str;
        });
    impl_->conditional_formats_.erase(it, impl_->conditional_formats_.end());
}

const std::vector<ConditionalFormat>& ConditionalFormatManager::get_conditional_formats() const {
    return impl_->conditional_formats_;
}

void ConditionalFormatManager::clear() {
    impl_->conditional_formats_.clear();
}

} // namespace tinakit::excel
