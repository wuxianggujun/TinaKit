/**
 * @file tinakit.hpp
 * @brief TinaKit - Modern OpenXML File Processing C++ Library
 * @version 1.0.0
 * @author TinaKit Team
 * @date 2025-6-16
 *
 */

#pragma once

#include "core/types.hpp"
#include "core/exceptions.hpp"
#include "core/async.hpp"
#include "core/xml_parser.hpp"
#include "core/openxml_archiver.hpp"
#include "tinakit/excel/workbook.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/cell.hpp"
#include "tinakit/excel/row.hpp"
#include <filesystem>
#include <functional>

/*#include "excel/worksheet.hpp"
#include "excel/cell.hpp"
#include "excel/range.hpp"
#include "excel/row.hpp"
#include "word/document.hpp"
#include "formatting/style.hpp"
#include "plugins/plugin_manager.hpp"*/

/*/**
 * @namespace tinakit
 * @brief TinaKit 库的主命名空间
 #1#
namespace tinakit {

/**
 * @brief 获取 TinaKit 库版本信息
 * @return 版本字符串，格式为 "major.minor.patch"
 #1#
constexpr std::string_view version() noexcept {
    return "1.0.0";
}

/**
 * @brief 获取支持的文件格式列表
 * @return 支持的文件扩展名列表
 #1#
std::vector<std::string> supported_formats();

/**
 * @brief 注册自定义文件格式处理器
 * @tparam FormatHandler 格式处理器类型
 * @param extension 文件扩展名（包含点号，如 ".xlsx"）
 #1#
template<typename FormatHandler>
void register_format(std::string_view extension);

/**
 * @brief 通用文件打开函数
 * @param path 文件路径
 * @return 文档对象的智能指针
 * @throws FileNotFoundException 文件不存在
 * @throws CorruptedFileException 文件损坏
 * @throws UnsupportedFormatException 不支持的文件格式
 #1#
std::unique_ptr<Document> open(const std::filesystem::path& path);

/**
 * @brief 异步打开文件
 * @param path 文件路径
 * @return 返回文档对象的 Task
 #1#
Task<std::unique_ptr<Document>> open_async(const std::filesystem::path& path);

/**
 * @namespace tinakit::Excel
 * @brief Excel 文件处理相关功能
 #1#
namespace Excel {
    /**
     * @brief 打开 Excel 文件
     * @param path 文件路径
     * @return Workbook 对象
     * @throws FileNotFoundException 文件不存在
     * @throws CorruptedFileException 文件损坏
     #1#
    Workbook open(const std::filesystem::path& path);
    
    /**
     * @brief 异步打开 Excel 文件
     * @param path 文件路径
     * @return 返回 Workbook 的 Task
     #1#
    Task<Workbook> open_async(const std::filesystem::path& path);
    
    /**
     * @brief 创建新的 Excel 文件
     * @return 新的 Workbook 对象
     #1#
    Workbook create();
    
    /**
     * @brief 注册自定义 Excel 函数
     * @param name 函数名称
     * @param function 函数实现
     #1#
    void register_function(std::string_view name, 
                          std::function<double(const std::vector<double>&)> function);
}

/**
 * @namespace tinakit::Word
 * @brief Word 文档处理相关功能
 #1#
namespace Word {
    /**
     * @brief 打开 Word 文档
     * @param path 文件路径
     * @return Document 对象
     * @throws FileNotFoundException 文件不存在
     * @throws CorruptedFileException 文件损坏
     #1#
    Document open(const std::filesystem::path& path);
    
    /**
     * @brief 异步打开 Word 文档
     * @param path 文件路径
     * @return 返回 Document 的 Task
     #1#
    Task<Document> open_async(const std::filesystem::path& path);
    
    /**
     * @brief 创建新的 Word 文档
     * @return 新的 Document 对象
     #1#
    Document create();
}

/**
 * @namespace tinakit::TinaKit
 * @brief 核心库管理功能
 #1#
namespace TinaKit {
    /**
     * @brief 初始化 TinaKit 库
     * @param config 配置选项
     #1#
    void initialize(const Config& config = Config::default_config());
    
    /**
     * @brief 清理 TinaKit 库资源
     #1#
    void cleanup();
    
    /**
     * @brief 获取插件管理器
     * @return 插件管理器引用
     #1#
    PluginManager& plugin_manager();
}

} // namespace tinakit*/

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

namespace tinakit {

// 重新导出核心类型到 tinakit 命名空间
using XmlParser = core::XmlParser;

namespace Excel {
    
    /**
     * @brief 打开 Excel 文件
     * @param path 文件路径
     * @return Workbook 对象
     * @throws FileNotFoundException 文件不存在
     * @throws CorruptedFileException 文件损坏
     * @throws ParseException 解析错误
     */
    excel::Workbook open(const std::filesystem::path& path);
    
    /**
     * @brief 异步打开 Excel 文件
     * @param path 文件路径
     * @return 返回 Workbook 的 Task
     */
    async::Task<excel::Workbook> open_async(const std::filesystem::path& path);
    
    /**
     * @brief 创建新的工作簿
     * @return 新的 Workbook 对象
     */
    excel::Workbook create();

    /**
     * @brief 注册自定义函数
     * @param name 函数名称
     * @param function 函数实现
     */
    void register_function(std::string_view name,
                          std::function<double(const std::vector<double>&)> function);

} // namespace Excel

/**
 * @namespace Word
 * @brief Word 文档处理功能（TODO: 未来实现）
 */
namespace Word {
    // TODO: Word 文档相关功能
} // namespace Word

/**
 * @brief 注册自定义格式处理器
 * @tparam FormatHandler 格式处理器类型
 * @param extension 文件扩展名
 */
template<typename FormatHandler>
void register_format(std::string_view extension);

} // namespace tinakit
