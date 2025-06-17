/**
 * @file xlsx_reader_test.cpp
 * @brief 简单的 XLSX 文件读取测试
 * 
 * 这个程序用于测试读取现有的 xlsx 文件，展示如何：
 * 1. 打开 xlsx 文件
 * 2. 列出所有内部文件
 * 3. 读取关键的 XML 文件内容
 * 4. 分析 xlsx 文件结构
 */

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

#include "tinakit/tinakit.hpp"

using namespace tinakit::io;
using namespace tinakit::async;

// 辅助函数：将字节数组转换为字符串
std::string bytes_to_string(const std::vector<std::byte>& bytes) {
    std::string result;
    result.reserve(bytes.size());
    for (const auto& byte : bytes) {
        result.push_back(static_cast<char>(byte));
    }
    return result;
}

// 辅助函数：显示文件内容的前几行
void print_file_preview(const std::string& filename, const std::string& content, size_t max_lines = 10) {
    std::cout << "\n--- " << filename << " (前 " << max_lines << " 行) ---" << std::endl;
    
    std::istringstream iss(content);
    std::string line;
    size_t line_count = 0;
    
    while (std::getline(iss, line) && line_count < max_lines) {
        std::cout << std::setw(3) << (line_count + 1) << ": " << line << std::endl;
        line_count++;
    }
    
    if (line_count == max_lines) {
        std::cout << "... (内容被截断)" << std::endl;
    }
}

// 分析 xlsx 文件结构
Task<void> analyze_xlsx_structure(const std::string& xlsx_path) {
    std::cout << "正在分析 XLSX 文件: " << xlsx_path << std::endl;
    std::cout << "================================================" << std::endl;
    
    try {
        // 打开文件
        auto archiver = co_await XlsxArchiver::open_from_file(xlsx_path);
        std::cout << "✓ 文件打开成功" << std::endl;
        
        // 列出所有文件
        auto files = co_await archiver.list_files();
        std::cout << "\n📁 文件结构 (共 " << files.size() << " 个文件):" << std::endl;
        
        // 按类型分组显示文件
        std::vector<std::string> xml_files;
        std::vector<std::string> rels_files;
        std::vector<std::string> other_files;
        
        for (const auto& file : files) {
            std::cout << "  📄 " << file << std::endl;
            
            if (file.ends_with(".xml")) {
                xml_files.push_back(file);
            } else if (file.ends_with(".rels")) {
                rels_files.push_back(file);
            } else {
                other_files.push_back(file);
            }
        }
        
        // 读取关键文件内容
        std::cout << "\n📖 关键文件内容分析:" << std::endl;
        
        // 1. Content Types
        if (co_await archiver.has_file("[Content_Types].xml")) {
            auto content = co_await archiver.read_file("[Content_Types].xml");
            print_file_preview("[Content_Types].xml", bytes_to_string(content), 15);
        }
        
        // 2. 主关系文件
        if (co_await archiver.has_file("_rels/.rels")) {
            auto content = co_await archiver.read_file("_rels/.rels");
            print_file_preview("_rels/.rels", bytes_to_string(content), 10);
        }
        
        // 3. 工作簿文件
        if (co_await archiver.has_file("xl/workbook.xml")) {
            auto content = co_await archiver.read_file("xl/workbook.xml");
            print_file_preview("xl/workbook.xml", bytes_to_string(content), 15);
        }
        
        // 4. 第一个工作表
        if (co_await archiver.has_file("xl/worksheets/sheet1.xml")) {
            auto content = co_await archiver.read_file("xl/worksheets/sheet1.xml");
            print_file_preview("xl/worksheets/sheet1.xml", bytes_to_string(content), 20);
        }
        
        // 5. 共享字符串（如果存在）
        if (co_await archiver.has_file("xl/sharedStrings.xml")) {
            auto content = co_await archiver.read_file("xl/sharedStrings.xml");
            print_file_preview("xl/sharedStrings.xml", bytes_to_string(content), 15);
        }
        
        // 统计信息
        std::cout << "\n📊 文件统计:" << std::endl;
        std::cout << "  XML 文件: " << xml_files.size() << " 个" << std::endl;
        std::cout << "  关系文件: " << rels_files.size() << " 个" << std::endl;
        std::cout << "  其他文件: " << other_files.size() << " 个" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ 错误: " << e.what() << std::endl;
    }
}

// 主函数
Task<void> run_test() {
    std::cout << "TinaKit XLSX 文件读取测试" << std::endl;
    std::cout << "=========================" << std::endl;
    
    // 可以测试的文件路径
    std::vector<std::string> test_files = {
        "test.xlsx",           // 当前目录
        "sample.xlsx",         // 示例文件
        "examples/test.xlsx",  // examples 目录
        "../test.xlsx"         // 上级目录
    };
    
    std::string xlsx_file;
    bool found = false;
    
    // 查找可用的测试文件
    for (const auto& file : test_files) {
        if (std::filesystem::exists(file)) {
            xlsx_file = file;
            found = true;
            break;
        }
    }
    
    if (!found) {
        std::cout << "❌ 未找到测试用的 XLSX 文件" << std::endl;
        std::cout << "\n请将一个 XLSX 文件放在以下位置之一:" << std::endl;
        for (const auto& file : test_files) {
            std::cout << "  - " << file << std::endl;
        }
        std::cout << "\n或者运行 xlsx_archiver_demo 程序先创建示例文件。" << std::endl;
        co_return;
    }
    
    // 分析找到的文件
    co_await analyze_xlsx_structure(xlsx_file);
    
    std::cout << "\n✅ 测试完成!" << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        // 如果提供了命令行参数，使用指定的文件
        if (argc > 1) {
            std::string xlsx_file = argv[1];
            if (!std::filesystem::exists(xlsx_file)) {
                std::cerr << "错误: 文件不存在: " << xlsx_file << std::endl;
                return 1;
            }
            
            auto task = analyze_xlsx_structure(xlsx_file);
            sync_wait(std::move(task));
        } else {
            auto task = run_test();
            sync_wait(std::move(task));
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    }
}
