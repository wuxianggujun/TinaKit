/**
 * @file shared_strings.cpp
 * @brief Excel 共享字符串管理器实现
 * @author TinaKit Team
 * @date 2025-01-08
 */

#include "tinakit/excel/shared_strings.hpp"
#include "tinakit/core/xml_parser.hpp"
#include <sstream>
#include <stdexcept>

namespace tinakit::excel {

std::uint32_t SharedStrings::add_string(const std::string& str) {
    // 检查字符串是否已存在
    auto it = string_to_index_.find(str);
    if (it != string_to_index_.end()) {
        return it->second;
    }
    
    // 添加新字符串
    std::uint32_t index = static_cast<std::uint32_t>(strings_.size());
    strings_.push_back(str);
    string_to_index_[str] = index;
    
    return index;
}

std::optional<std::uint32_t> SharedStrings::get_index(const std::string& str) const {
    auto it = string_to_index_.find(str);
    if (it != string_to_index_.end()) {
        return it->second;
    }
    return std::nullopt;
}

const std::string& SharedStrings::get_string(std::uint32_t index) const {
    if (index >= strings_.size()) {
        throw std::out_of_range("SharedStrings: index out of range: " + std::to_string(index));
    }
    return strings_[index];
}

void SharedStrings::clear() {
    strings_.clear();
    string_to_index_.clear();
}

std::string SharedStrings::generate_xml() const {
    std::ostringstream xml;
    
    // XML 声明
    xml << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)" << '\n';
    
    // 根元素
    xml << R"(<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count=")" 
        << strings_.size() << R"(" uniqueCount=")" << strings_.size() << R"(">)" << '\n';
    
    // 为每个字符串生成 <si> 元素
    for (const auto& str : strings_) {
        xml << "  <si><t>";
        
        // 对特殊字符进行 XML 转义
        for (char c : str) {
            switch (c) {
                case '<': xml << "&lt;"; break;
                case '>': xml << "&gt;"; break;
                case '&': xml << "&amp;"; break;
                case '"': xml << "&quot;"; break;
                case '\'': xml << "&apos;"; break;
                default: xml << c; break;
            }
        }
        
        xml << "</t></si>\n";
    }
    
    xml << "</sst>\n";
    
    return xml.str();
}

void SharedStrings::load_from_xml(const std::string& xml_data) {
    clear();
    
    std::istringstream stream(xml_data);
    core::XmlParser parser(stream, "sharedStrings.xml");
    
    bool in_si = false;
    bool in_t = false;
    std::string current_string;
    
    // 首先尝试获取 uniqueCount 以预分配内存
    for (auto it = parser.begin(); it != parser.end(); ++it) {
        if (it.is_start_element() && it.name() == "sst") {
            auto count_attr = it.attribute("uniqueCount");
            if (count_attr) {
                try {
                    std::size_t unique_count = std::stoull(*count_attr);
                    reserve(unique_count);
                } catch (...) {
                    // 忽略解析错误
                }
            }
            break;
        }
    }
    
    // 重新解析以获取字符串
    parser = core::XmlParser(stream, "sharedStrings.xml");
    
    for (auto it = parser.begin(); it != parser.end(); ++it) {
        if (it.is_start_element()) {
            if (it.name() == "si") {
                in_si = true;
                current_string.clear();
            } else if (in_si && it.name() == "t") {
                in_t = true;
                // 获取文本内容
                ++it;
                if (it != parser.end() && it.is_characters()) {
                    current_string = it.value();
                }
                in_t = false;
            }
        } else if (it.is_end_element()) {
            if (it.name() == "si") {
                in_si = false;
                // 添加字符串到表中
                add_string(current_string);
                current_string.clear();
            }
        }
    }
}

void SharedStrings::reserve(std::size_t size) {
    strings_.reserve(size);
    string_to_index_.reserve(size);
}

} // namespace tinakit::excel 