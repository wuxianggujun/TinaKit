/**
 * @file tinakit.hpp
 * @brief TinaKit - Modern OpenXML File Processing C++ Library
 * @version 1.0.0
 * @author TinaKit Team
 * @date 2025-6-16
 *
 */

#pragma once

#include "tinakit/core/types.hpp"
#include "tinakit/core/exceptions.hpp"
#include "tinakit/core/async.hpp"
#include "tinakit/core/xml_parser.hpp"
#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/excel/workbook.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/style_manager.hpp"
#include "tinakit/excel/style.hpp"
#include "tinakit/excel/cell.hpp"
#include "tinakit/excel/row.hpp"
#include <filesystem>
#include <functional>

namespace tinakit {

// 重新导出核心类型到 tinakit 命名空间
using XmlParser = core::XmlParser;

namespace excel {

    /**
     * @brief 注册自定义函数
     * @param name 函数名称
     * @param function 函数实现
     */
    void register_function(std::string_view name,
                          std::function<double(const std::vector<double>&)> function);

} // namespace excel

// 为了兼容性，提供 Excel 命名空间别名（首字母大写）
namespace Excel = excel;

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

/**
 * @example basic_usage.cpp
 * 基本使用示例：
 * @code
 * #include <tinakit/tinakit.hpp>
 * 
 * int main() {
 *     using namespace tinakit;
 *     
 *     // 打开 Excel 文件 - 支持多种方式
 *     auto workbook = excel::Workbook::load("data.xlsx");
 *     // 或者
 *     auto workbook2 = excel::Workbook::load(std::filesystem::path("data.xlsx"));
 *     
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
