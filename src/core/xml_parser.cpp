//
// Created by wuxianggujun on 2025/6/16.
//

#include "tinakit/core/xml_parser.hpp"
#include <fstream>
#include <libstudxml/parser.hxx>

namespace tinakit::core
{
    struct XmlParser::Impl
    {
        // Manages file stream lifetime
        std::unique_ptr<std::ifstream> owned_stream;
        // Reference to the active stream
        std::istream& stream_ref;
        // The actual libstudxml parser
        std::unique_ptr<xml::parser> parser;

        Impl(std::unique_ptr<std::ifstream> stream, const std::string& document_name): owned_stream(std::move(stream)),
            stream_ref(*owned_stream),
            parser(std::make_unique<xml::parser>(stream_ref, document_name))
        {
        }

        Impl(std::istream& stream, const std::string& document_name): owned_stream(nullptr), stream_ref(stream),
                                                                      parser(std::make_unique<xml::parser>(
                                                                          stream_ref, document_name))
        {
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
    
    void XmlParser::set_features(int features)
    {
        // 注意：libstudxml 在构造时设置 features，不能在运行时修改
        // 这个方法保留为将来可能的扩展
        (void)features; // 避免未使用参数警告
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
    
    bool XmlParser::iterator::has_attribute(const std::string& qname) const
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
            // Wrap libstudxml's exception into our project-specific exception type.
            throw ParseException(e.what());
        }
        
    }
} // tinakit
