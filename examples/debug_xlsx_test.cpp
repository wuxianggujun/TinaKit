/**
 * @file debug_xlsx_test.cpp
 * @brief 简单的 XLSX 调试测试程序
 */

#include <iostream>
#include <string>
#include <vector>

#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/core/async.hpp"

using namespace tinakit::core;
using namespace tinakit::async;

// 辅助函数：将字符串转换为字节数组
std::vector<std::byte> string_to_bytes(const std::string& str) {
    std::vector<std::byte> result;
    result.reserve(str.size());
    for (char c : str) {
        result.push_back(static_cast<std::byte>(c));
    }
    return result;
}

// 简单的测试
Task<void> simple_test() {
    std::cout << "=== 开始简单的 XLSX 测试 ===" << std::endl;
    
    try {
        std::cout << "1. 创建内存写入器..." << std::endl;
        auto archiver = OpenXmlArchiver::create_in_memory_writer();
        std::cout << "   ✓ 内存写入器创建成功" << std::endl;
        
        std::cout << "2. 准备测试内容..." << std::endl;
        std::string simple_content = "Hello, World!";
        auto content_bytes = string_to_bytes(simple_content);
        std::cout << "   ✓ 测试内容准备完成，大小: " << content_bytes.size() << " 字节" << std::endl;
        
        std::cout << "3. 添加文件到归档..." << std::endl;
        co_await archiver.add_file("test.txt", std::move(content_bytes));
        std::cout << "   ✓ 文件添加到待处理列表" << std::endl;
        
        std::cout << "4. 保存到内存..." << std::endl;
        auto result = co_await archiver.save_to_memory();
        std::cout << "   ✓ 保存成功，结果大小: " << result.size() << " 字节" << std::endl;
        
        std::cout << "=== 测试完成 ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ 错误: " << e.what() << std::endl;
    }
}

// 测试多文件压缩
Task<void> test_multiple_files() {
    std::cout << "\n=== 测试多文件压缩 ===" << std::endl;

    try {
        auto archiver = OpenXmlArchiver::create_in_memory_writer();

        // 添加多个不同类型的文件
        co_await archiver.add_file("document.txt", string_to_bytes("This is a text document."));
        co_await archiver.add_file("data.json", string_to_bytes(R"({"name": "test", "value": 123})"));
        co_await archiver.add_file("config.xml", string_to_bytes("<?xml version=\"1.0\"?><config></config>"));

        auto result = co_await archiver.save_to_memory();
        std::cout << "   ✓ 多文件压缩成功，总大小: " << result.size() << " 字节" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "❌ 错误: " << e.what() << std::endl;
    }
}

// 主函数
Task<void> run_debug_test() {
    std::cout << "TinaKit XLSX 调试测试" << std::endl;
    std::cout << "===================" << std::endl;
    
    // 运行简单测试
    co_await simple_test();

    // 运行多文件测试
    co_await test_multiple_files();
    
    std::cout << "\n=== 所有测试完成 ===" << std::endl;
}

int main() {
    try {
        auto task = run_debug_test();
        sync_wait(std::move(task));
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    }
}
