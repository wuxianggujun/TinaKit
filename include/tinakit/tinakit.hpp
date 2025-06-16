/**
 * @file tinakit.hpp
 * @brief TinaKit - 现代化的 OpenXML 文件处理 C++ 库
 * @version 1.0.0
 * @author TinaKit Team
 * @date 2024-12-16
 * 
 * TinaKit 是一个专为现代 C++ 设计的高性能 OpenXML 文件处理库，
 * 专注于 Excel (.xlsx)、Word (.docx) 等 Office 文档的读取和写入操作。
 * 
 * 核心特性：
 * - 现代 C++20 设计
 * - 直观的 API 接口
 * - 高性能异步处理
 * - 强类型安全
 * - 可扩展插件系统
 */

#pragma once

#include "core/types.hpp"
#include "core/exceptions.hpp"
#include "core/task.hpp"
#include "excel/workbook.hpp"
#include "excel/worksheet.hpp"
#include "excel/cell.hpp"
#include "excel/range.hpp"
#include "excel/row.hpp"
#include "word/document.hpp"
#include "formatting/style.hpp"
#include "plugins/plugin_manager.hpp"

/**
 * @namespace tinakit
 * @brief TinaKit 库的主命名空间
 */
namespace tinakit {

/**
 * @brief 获取 TinaKit 库版本信息
 * @return 版本字符串，格式为 "major.minor.patch"
 */
constexpr std::string_view version() noexcept {
    return "1.0.0";
}

/**
 * @brief 获取支持的文件格式列表
 * @return 支持的文件扩展名列表
 */
std::vector<std::string> supported_formats();

/**
 * @brief 注册自定义文件格式处理器
 * @tparam FormatHandler 格式处理器类型
 * @param extension 文件扩展名（包含点号，如 ".xlsx"）
 */
template<typename FormatHandler>
void register_format(std::string_view extension);

/**
 * @brief 通用文件打开函数
 * @param path 文件路径
 * @return 文档对象的智能指针
 * @throws FileNotFoundException 文件不存在
 * @throws CorruptedFileException 文件损坏
 * @throws UnsupportedFormatException 不支持的文件格式
 */
std::unique_ptr<Document> open(const std::filesystem::path& path);

/**
 * @brief 异步打开文件
 * @param path 文件路径
 * @return 返回文档对象的 Task
 */
Task<std::unique_ptr<Document>> open_async(const std::filesystem::path& path);

/**
 * @namespace tinakit::Excel
 * @brief Excel 文件处理相关功能
 */
namespace Excel {
    /**
     * @brief 打开 Excel 文件
     * @param path 文件路径
     * @return Workbook 对象
     * @throws FileNotFoundException 文件不存在
     * @throws CorruptedFileException 文件损坏
     */
    Workbook open(const std::filesystem::path& path);
    
    /**
     * @brief 异步打开 Excel 文件
     * @param path 文件路径
     * @return 返回 Workbook 的 Task
     */
    Task<Workbook> open_async(const std::filesystem::path& path);
    
    /**
     * @brief 创建新的 Excel 文件
     * @return 新的 Workbook 对象
     */
    Workbook create();
    
    /**
     * @brief 注册自定义 Excel 函数
     * @param name 函数名称
     * @param function 函数实现
     */
    void register_function(std::string_view name, 
                          std::function<double(const std::vector<double>&)> function);
}

/**
 * @namespace tinakit::Word
 * @brief Word 文档处理相关功能
 */
namespace Word {
    /**
     * @brief 打开 Word 文档
     * @param path 文件路径
     * @return Document 对象
     * @throws FileNotFoundException 文件不存在
     * @throws CorruptedFileException 文件损坏
     */
    Document open(const std::filesystem::path& path);
    
    /**
     * @brief 异步打开 Word 文档
     * @param path 文件路径
     * @return 返回 Document 的 Task
     */
    Task<Document> open_async(const std::filesystem::path& path);
    
    /**
     * @brief 创建新的 Word 文档
     * @return 新的 Document 对象
     */
    Document create();
}

/**
 * @namespace tinakit::TinaKit
 * @brief 核心库管理功能
 */
namespace TinaKit {
    /**
     * @brief 初始化 TinaKit 库
     * @param config 配置选项
     */
    void initialize(const Config& config = Config::default_config());
    
    /**
     * @brief 清理 TinaKit 库资源
     */
    void cleanup();
    
    /**
     * @brief 获取插件管理器
     * @return 插件管理器引用
     */
    PluginManager& plugin_manager();
}

} // namespace tinakit

/**
 * @example basic_usage.cpp
 * 基本使用示例：
 * @code
 * #include <tinakit/tinakit.hpp>
 * 
 * int main() {
 *     using namespace tinakit;
 *     
 *     // 打开 Excel 文件
 *     auto workbook = Excel::open("data.xlsx");
 *     auto sheet = workbook["Sheet1"];
 *     
 *     // 读取单元格
 *     auto value = sheet["A1"].as<std::string>();
 *     std::cout << "A1: " << value << std::endl;
 *     
 *     // 写入数据
 *     sheet["B1"].value("Hello, TinaKit!");
 *     
 *     // 保存文件
 *     workbook.save("output.xlsx");
 *     
 *     return 0;
 * }
 * @endcode
 */

/**
 * @example async_processing.cpp
 * 异步处理示例：
 * @code
 * #include <tinakit/tinakit.hpp>
 * 
 * Task<void> process_large_file() {
 *     using namespace tinakit;
 *     
 *     auto workbook = co_await Excel::open_async("large_file.xlsx");
 *     auto sheet = workbook[0];
 *     
 *     co_await sheet.process_rows_async([](const Row& row) -> Task<void> {
 *         // 处理每一行
 *         co_return;
 *     });
 * }
 * @endcode
 */

/**
 * @example modern_cpp_features.cpp
 * 现代 C++ 特性示例：
 * @code
 * #include <tinakit/tinakit.hpp>
 * #include <ranges>
 * 
 * void process_data() {
 *     using namespace tinakit;
 *     using namespace std::ranges;
 *     
 *     auto workbook = Excel::open("data.xlsx");
 *     auto sheet = workbook[0];
 *     
 *     // 使用 ranges 处理数据
 *     auto results = sheet.rows(2, 100)
 *         | views::filter([](const Row& row) { 
 *             return row["C"].as<double>() > 1000; 
 *         })
 *         | views::transform([](const Row& row) {
 *             return row["A"].as<std::string>();
 *         })
 *         | to<std::vector>();
 * }
 * @endcode
 */
