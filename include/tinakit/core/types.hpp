/**
 * @file types.hpp
 * @brief TinaKit core type definitions
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include <filesystem>
#include "color.hpp"

namespace tinakit {

// Forward declarations (移除了不必要的前向声明)

/**
 * @brief Configuration class
 */
struct Config {
    /**
     * @brief Default configuration
     * @return Default configuration object
     */
    static Config default_config();

    bool enable_async = true;           ///< Enable async processing
    std::size_t thread_pool_size = 4;   ///< Thread pool size
    std::size_t max_memory_usage = 1024 * 1024 * 1024;  ///< Maximum memory usage (bytes)
    bool enable_formula_calculation = true;  ///< Enable formula calculation
    std::string temp_directory = "";    ///< Temporary directory path
};



/**
 * @brief Alignment enumeration
 */
enum class Alignment {
    Left,           ///< Left alignment
    Center,         ///< Center alignment
    Right,          ///< Right alignment
    Justify,        ///< Justify alignment
    Top,            ///< Top alignment
    Middle,         ///< Vertical center
    Bottom          ///< Bottom alignment
};

/**
 * @brief Border type enumeration
 */
enum class BorderType {
    None,           ///< No border
    All,            ///< All borders
    Top,            ///< Top border
    Bottom,         ///< Bottom border
    Left,           ///< Left border
    Right,          ///< Right border
    Outline         ///< Outline border
};

/**
 * @brief Border style enumeration
 */
enum class BorderStyle {
    None,           ///< No style
    Thin,           ///< Thin line
    Medium,         ///< Medium line
    Thick,          ///< Thick line
    Double,         ///< Double line
    Dotted,         ///< Dotted line
    Dashed          ///< Dashed line
};

/**
 * @brief Position class
 */
struct Position {
    std::size_t row;     ///< Row number (1-based)
    std::size_t column;  ///< Column number (1-based)

    /**
     * @brief Constructor
     * @param r Row number
     * @param c Column number
     */
    Position(std::size_t r, std::size_t c) : row(r), column(c) {}

    /**
     * @brief Create position from address string
     * @param address Address string (e.g., "A1")
     * @return Position object
     */
    static Position from_address(const std::string& address);

    /**
     * @brief Convert to address string
     * @return Address string
     */
    std::string to_address() const;

    /**
     * @brief Create position after paragraph
     * @param paragraph_index Paragraph index
     * @return Position object
     */
    static Position after_paragraph(std::size_t paragraph_index);

    /**
     * @brief Equality operator
     * @param other Other position
     * @return True if positions are equal
     */
    bool operator==(const Position& other) const {
        return row == other.row && column == other.column;
    }

    /**
     * @brief Inequality operator
     * @param other Other position
     * @return True if positions are not equal
     */
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

/**
 * @brief Range class
 */
struct Range {
    Position start;  ///< Start position
    Position end;    ///< End position

    /**
     * @brief Constructor
     * @param start_pos Start position
     * @param end_pos End position
     */
    Range(const Position& start_pos, const Position& end_pos)
        : start(start_pos), end(end_pos) {}

    /**
     * @brief Create range from range string
     * @param range_str Range string (e.g., "A1:C10")
     * @return Range object
     */
    static Range from_string(const std::string& range_str);

    /**
     * @brief Convert to range string
     * @return Range string
     */
    std::string to_string() const;

    /**
     * @brief Check if position is within range
     * @param pos Position
     * @return True if position is within range
     */
    bool contains(const Position& pos) const;

    /**
     * @brief Get range size
     * @return Pair of row count and column count
     */
    std::pair<std::size_t, std::size_t> size() const;
};

/**
 * @brief Document type enumeration
 */
enum class DocumentType {
    Excel,          ///< Excel document
    Word,           ///< Word document
    PowerPoint,     ///< PowerPoint document
    Unknown         ///< Unknown type
};

/**
 * @brief Get document type from file extension
 * @param extension File extension
 * @return Document type
 */
DocumentType get_document_type(const std::string& extension);

/**
 * @brief Check if file extension is supported
 * @param extension File extension
 * @return True if supported
 */
bool is_supported_format(const std::string& extension);

/**
 * @brief Convert column number to column name
 * @param column Column number (1-based)
 * @return Column name (e.g., "A", "B", "AA")
 */
std::string column_number_to_name(std::size_t column);

/**
 * @brief Convert column name to column number
 * @param column_name Column name
 * @return Column number (1-based)
 */
std::size_t column_name_to_number(const std::string& column_name);

} // namespace tinakit

/**
 * @example types_usage.cpp
 * Type usage example:
 * @code
 * #include <tinakit/core/types.hpp>
 *
 * void types_example() {
 *     using namespace tinakit;
 *
 *     // Color usage
 *     auto red = Color(255, 0, 0);
 *     auto blue = Color("#0000FF");
 *
 *     // Position and range
 *     auto pos = Position::from_address("A1");
 *     auto range = Range::from_string("A1:C10");
 *
 *     // Column name conversion
 *     auto col_name = column_number_to_name(1);  // "A"
 *     auto col_num = column_name_to_number("AA");  // 27
 * }
 * @endcode
 */
