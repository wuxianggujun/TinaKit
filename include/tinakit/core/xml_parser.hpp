//
// Created by wuxianggujun on 2025/6/16.
//

#pragma once

#include <istream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
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


            [[nodiscard]] const std::string& name() const;

            [[nodiscard]] const std::string& value() const;

            [[nodiscard]] std::optional<std::string> attribute(const std::string& qname) const;

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
