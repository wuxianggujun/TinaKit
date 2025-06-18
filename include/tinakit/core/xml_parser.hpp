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

#include "tinakit/core/exceptions.hpp"


namespace xml
{
    class parser;
}


namespace tinakit::core
{
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
         * @brief 便利方法：解析整个 XML 元素及其子元素
         * @param element_name 要查找的元素名称
         * @param callback 处理元素的回调函数
         */
        void for_each_element(const std::string& element_name, 
                            std::function<void(iterator&)> callback);
        
        /**
         * @brief 便利方法：获取第一个匹配的元素的文本内容
         * @param element_name 元素名称
         * @return 元素的文本内容，如果未找到返回空字符串
         */
        std::string get_element_text(const std::string& element_name);
        
        /**
         * @brief 设置解析特性（如是否接收字符数据、属性等）
         * @param features 特性标志的组合
         */
        void set_features(int features);


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
            [[nodiscard]] bool has_attribute(const std::string& qname) const;
            
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
