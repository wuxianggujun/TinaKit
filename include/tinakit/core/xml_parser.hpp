//
// Created by wuxianggujun on 2025/6/16.
//

#pragma once

#include <istream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <map>
#include <functional>
#include <libstudxml/parser.hxx>
#include <libstudxml/serializer.hxx>

#include "tinakit/core/exceptions.hpp"


namespace xml
{
    class parser;
    class serializer;
}


namespace tinakit::core
{
    /**
     * @brief XML解析错误信息
     */
    struct XmlParseError {
        std::string message;
        std::size_t line;
        std::size_t column;
        std::string element_name;
        std::string attribute_name;

        XmlParseError(const std::string& msg, std::size_t ln = 0, std::size_t col = 0,
                     const std::string& elem = "", const std::string& attr = "")
            : message(msg), line(ln), column(col), element_name(elem), attribute_name(attr) {}
    };

    /**
     * @brief XML序列化器包装类，提供正确的命名空间处理
     */
    class XmlSerializer
    {
    public:
        explicit XmlSerializer(std::ostream& stream, const std::string& document_name = "document", unsigned short indentation = 2);
        ~XmlSerializer();

        XmlSerializer(const XmlSerializer&) = delete;
        XmlSerializer& operator=(const XmlSerializer&) = delete;
        XmlSerializer(XmlSerializer&&) = delete;
        XmlSerializer& operator=(XmlSerializer&&) = delete;

        // XML声明
        void xml_declaration(const std::string& version = "1.0", const std::string& encoding = "UTF-8", const std::string& standalone = "yes");

        // 元素操作
        void start_element(const std::string& name);
        void start_element(const std::string& namespace_uri, const std::string& name);
        void end_element();
        void element(const std::string& name, const std::string& content);
        void element_with_namespace(const std::string& namespace_uri, const std::string& name, const std::string& content);

        // 属性操作
        void attribute(const std::string& name, const std::string& value);
        void attribute(const std::string& namespace_uri, const std::string& name, const std::string& value);

        // 命名空间声明
        void namespace_declaration(const std::string& namespace_uri, const std::string& prefix = "");

        // 文本内容
        void characters(const std::string& text);

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

    class XmlParser
    {
    public:
        class iterator;

        explicit XmlParser(const std::string& file_path);

        /**
         * @brief Construct an XmlParser from an existing input stream.
         * @param stream The input stream to parse.
         * @param document_name A name for the document,used in error messages.
         */
        XmlParser(std::istream& stream, const std::string& document_name);

        ~XmlParser();

        XmlParser(const XmlParser&) = delete;
        XmlParser& operator=(const XmlParser&) = delete;
        XmlParser(XmlParser&&) = delete;
        XmlParser& operator=(XmlParser&&) = delete;

        /**
         * @brief Returns an iterator to the beginning of the XML document.
         * @return An iterator pointing to the first XML node/event
         */
        iterator begin();

        /**
         * @brief Returns a sentinel iterator representing the end of the XML document.
         * @return A sentinel end iterator
         */
        iterator end();

        /**
         * @brief Reset the parser to the beginning of the document for re-parsing
         * @return true if reset was successful, false otherwise
         */
        bool reset();

        /**
         * @brief Parse multiple element types in a single pass for better performance
         * @param element_handlers Map of element names to their handlers
         */
        void parse_multiple_elements(const std::map<std::string, std::function<void(iterator&)>>& element_handlers);

        /**
         * @brief Parse multiple element types with namespace support in a single pass
         * @param element_handlers Map of (namespace, element_name) pairs to their handlers
         */
        void parse_multiple_elements_ns(const std::map<std::pair<std::string, std::string>, std::function<void(iterator&)>>& element_handlers);

        /**
         * @brief Get the last parsing error (if any)
         * @return Optional error information
         */
        std::optional<XmlParseError> get_last_error() const;

        /**
         * @brief Enable or disable error recovery mode
         * @param enable If true, parser will try to continue parsing after errors
         */
        void set_error_recovery(bool enable);
        
        /**
         * @brief 便利方法：解析整个 XML 元素及其子元素
         * @param element_name 要查找的元素名称
         * @param callback 处理元素的回调函数
         */
        void for_each_element(const std::string& element_name,
                            std::function<void(iterator&)> callback);

        /**
         * @brief 便利方法：解析整个 XML 元素及其子元素（支持命名空间）
         * @param namespace_uri 命名空间URI
         * @param element_name 要查找的元素名称
         * @param callback 处理元素的回调函数
         */
        void for_each_element_ns(const std::string& namespace_uri, const std::string& element_name,
                               std::function<void(iterator&)> callback);

        /**
         * @brief 便利方法：获取第一个匹配的元素的文本内容
         * @param element_name 元素名称
         * @return 元素的文本内容，如果未找到返回空字符串
         */
        std::string get_element_text(const std::string& element_name);


        /**
         * @class iterator
         * @brief An Input iterator for traversing XML nodes.
         */
        class iterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = const iterator;
            using difference_type = std::ptrdiff_t;
            using pointer = const iterator*;
            using reference = const iterator&;

            iterator();

            reference operator*() const;
            pointer operator->() const;

            iterator& operator++();

            bool operator==(const iterator& other) const;
            bool operator!=(const iterator& other) const;

            [[nodiscard]] bool is_start_element() const;
            [[nodiscard]] bool is_end_element() const;
            [[nodiscard]] bool is_start_attribute() const;
            [[nodiscard]] bool is_end_attribute() const;
            [[nodiscard]] bool is_characters() const;
            [[nodiscard]] bool is_eof() const;

            [[nodiscard]] const std::string& name() const;
            [[nodiscard]] const std::string& value() const;
            [[nodiscard]] const std::string& namespace_uri() const;
            [[nodiscard]] const std::string& prefix() const;
            
            // 获取当前元素的文本内容（自动处理CDATA和字符实体）
            [[nodiscard]] std::string text_content();

            [[nodiscard]] std::optional<std::string> attribute(const std::string& qname) const;
            [[nodiscard]] std::optional<std::string> attribute(const xml::qname& qname) const;
            [[nodiscard]] bool has_attribute(const std::string& qname) const;
            [[nodiscard]] bool has_attribute(const xml::qname& qname) const;
            
            // 获取所有属性
            [[nodiscard]] std::map<std::string, std::string> attributes() const;
            
            // 获取当前解析位置（用于错误报告）
            [[nodiscard]] std::size_t line() const;
            [[nodiscard]] std::size_t column() const;

        private:
            friend class XmlParser;

            explicit iterator(XmlParser* parser);

            void advance();

            XmlParser* parser_ = nullptr;

            xml::parser::event_type current_event_;
        };

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
