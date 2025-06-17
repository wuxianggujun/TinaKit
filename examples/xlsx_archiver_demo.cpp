/**
 * @file xlsx_archiver_demo.cpp
 * @brief TinaKit OpenXmlArchiver 使用示例
 *
 * 本示例展示如何使用 OpenXmlArchiver 类来处理 OpenXML 格式文件：
 * 1. 读取现有的 xlsx 文件
 * 2. 列出文件内容
 * 3. 读取特定文件内容
 * 4. 添加新文件
 * 5. 删除文件
 * 6. 保存修改后的文件
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/core/async.hpp"
#include "../include/tinakit/core/io.hpp"

using namespace tinakit::core;
using namespace tinakit::async;

// 辅助函数：将字节数组转换为字符串（用于显示文本内容）
std::string bytes_to_string(const std::vector<std::byte>& bytes) {
    std::string result;
    result.reserve(bytes.size());
    for (const auto& byte : bytes) {
        result.push_back(static_cast<char>(byte));
    }
    return result;
}

// 辅助函数：将字符串转换为字节数组
std::vector<std::byte> string_to_bytes(const std::string& str) {
    std::vector<std::byte> result;
    result.reserve(str.size());
    for (char c : str) {
        result.push_back(static_cast<std::byte>(c));
    }
    return result;
}

// 辅助函数：创建一个简单的测试 xlsx 文件内容
Task<void> create_sample_xlsx(const std::string& filename) {
    std::cout << "\n=== 创建示例 XLSX 文件 ===" << std::endl;
    
    auto archiver = OpenXmlArchiver::create_in_memory_writer();
    
    // 添加 [Content_Types].xml
    std::string content_types = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
    <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
    <Default Extension="xml" ContentType="application/xml"/>
    <Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
    <Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
</Types>)";
    
    co_await archiver.add_file("[Content_Types].xml", string_to_bytes(content_types));
    
    // 添加 _rels/.rels
    std::string rels = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
    <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>)";
    
    co_await archiver.add_file("_rels/.rels", string_to_bytes(rels));
    
    // 添加 xl/workbook.xml
    std::string workbook = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
    <sheets>
        <sheet name="Sheet1" sheetId="1" r:id="rId1"/>
    </sheets>
</workbook>)";
    
    co_await archiver.add_file("xl/workbook.xml", string_to_bytes(workbook));
    
    // 添加 xl/worksheets/sheet1.xml
    std::string worksheet = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
    <sheetData>
        <row r="1">
            <c r="A1" t="inlineStr"><is><t>Hello</t></is></c>
            <c r="B1" t="inlineStr"><is><t>World</t></is></c>
        </row>
        <row r="2">
            <c r="A2" t="inlineStr"><is><t>TinaKit</t></is></c>
            <c r="B2" t="inlineStr"><is><t>Demo</t></is></c>
        </row>
    </sheetData>
</worksheet>)";
    
    co_await archiver.add_file("xl/worksheets/sheet1.xml", string_to_bytes(worksheet));
    
    // 保存到文件
    co_await archiver.save_to_file(filename);
    std::cout << "示例 XLSX 文件已创建: " << filename << std::endl;
}

// 示例1：读取并列出 xlsx 文件内容
Task<void> demo_list_files(const std::string& filename) {
    std::cout << "\n=== 示例1: 列出 XLSX 文件内容 ===" << std::endl;
    
    try {
        auto archiver = co_await OpenXmlArchiver::open_from_file(filename);
        auto files = co_await archiver.list_files();
        
        std::cout << "文件包含 " << files.size() << " 个条目:" << std::endl;
        for (const auto& file : files) {
            std::cout << "  - " << file << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

// 示例2：读取特定文件内容
Task<void> demo_read_file_content(const std::string& filename) {
    std::cout << "\n=== 示例2: 读取特定文件内容 ===" << std::endl;
    
    try {
        auto archiver = co_await OpenXmlArchiver::open_from_file(filename);
        
        // 读取工作表内容
        std::string target_file = "xl/worksheets/sheet1.xml";
        bool has_file = co_await archiver.has_file(target_file);
        
        if (has_file) {
            auto content = co_await archiver.read_file(target_file);
            std::string content_str = bytes_to_string(content);
            
            std::cout << "文件 '" << target_file << "' 内容 (" << content.size() << " 字节):" << std::endl;
            std::cout << content_str << std::endl;
        } else {
            std::cout << "文件 '" << target_file << "' 不存在" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

// 示例3：修改 xlsx 文件（添加和删除文件）
Task<void> demo_modify_xlsx(const std::string& input_filename, const std::string& output_filename) {
    std::cout << "\n=== 示例3: 修改 XLSX 文件 ===" << std::endl;
    
    try {
        auto archiver = co_await OpenXmlArchiver::open_from_file(input_filename);
        
        // 添加自定义文件
        std::string custom_content = R"(<?xml version="1.0" encoding="UTF-8"?>
<customData>
    <info>This is a custom file added by TinaKit</info>
    <timestamp>)" + std::to_string(std::time(nullptr)) + R"(</timestamp>
</customData>)";
        
        co_await archiver.add_file("custom/metadata.xml", string_to_bytes(custom_content));
        std::cout << "已添加自定义文件: custom/metadata.xml" << std::endl;
        
        // 列出修改后的文件
        auto files = co_await archiver.list_files();
        std::cout << "修改后的文件列表 (" << files.size() << " 个文件):" << std::endl;
        for (const auto& file : files) {
            std::cout << "  - " << file << std::endl;
        }
        
        // 保存修改后的文件
        co_await archiver.save_to_file(output_filename);
        std::cout << "修改后的文件已保存为: " << output_filename << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

// 主函数
Task<void> run_demo() {
    std::cout << "TinaKit OpenXmlArchiver 使用示例" << std::endl;
    std::cout << "==============================" << std::endl;
    
    const std::string sample_file = "sample.xlsx";
    const std::string modified_file = "modified_sample.xlsx";
    
    // 创建示例文件
    co_await create_sample_xlsx(sample_file);
    
    // 运行各种示例
    co_await demo_list_files(sample_file);
    co_await demo_read_file_content(sample_file);
    co_await demo_modify_xlsx(sample_file, modified_file);
    
    std::cout << "\n=== 演示完成 ===" << std::endl;
    std::cout << "生成的文件:" << std::endl;
    std::cout << "  - " << sample_file << " (原始示例文件)" << std::endl;
    std::cout << "  - " << modified_file << " (修改后的文件)" << std::endl;
}

int main() {
    try {
        auto task = run_demo();
        sync_wait(std::move(task));
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    }
}
