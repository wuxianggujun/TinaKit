//
// Created by wuxianggujun on 2025/6/16.
//

#include "tinakit/core/xml_parser.hpp"
#include <fstream>
#include <iostream>
#include <libstudxml/parser.hxx>
#include <libstudxml/serializer.hxx>
#include <unordered_set>

namespace tinakit::core
{
    // ========================================
    // XmlSerializer Implementation
    // ========================================

    struct XmlSerializer::Impl
    {
        std::unique_ptr<xml::serializer> serializer;

        Impl(std::ostream& stream, const std::string& document_name, unsigned short indentation)
            : serializer(std::make_unique<xml::serializer>(stream, document_name, indentation))
        {
        }
    };

    XmlSerializer::XmlSerializer(std::ostream& stream, const std::string& document_name, unsigned short indentation)
        : impl_(std::make_unique<Impl>(stream, document_name, indentation))
    {
    }

    XmlSerializer::~XmlSerializer() = default;

    void XmlSerializer::xml_declaration(const std::string& version, const std::string& encoding, const std::string& standalone)
    {
        impl_->serializer->xml_decl(version, encoding, standalone);
    }

    void XmlSerializer::start_element(const std::string& name)
    {
        impl_->serializer->start_element(name);
    }

    void XmlSerializer::start_element(const std::string& namespace_uri, const std::string& name)
    {
        impl_->serializer->start_element(namespace_uri, name);
    }

    void XmlSerializer::end_element()
    {
        impl_->serializer->end_element();
    }

    void XmlSerializer::element(const std::string& name, const std::string& content)
    {
        impl_->serializer->element(name, content);
    }

    void XmlSerializer::element_with_namespace(const std::string& namespace_uri, const std::string& name, const std::string& content)
    {
        impl_->serializer->element(namespace_uri, name, content);
    }

    void XmlSerializer::attribute(const std::string& name, const std::string& value)
    {
        impl_->serializer->attribute(name, value);
    }

    void XmlSerializer::attribute(const std::string& namespace_uri, const std::string& name, const std::string& value)
    {
        impl_->serializer->attribute(namespace_uri, name, value);
    }

    void XmlSerializer::namespace_declaration(const std::string& namespace_uri, const std::string& prefix)
    {
        impl_->serializer->namespace_decl(namespace_uri, prefix);
    }

    void XmlSerializer::characters(const std::string& text)
    {
        impl_->serializer->characters(text);
    }

    // ========================================
    // XmlParser Implementation
    // ========================================

    // OpenXML标准属性集合
    const std::unordered_set<std::string> known_openxml_attributes = {
            // 通用属性
            "ref", "count", "uniqueCount", "r", "t", "s", "name", "sheetId",
            // 工作表属性
            "tabSelected", "workbookViewId", "defaultRowHeight", "defaultColWidth",
            // 关系属性
            "Id", "Type", "Target", "r:id",
            // 命名空间属性
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships#id",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
            // 样式属性
            "numFmtId", "fontId", "fillId", "borderId", "xfId",
            // 填充样式属性
            "patternType",
            // 单元格样式应用属性
            "applyFont", "applyFill", "applyBorder", "applyAlignment", "applyNumberFormat", "applyProtection",
            // Excel兼容性属性
            "pivotButton", "quotePrefix", "builtinId",
            // 条件格式属性
            "dxfId", "priority", "operator", "type", "sqref",
            // 其他常见属性
            "val", "sz", "b", "i", "u", "color", "rgb", "theme", "indexed"
        };
    
    struct XmlParser::Impl
    {
        // Manages file stream lifetime
        std::unique_ptr<std::ifstream> owned_stream;
        // Reference to the active stream
        std::istream& stream_ref;
        // The actual libstudxml parser
        std::unique_ptr<xml::parser> parser;
        // Document name for error reporting
        std::string document_name;
        // Error handling
        std::optional<XmlParseError> last_error;
        bool error_recovery_enabled = false;

        Impl(std::unique_ptr<std::ifstream> stream, const std::string& doc_name):
            owned_stream(std::move(stream)),
            stream_ref(*owned_stream),
            document_name(doc_name)
        {
            create_parser();
        }

        Impl(std::istream& stream, const std::string& doc_name):
            owned_stream(nullptr),
            stream_ref(stream),
            document_name(doc_name)
        {
            create_parser();
        }

        void create_parser() {
            parser = std::make_unique<xml::parser>(stream_ref, document_name,
                xml::parser::receive_elements |
                xml::parser::receive_characters |
                xml::parser::receive_attributes_map |
                xml::parser::receive_namespace_decls);
        }

        bool reset() {
            try {
                // Reset stream position to beginning
                stream_ref.clear(); // Clear any error flags
                stream_ref.seekg(0, std::ios::beg);

                if (stream_ref.good()) {
                    // Recreate parser with fresh stream
                    create_parser();
                    return true;
                }
                return false;
            } catch (...) {
                return false;
            }
        }
    };


    // -- XmlParser Implementation -- //


    XmlParser::XmlParser(const std::string& file_path)
    {
        auto ifs = std::make_unique<std::ifstream>(file_path, std::ios::binary);
        if (!ifs->is_open())
        {
            throw FileNotFoundException("Failed to open XML file: " + file_path);
        }
        impl_ = std::make_unique<Impl>(std::move(ifs), file_path);
    }

    XmlParser::XmlParser(std::istream& stream, const std::string& document_name)
    {
        impl_ = std::make_unique<Impl>(stream, document_name);
    }

    XmlParser::~XmlParser() = default;

    XmlParser::iterator XmlParser::begin()
    {
        return iterator(this);  
    }

    XmlParser::iterator XmlParser::end()
    {
        return {};
    }

    bool XmlParser::reset()
    {
        impl_->last_error.reset(); // Clear any previous errors
        return impl_->reset();
    }

    void XmlParser::parse_multiple_elements(const std::map<std::string, std::function<void(iterator&)>>& element_handlers)
    {
        for (auto it = begin(); it != end(); ++it)
        {
            if (it.is_start_element()) {
                auto handler_it = element_handlers.find(it.name());
                if (handler_it != element_handlers.end()) {
                    try {
                        handler_it->second(it);
                    } catch (const std::exception& e) {
                        impl_->last_error = XmlParseError(e.what(), it.line(), it.column(), it.name());
                        if (!impl_->error_recovery_enabled) {
                            throw;
                        }
                    }
                }
            }
        }
    }

    void XmlParser::parse_multiple_elements_ns(const std::map<std::pair<std::string, std::string>, std::function<void(iterator&)>>& element_handlers)
    {
        for (auto it = begin(); it != end(); ++it)
        {
            if (it.is_start_element()) {
                std::pair<std::string, std::string> key = {it.namespace_uri(), it.name()};
                auto handler_it = element_handlers.find(key);
                if (handler_it != element_handlers.end()) {
                    try {
                        handler_it->second(it);
                    } catch (const std::exception& e) {
                        impl_->last_error = XmlParseError(e.what(), it.line(), it.column(), it.name());
                        if (!impl_->error_recovery_enabled) {
                            throw;
                        }
                    }
                }
            }
        }
    }

    std::optional<XmlParseError> XmlParser::get_last_error() const
    {
        return impl_->last_error;
    }

    void XmlParser::set_error_recovery(bool enable)
    {
        impl_->error_recovery_enabled = enable;
    }
    
    void XmlParser::for_each_element(const std::string& element_name,
                                    std::function<void(iterator&)> callback)
    {
        for (auto it = begin(); it != end(); ++it)
        {
            if (it.is_start_element() && it.name() == element_name)
            {
                callback(it);
            }
        }
    }

    void XmlParser::for_each_element_ns(const std::string& namespace_uri, const std::string& element_name,
                                       std::function<void(iterator&)> callback)
    {
        for (auto it = begin(); it != end(); ++it)
        {
            if (it.is_start_element() && it.name() == element_name && it.namespace_uri() == namespace_uri)
            {
                callback(it);
            }
        }
    }

    std::string XmlParser::get_element_text(const std::string& element_name)
    {
        for (auto it = begin(); it != end(); ++it)
        {
            if (it.is_start_element() && it.name() == element_name)
            {
                return it.text_content();
            }
        }
        return "";
    }
    



    // -- XmlParser::iterator Implementation -- //

    XmlParser::iterator::iterator():parser_(nullptr),current_event_(xml::parser::event_type::eof)
    {
    }

    XmlParser::iterator::reference XmlParser::iterator::operator*() const
    {
        return *this;
    }

    XmlParser::iterator::pointer XmlParser::iterator::operator->() const
    {
        return this;
    }

    XmlParser::iterator& XmlParser::iterator::operator++()
    {
        advance();
        return *this;
    }

    bool XmlParser::iterator::operator==(const iterator& other) const
    {
        return parser_ == other.parser_;
    }

    bool XmlParser::iterator::operator!=(const iterator& other) const
    {
        return !(*this == other);
    }

    bool XmlParser::iterator::is_start_element() const
    {
        return current_event_ == xml::parser::event_type::start_element;
    }

    bool XmlParser::iterator::is_end_element() const
    {
        return current_event_ == xml::parser::event_type::end_element;
    }
    
    bool XmlParser::iterator::is_start_attribute() const
    {
        return current_event_ == xml::parser::event_type::start_attribute;
    }
    
    bool XmlParser::iterator::is_end_attribute() const
    {
        return current_event_ == xml::parser::event_type::end_attribute;
    }
    
    bool XmlParser::iterator::is_characters() const
    {
        return current_event_ == xml::parser::event_type::characters;
    }
    
    bool XmlParser::iterator::is_eof() const
    {
        return current_event_ == xml::parser::event_type::eof;
    }

    const std::string& XmlParser::iterator::name() const
    {
        if (parser_)
        {
            return parser_->impl_->parser->name();
        }
        static const std::string empty_string;
        return empty_string;
    }

    const std::string& XmlParser::iterator::value() const
    {
        if (parser_)
        {
            return parser_->impl_->parser->value();
        }
        static const std::string empty_string;
        return empty_string;
    }

    const std::string& XmlParser::iterator::namespace_uri() const
    {
        if (parser_)
        {
            return parser_->impl_->parser->namespace_();
        }
        static const std::string empty_string;
        return empty_string;
    }
    
    const std::string& XmlParser::iterator::prefix() const
    {
        if (parser_)
        {
            return parser_->impl_->parser->prefix();
        }
        static const std::string empty_string;
        return empty_string;
    }
    
    std::string XmlParser::iterator::text_content()
    {
        if (!parser_ || !is_start_element())
        {
            return "";
        }
        
        std::string content;
        int depth = 1;
        
        // 前进到下一个事件
        ++(*this);
        
        while (parser_ && depth > 0)
        {
            if (is_characters())
            {
                content += value();
            }
            else if (is_start_element())
            {
                depth++;
            }
            else if (is_end_element())
            {
                depth--;
                if (depth == 0)
                {
                    break;
                }
            }
            ++(*this);
        }
        
        return content;
    }

    std::optional<std::string> XmlParser::iterator::attribute(const std::string& qname) const
    {
        if (!parser_)
        {
            return std::nullopt;
        }

        try {
            // libstudxml 使用 attribute 方法获取属性，如果属性不存在会抛出异常
            return parser_->impl_->parser->attribute(qname);
        } catch (const std::exception&) {
            // 属性不存在时返回 nullopt
            return std::nullopt;
        }
    }

    std::optional<std::string> XmlParser::iterator::attribute(const xml::qname& qname) const
    {
        if (!parser_)
        {
            return std::nullopt;
        }

        try {
            // libstudxml 使用 attribute 方法获取属性，如果属性不存在会抛出异常
            return parser_->impl_->parser->attribute(qname);
        } catch (const std::exception&) {
            // 属性不存在时返回 nullopt
            return std::nullopt;
        }
    }

    bool XmlParser::iterator::has_attribute(const std::string& qname) const
    {
        return attribute(qname).has_value();
    }

    bool XmlParser::iterator::has_attribute(const xml::qname& qname) const
    {
        return attribute(qname).has_value();
    }
    
    std::map<std::string, std::string> XmlParser::iterator::attributes() const
    {
        std::map<std::string, std::string> attrs;
        
        if (!parser_ || !is_start_element())
        {
            return attrs;
        }
        
        // 获取属性映射（这是 libstudxml 的特性）
        const auto& attr_map = parser_->impl_->parser->attribute_map();
        for (const auto& [qname, attr_value] : attr_map)
        {
            attrs[qname.name()] = attr_value.value;
        }
        
        return attrs;
    }
    
    std::size_t XmlParser::iterator::line() const
    {
        if (parser_)
        {
            return parser_->impl_->parser->line();
        }
        return 0;
    }
    
    std::size_t XmlParser::iterator::column() const
    {
        if (parser_)
        {
            return parser_->impl_->parser->column();
        }
        return 0;
    }

    XmlParser::iterator::iterator(XmlParser* parser) : parser_(parser)
    {
        // Prime the iterator by loading the first event
        advance();
    }

    void XmlParser::iterator::advance()
    {
        if (parser_ == nullptr) return;

        try
        {
            current_event_ = parser_->impl_->parser->next();
            // if we reach the end, nullify the parser pointer to match the end sentinel.
            if (current_event_ ==  xml::parser::event_type::eof)
            {
                parser_ = nullptr;
            }
        }catch (const xml::parsing& e)
        {
            // 检查是否是属性相关的错误
            std::string error_msg = e.what();
            if (error_msg.find("unexpected attribute") != std::string::npos) {
                // 提取属性名
                std::string attr_name;
                auto quote_pos = error_msg.find("'");
                if (quote_pos != std::string::npos) {
                    auto end_quote = error_msg.find("'", quote_pos + 1);
                    if (end_quote != std::string::npos) {
                        attr_name = error_msg.substr(quote_pos + 1, end_quote - quote_pos - 1);
                    }
                }

                // 检查是否是已知的OpenXML属性
                if (known_openxml_attributes.find(attr_name) != known_openxml_attributes.end()) {
                    // 这是已知的OpenXML属性，静默忽略
                    try {
                        // 尝试继续解析下一个事件
                        current_event_ = parser_->impl_->parser->next();
                        if (current_event_ ==  xml::parser::event_type::eof) {
                            parser_ = nullptr;
                        }
                        return;
                    } catch (...) {
                        // 如果还是失败，则抛出原始异常
                    }
                } else {
                    // 真正未知的属性，显示警告
                    std::cout << "警告: 遇到未知属性 '" << attr_name << "': " << error_msg << std::endl;
                    try {
                        current_event_ = parser_->impl_->parser->next();
                        if (current_event_ ==  xml::parser::event_type::eof) {
                            parser_ = nullptr;
                        }
                        return;
                    } catch (...) {
                        // 如果还是失败，则抛出原始异常
                    }
                }
            }

            // Wrap libstudxml's exception into our project-specific exception type.
            throw ParseException(e.what());
        }

    }
} // tinakit
