/**
 * @file shared_strings.cpp
 * @brief Excel 共享字符串管理器实现
 * @author TinaKit Team
 * @date 2025-01-08
 */

#include "tinakit/excel/shared_strings.hpp"
#include "tinakit/excel/openxml_namespaces.hpp"

#include <iostream>

#include "tinakit/core/xml_parser.hpp"
#include "tinakit/core/xml_parser.hpp"
#include <sstream>
#include <stdexcept>

namespace tinakit::excel {

std::uint32_t SharedStrings::add_string(const std::string& str) {
    // 调试输出
    std::cout << "SharedStrings::add_string() 被调用，字符串: \"" << str << "\"" << std::endl;

    // 检查字符串是否已存在
    auto it = string_to_index_.find(str);
    if (it != string_to_index_.end()) {
        std::cout << "  字符串已存在，索引: " << it->second << std::endl;
        return it->second;
    }

    // 添加新字符串
    std::uint32_t index = static_cast<std::uint32_t>(strings_.size());
    strings_.push_back(str);
    string_to_index_[str] = index;

    std::cout << "  添加新字符串，索引: " << index << "，当前总数: " << strings_.size() << std::endl;

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
    std::ostringstream oss;
    core::XmlSerializer serializer(oss, "sharedStrings.xml");

    // XML声明
    serializer.xml_declaration("1.0", "UTF-8", "yes");

    // 开始sst元素（使用命名空间）
    serializer.start_element(excel::openxml_ns::main, "sst");

    // 声明命名空间
    serializer.namespace_declaration(excel::openxml_ns::main, "");

    // 添加count和uniqueCount属性
    serializer.attribute("count", std::to_string(strings_.size()));
    serializer.attribute("uniqueCount", std::to_string(strings_.size()));

    // 为每个字符串生成 <si> 元素
    for (const auto& str : strings_) {
        serializer.start_element("si");

        // 添加 <t> 元素包含文本内容
        // XmlSerializer会自动处理XML转义
        serializer.element("t", str);

        serializer.end_element(); // si
    }

    serializer.end_element(); // sst

    // 获取生成的XML内容
    std::string xml_content = oss.str();

    // 在调试模式下输出XML内容
    #ifdef _DEBUG
    std::cout << "\n=== 生成的共享字符串XML ===\n" << xml_content << "\n=== 共享字符串XML结束 ===\n" << std::endl;
    #endif

    return xml_content;
}

void SharedStrings::load_from_xml(const std::string& xml_data) {
    clear();
    
    std::istringstream stream(xml_data);
    core::XmlParser parser(stream, "sharedStrings.xml");
    
    // 首先找到 sst 元素并获取 uniqueCount
    for (auto it = parser.begin(); it != parser.end(); ++it) {
        if (it.is_start_element() && it.name() == "sst") {
            // 尝试获取 uniqueCount 以预分配内存
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
    
    // 重新解析以提取字符串
    stream.clear();
    stream.seekg(0);
    core::XmlParser parser2(stream, "sharedStrings.xml");
    
    // 使用改进的 API 解析每个 si 元素
    parser2.for_each_element("si", [this](core::XmlParser::iterator& it) {
        // si 元素可能包含 <t> 子元素或其他富文本元素
        std::string text = it.text_content();

        // 去除前后的空白字符（包括换行符、制表符、空格等）
        if (!text.empty()) {
            // 去除前面的空白字符
            auto start = text.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) {
                // 字符串全是空白字符
                return;
            }

            // 去除后面的空白字符
            auto end = text.find_last_not_of(" \t\n\r");
            text = text.substr(start, end - start + 1);

            if (!text.empty()) {
                add_string(text);
            }
        }
    });
}

void SharedStrings::reserve(std::size_t size) {
    strings_.reserve(size);
    string_to_index_.reserve(size);
}

bool SharedStrings::should_use_shared_string(const std::string& str) {
    // 智能决策规则：
    // 1. 空字符串使用内联
    if (str.empty()) {
        return false;
    }

    // 2. 很短的字符串（1-2个字符）使用内联
    if (str.length() <= 2) {
        return false;
    }

    // 3. 很长的字符串（>100字符）使用内联（避免共享字符串表过大）
    if (str.length() > 100) {
        return false;
    }

    // 4. 包含特殊字符或数字的字符串倾向于使用内联
    bool has_digit = std::any_of(str.begin(), str.end(), ::isdigit);
    if (has_digit && str.length() < 10) {
        return false;
    }

    // 5. 常见的标题、标签类文本使用共享
    // 包含中文、英文单词的文本倾向于共享
    return true;
}

void SharedStrings::record_string_usage(const std::string& str) {
    usage_count_[str]++;
}

std::size_t SharedStrings::get_usage_count(const std::string& str) const {
    auto it = usage_count_.find(str);
    return it != usage_count_.end() ? it->second : 0;
}

} // namespace tinakit::excel
