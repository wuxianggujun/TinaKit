/**
 * @file exceptions.hpp
 * @brief TinaKit 异常类定义
 * @author TinaKit Team
 * @date 2024-12-16
 */

#pragma once

#include <exception>
#include <string>
#include <filesystem>

namespace tinakit {

/**
 * @class TinaKitException
 * @brief TinaKit 基础异常类
 * 
 * 所有 TinaKit 特定异常的基类，提供统一的错误处理接口。
 */
class TinaKitException : public std::exception {
public:
    /**
     * @brief 构造函数
     * @param message 错误消息
     * @param context 错误上下文信息
     */
    explicit TinaKitException(std::string message, std::string context = "");
    
    /**
     * @brief 获取错误消息
     * @return 错误消息字符串
     */
    const char* what() const noexcept override;
    
    /**
     * @brief 获取错误上下文
     * @return 错误上下文字符串
     */
    const std::string& context() const noexcept;
    
    /**
     * @brief 获取完整错误信息
     * @return 包含消息和上下文的完整错误信息
     */
    std::string full_message() const;

private:
    std::string message_;   ///< 错误消息
    std::string context_;   ///< 错误上下文
};

/**
 * @class FileNotFoundException
 * @brief 文件未找到异常
 */
class FileNotFoundException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     * @param path 文件路径
     */
    explicit FileNotFoundException(const std::filesystem::path& path);
    
    /**
     * @brief 获取文件路径
     * @return 文件路径
     */
    const std::filesystem::path& file_path() const noexcept;

private:
    std::filesystem::path path_;
};

/**
 * @class CorruptedFileException
 * @brief 文件损坏异常
 */
class CorruptedFileException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     * @param path 文件路径
     * @param reason 损坏原因
     */
    CorruptedFileException(const std::filesystem::path& path, 
                          const std::string& reason = "");
    
    /**
     * @brief 获取文件路径
     * @return 文件路径
     */
    const std::filesystem::path& file_path() const noexcept;

private:
    std::filesystem::path path_;
};

/**
 * @class ParseException
 * @brief 解析异常
 */
class ParseException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     * @param message 错误消息
     * @param line 错误行号（从 1 开始）
     * @param column 错误列号（从 1 开始）
     */
    ParseException(const std::string& message, 
                   std::size_t line = 0, 
                   std::size_t column = 0);
    
    /**
     * @brief 获取错误位置
     * @return 行号和列号的对
     */
    std::pair<std::size_t, std::size_t> location() const noexcept;
    
    /**
     * @brief 获取错误行号
     * @return 行号（从 1 开始）
     */
    std::size_t line() const noexcept;
    
    /**
     * @brief 获取错误列号
     * @return 列号（从 1 开始）
     */
    std::size_t column() const noexcept;

private:
    std::size_t line_;
    std::size_t column_;
};

/**
 * @class IOException
 * @brief I/O 异常
 */
class IOException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     * @param message 错误消息
     * @param path 相关文件路径
     */
    IOException(const std::string& message, 
                const std::filesystem::path& path = {});
    
    /**
     * @brief 获取文件路径
     * @return 文件路径
     */
    const std::filesystem::path& file_path() const noexcept;

private:
    std::filesystem::path path_;
};

/**
 * @class UnsupportedFormatException
 * @brief 不支持的格式异常
 */
class UnsupportedFormatException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     * @param format 不支持的格式
     */
    explicit UnsupportedFormatException(const std::string& format);
    
    /**
     * @brief 获取格式名称
     * @return 格式名称
     */
    const std::string& format() const noexcept;

private:
    std::string format_;
};

/**
 * @class TypeConversionException
 * @brief 类型转换异常
 */
class TypeConversionException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     * @param from_type 源类型名称
     * @param to_type 目标类型名称
     * @param value 转换失败的值
     */
    TypeConversionException(const std::string& from_type,
                           const std::string& to_type,
                           const std::string& value = "");
    
    /**
     * @brief 获取源类型
     * @return 源类型名称
     */
    const std::string& from_type() const noexcept;
    
    /**
     * @brief 获取目标类型
     * @return 目标类型名称
     */
    const std::string& to_type() const noexcept;
    
    /**
     * @brief 获取转换失败的值
     * @return 值的字符串表示
     */
    const std::string& value() const noexcept;

private:
    std::string from_type_;
    std::string to_type_;
    std::string value_;
};

/**
 * @class WorksheetNotFoundException
 * @brief 工作表未找到异常
 */
class WorksheetNotFoundException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     * @param name 工作表名称
     */
    explicit WorksheetNotFoundException(const std::string& name);
    
    /**
     * @brief 获取工作表名称
     * @return 工作表名称
     */
    const std::string& worksheet_name() const noexcept;

private:
    std::string name_;
};

/**
 * @class DuplicateWorksheetNameException
 * @brief 重复工作表名称异常
 */
class DuplicateWorksheetNameException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     * @param name 重复的工作表名称
     */
    explicit DuplicateWorksheetNameException(const std::string& name);
    
    /**
     * @brief 获取工作表名称
     * @return 工作表名称
     */
    const std::string& worksheet_name() const noexcept;

private:
    std::string name_;
};

/**
 * @class CannotDeleteLastWorksheetException
 * @brief 不能删除最后一个工作表异常
 */
class CannotDeleteLastWorksheetException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     */
    CannotDeleteLastWorksheetException();
};

/**
 * @class InvalidCellAddressException
 * @brief 无效单元格地址异常
 */
class InvalidCellAddressException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     * @param address 无效的地址字符串
     */
    explicit InvalidCellAddressException(const std::string& address);
    
    /**
     * @brief 获取无效地址
     * @return 地址字符串
     */
    const std::string& address() const noexcept;

private:
    std::string address_;
};

/**
 * @class InvalidRowIndexException
 * @brief 无效行索引异常
 */
class InvalidRowIndexException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     * @param message 错误消息
     */
    explicit InvalidRowIndexException(const std::string& message);
};

/**
 * @class FormulaException
 * @brief 公式异常
 */
class FormulaException : public TinaKitException {
public:
    /**
     * @brief 构造函数
     * @param formula 出错的公式
     * @param reason 错误原因
     */
    FormulaException(const std::string& formula, const std::string& reason);
    
    /**
     * @brief 获取公式
     * @return 公式字符串
     */
    const std::string& formula() const noexcept;

private:
    std::string formula_;
};

} // namespace tinakit

/**
 * @example exception_handling.cpp
 * 异常处理示例：
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
 *         std::cerr << "文件未找到: " << e.file_path() << std::endl;
 *     } catch (const CorruptedFileException& e) {
 *         std::cerr << "文件损坏: " << e.file_path() 
 *                   << " - " << e.what() << std::endl;
 *     } catch (const ParseException& e) {
 *         std::cerr << "解析错误: " << e.what() 
 *                   << " 位置: " << e.line() << ":" << e.column() << std::endl;
 *     } catch (const TinaKitException& e) {
 *         std::cerr << "TinaKit 错误: " << e.full_message() << std::endl;
 *     } catch (const std::exception& e) {
 *         std::cerr << "系统错误: " << e.what() << std::endl;
 *     }
 * }
 * @endcode
 */
