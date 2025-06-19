/**
 * @file types.hpp
 * @brief TinaKit core type definitions
 * @author TinaKit Team
 * @date 2025-6-16
 * 
 * @section error_handling Error Handling Strategy
 * 
 * TinaKit follows these error handling principles:
 * 
 * 1. **Exceptions for Errors**: Operations that can fail throw exceptions
 *    - File I/O errors throw IOException
 *    - Invalid data throws ParseException
 *    - Type conversions throw TypeConversionException
 * 
 * 2. **Optional for Missing Values**: Operations that may not have a result return std::optional
 *    - Cell::try_as<T>() returns std::optional<T>
 *    - Finding elements that may not exist
 * 
 * 3. **Noexcept for Query Operations**: Simple getters are noexcept
 *    - Getting size, count, or status information
 *    - Accessing already-validated data
 * 
 * 4. **RAII for Resource Management**: All resources are managed automatically
 *    - No manual cleanup required
 *    - Exception-safe by design
 */

#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include <filesystem>
#include "color.hpp"

namespace tinakit {

/**
 * @brief Configuration class
 */
struct Config {
    /**
     * @brief Default configuration
     * @return Default configuration object
     */
    static Config default_config() {
        return Config{};
    }

    bool enable_async = true;           ///< Enable async processing
    std::size_t thread_pool_size = 4;   ///< Thread pool size
    std::size_t max_memory_usage = 1024 * 1024 * 1024;  ///< Maximum memory usage (bytes)
    bool enable_formula_calculation = true;  ///< Enable formula calculation
    std::string temp_directory = "";    ///< Temporary directory path
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
 *     // Configuration
 *     auto config = Config::default_config();
 *     config.enable_async = false;
 *     
 *     // Document type
 *     auto doc_type = get_document_type(".xlsx");  // DocumentType::Excel
 * }
 * @endcode
 */