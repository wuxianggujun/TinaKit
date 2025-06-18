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
