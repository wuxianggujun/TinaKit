/**
 * @file exceptions.hpp
 * @brief TinaKit exception class definitions
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include <exception>
#include <string>
#include <filesystem>

namespace tinakit {

/**
 * @class TinaKitException
 * @brief Base exception class for TinaKit
 *
 * Base class for all TinaKit-specific exceptions, providing unified error handling interface.
 */
class TinaKitException : public std::exception {
public:
    /**
     * @brief Constructor
     * @param message Error message
     * @param context Error context information
     */
    explicit TinaKitException(std::string message, std::string context = "")
        : message_(std::move(message)), context_(std::move(context)) {}

    /**
     * @brief Get error message
     * @return Error message string
     */
    const char* what() const noexcept override {
        return message_.c_str();
    }

    /**
     * @brief Get error context
     * @return Error context string
     */
    const std::string& context() const noexcept {
        return context_;
    }

    /**
     * @brief Get full error information
     * @return Complete error information including message and context
     */
    std::string full_message() const {
        if (context_.empty()) {
            return message_;
        }
        return message_ + " (Context: " + context_ + ")";
    }

private:
    std::string message_;   ///< Error message
    std::string context_;   ///< Error context
};

/**
 * @class FileNotFoundException
 * @brief File not found exception
 */
class FileNotFoundException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param path File path
     */
    explicit FileNotFoundException(const std::filesystem::path& path)
        : TinaKitException("File not found: " + path.string()), path_(path) {}

    /**
     * @brief Get file path
     * @return File path
     */
    const std::filesystem::path& file_path() const noexcept {
        return path_;
    }

private:
    std::filesystem::path path_;
};

/**
 * @class CorruptedFileException
 * @brief Corrupted file exception
 */
class CorruptedFileException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param path File path
     * @param reason Corruption reason
     */
    CorruptedFileException(const std::filesystem::path& path,
                          const std::string& reason = "")
        : TinaKitException("Corrupted file: " + path.string() +
                          (reason.empty() ? "" : " (" + reason + ")")),
          path_(path) {}

    /**
     * @brief Get file path
     * @return File path
     */
    const std::filesystem::path& file_path() const noexcept {
        return path_;
    }

private:
    std::filesystem::path path_;
};

/**
 * @class ParseException
 * @brief Parse exception
 */
class ParseException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param message Error message
     * @param line Error line number (1-based)
     * @param column Error column number (1-based)
     */
    ParseException(const std::string& message,
                   std::size_t line = 0,
                   std::size_t column = 0)
        : TinaKitException(message + (line > 0 ? " at line " + std::to_string(line) : "") +
                          (column > 0 ? ", column " + std::to_string(column) : "")),
          line_(line), column_(column) {}

    /**
     * @brief Get error location
     * @return Pair of line and column numbers
     */
    std::pair<std::size_t, std::size_t> location() const noexcept {
        return {line_, column_};
    }

    /**
     * @brief Get error line number
     * @return Line number (1-based)
     */
    std::size_t line() const noexcept {
        return line_;
    }

    /**
     * @brief Get error column number
     * @return Column number (1-based)
     */
    std::size_t column() const noexcept {
        return column_;
    }

private:
    std::size_t line_;
    std::size_t column_;
};

/**
 * @class IOException
 * @brief I/O exception
 */
class IOException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param message Error message
     * @param path Related file path
     */
    IOException(const std::string& message,
                const std::filesystem::path& path = {})
        : TinaKitException(message + (path.empty() ? "" : " (File: " + path.string() + ")")),
          path_(path) {}

    /**
     * @brief Get file path
     * @return File path
     */
    const std::filesystem::path& file_path() const noexcept {
        return path_;
    }

private:
    std::filesystem::path path_;
};

/**
 * @class UnsupportedFormatException
 * @brief Unsupported format exception
 */
class UnsupportedFormatException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param format Unsupported format
     */
    explicit UnsupportedFormatException(const std::string& format)
        : TinaKitException("Unsupported format: " + format), format_(format) {}

    /**
     * @brief Get format name
     * @return Format name
     */
    const std::string& format() const noexcept {
        return format_;
    }

private:
    std::string format_;
};

/**
 * @class TypeConversionException
 * @brief Type conversion exception
 */
class TypeConversionException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param from_type Source type name
     * @param to_type Target type name
     * @param value Value that failed to convert
     */
    TypeConversionException(const std::string& from_type,
                           const std::string& to_type,
                           const std::string& value = "")
        : TinaKitException("Cannot convert from " + from_type + " to " + to_type +
                          (value.empty() ? "" : " (value: " + value + ")")),
          from_type_(from_type), to_type_(to_type), value_(value) {}

    /**
     * @brief Get source type
     * @return Source type name
     */
    const std::string& from_type() const noexcept {
        return from_type_;
    }

    /**
     * @brief Get target type
     * @return Target type name
     */
    const std::string& to_type() const noexcept {
        return to_type_;
    }

    /**
     * @brief Get value that failed to convert
     * @return String representation of the value
     */
    const std::string& value() const noexcept {
        return value_;
    }

private:
    std::string from_type_;
    std::string to_type_;
    std::string value_;
};

/**
 * @class WorksheetNotFoundException
 * @brief Worksheet not found exception
 */
class WorksheetNotFoundException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param name Worksheet name
     */
    explicit WorksheetNotFoundException(const std::string& name)
        : TinaKitException("Worksheet not found: " + name), name_(name) {}

    /**
     * @brief Get worksheet name
     * @return Worksheet name
     */
    const std::string& worksheet_name() const noexcept {
        return name_;
    }

private:
    std::string name_;
};

/**
 * @class DuplicateWorksheetNameException
 * @brief Duplicate worksheet name exception
 */
class DuplicateWorksheetNameException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param name Duplicate worksheet name
     */
    explicit DuplicateWorksheetNameException(const std::string& name)
        : TinaKitException("Duplicate worksheet name: " + name), name_(name) {}

    /**
     * @brief Get worksheet name
     * @return Worksheet name
     */
    const std::string& worksheet_name() const noexcept {
        return name_;
    }

private:
    std::string name_;
};

/**
 * @class CannotDeleteLastWorksheetException
 * @brief Cannot delete last worksheet exception
 */
class CannotDeleteLastWorksheetException : public TinaKitException {
public:
    /**
     * @brief Constructor
     */
    CannotDeleteLastWorksheetException()
        : TinaKitException("Cannot delete the last worksheet") {}
};

/**
 * @class InvalidCellAddressException
 * @brief Invalid cell address exception
 */
class InvalidCellAddressException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param address Invalid address string
     */
    explicit InvalidCellAddressException(const std::string& address)
        : TinaKitException("Invalid cell address: " + address), address_(address) {}

    /**
     * @brief Get invalid address
     * @return Address string
     */
    const std::string& address() const noexcept {
        return address_;
    }

private:
    std::string address_;
};

/**
 * @class InvalidRowIndexException
 * @brief Invalid row index exception
 */
class InvalidRowIndexException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param message Error message
     */
    explicit InvalidRowIndexException(const std::string& message)
        : TinaKitException(message) {}
};

/**
 * @class FormulaException
 * @brief Formula exception
 */
class FormulaException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param formula Formula that caused the error
     * @param reason Error reason
     */
    FormulaException(const std::string& formula, const std::string& reason)
        : TinaKitException("Formula error: " + reason + " (Formula: " + formula + ")"),
          formula_(formula) {}

    /**
     * @brief Get formula
     * @return Formula string
     */
    const std::string& formula() const noexcept {
        return formula_;
    }

private:
    std::string formula_;
};

/**
 * @class OperationCanceledException
 * @brief Operation canceled exception (for async operations)
 */
class OperationCanceledException : public TinaKitException {
public:
    /**
     * @brief Constructor
     */
    OperationCanceledException()
        : TinaKitException("Operation was canceled", "Async") {}

    /**
     * @brief Constructor with custom message
     * @param message Custom error message
     */
    explicit OperationCanceledException(const std::string& message)
        : TinaKitException(message, "Async") {}
};

/**
 * @class TimeoutException
 * @brief Timeout exception (for async operations)
 */
class TimeoutException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param timeout_duration Timeout duration description
     */
    explicit TimeoutException(const std::string& timeout_duration = "")
        : TinaKitException("Operation timed out" +
                          (timeout_duration.empty() ? "" : " after " + timeout_duration),
                          "Async") {}
};

/**
 * @class ExecutorException
 * @brief Executor exception (for async operations)
 */
class ExecutorException : public TinaKitException {
public:
    /**
     * @brief Constructor
     * @param message Error message
     */
    explicit ExecutorException(const std::string& message)
        : TinaKitException(message, "Async::Executor") {}
};

} // namespace tinakit

/**
 * @example exception_handling.cpp
 * Exception handling example:
 * @code
 * #include <tinakit/core/exceptions.hpp>
 * #include <tinakit/tinakit.hpp>
 *
 * void exception_example() {
 *     using namespace tinakit;
 *
 *     try {
 *         auto workbook = Excel::open("nonexistent.xlsx");
 *     } catch (const FileNotFoundException& e) {
 *         std::cerr << "File not found: " << e.file_path() << std::endl;
 *     } catch (const CorruptedFileException& e) {
 *         std::cerr << "File corrupted: " << e.file_path()
 *                   << " - " << e.what() << std::endl;
 *     } catch (const ParseException& e) {
 *         std::cerr << "Parse error: " << e.what()
 *                   << " at " << e.line() << ":" << e.column() << std::endl;
 *     } catch (const TinaKitException& e) {
 *         std::cerr << "TinaKit error: " << e.full_message() << std::endl;
 *     } catch (const std::exception& e) {
 *         std::cerr << "System error: " << e.what() << std::endl;
 *     }
 * }
 * @endcode
 */
