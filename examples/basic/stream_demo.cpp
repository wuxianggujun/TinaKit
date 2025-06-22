/**
 * @file stream_demo.cpp
 * @brief OpenXmlArchiver 流式读写示例
 * 
 * 展示如何使用流式接口处理大文件，避免将整个文件加载到内存中
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/core/async.hpp"

using namespace tinakit::core;
using namespace tinakit::async;

// 创建一个大文件用于测试
Task<void> create_large_test_file(const std::string& filename, size_t size_mb) {
    std::cout << "创建 " << size_mb << "MB 的测试文件: " << filename << std::endl;
    
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法创建测试文件: " + filename);
    }
    
    // 创建更多样化的数据块，减少压缩率
    std::string chunk(1024, 'A');

    // 写入指定大小的数据
    size_t chunks_to_write = size_mb * 1024; // 每个块 1KB
    for (size_t i = 0; i < chunks_to_write; ++i) {
        // 创建更随机的内容
        for (size_t j = 0; j < chunk.size(); ++j) {
            // 使用块索引和位置创建伪随机内容
            chunk[j] = static_cast<char>('A' + ((i * 37 + j * 13) % 26));
        }

        // 每隔一段添加一些数字和特殊字符
        if (i % 10 == 0) {
            for (size_t j = 0; j < chunk.size(); j += 50) {
                chunk[j] = static_cast<char>('0' + (i % 10));
            }
        }

        file.write(chunk.data(), chunk.size());
        if (i % 1024 == 0) { // 每 1MB 显示进度
            std::cout << "  已写入: " << (i / 1024) << "MB" << std::endl;
        }
    }
    
    std::cout << "测试文件创建完成!" << std::endl;
    co_return;
}

// 演示流式添加大文件
Task<void> demo_stream_add_large_file() {
    std::cout << "\n=== 演示：流式添加大文件 ===" << std::endl;
    
    try {
        // 创建一个 10MB 的测试文件
        co_await create_large_test_file("large_test.txt", 10);
        
        // 创建归档器
        auto archiver = OpenXmlArchiver::create_in_memory_writer();
        
        // 使用流式接口添加大文件
        std::cout << "使用流式接口添加大文件到归档..." << std::endl;
        std::ifstream large_file("large_test.txt", std::ios::binary);
        if (!large_file) {
            throw std::runtime_error("无法打开测试文件");
        }
        
        // 预估文件大小（可选，有助于性能优化）
        large_file.seekg(0, std::ios::end);
        size_t file_size = large_file.tellg();
        large_file.seekg(0, std::ios::beg);
        
        std::cout << "原始文件大小: " << (file_size / 1024 / 1024) << "MB (" << file_size << " 字节)" << std::endl;

        // 流式添加文件
        co_await archiver.add_file_stream("data/large_file.txt", large_file, file_size);
        large_file.close();

        // 添加一些小文件
        std::string info_content = "这是一个由 TinaKit OpenXmlArchiver 创建的测试归档文件。";
        std::vector<std::byte> info_bytes;
        for (char c : info_content) {
            info_bytes.push_back(static_cast<std::byte>(c));
        }
        co_await archiver.add_file("info.txt", std::move(info_bytes));

        // 保存归档
        std::cout << "保存归档..." << std::endl;
        auto result = co_await archiver.save_to_memory();
        
        std::cout << "归档创建成功!" << std::endl;
        if (result.size() >= 1024 * 1024) {
            std::cout << "压缩后归档大小: " << (result.size() / 1024 / 1024) << "MB (" << result.size() << " 字节)" << std::endl;
        } else if (result.size() >= 1024) {
            std::cout << "压缩后归档大小: " << (result.size() / 1024) << "KB (" << result.size() << " 字节)" << std::endl;
        } else {
            std::cout << "压缩后归档大小: " << result.size() << " 字节" << std::endl;
        }

        // 计算压缩率
        double compression_ratio = (double)result.size() / file_size * 100.0;
        std::cout << "压缩率: " << std::fixed << std::setprecision(1) << compression_ratio << "% (节省了 "
                  << std::setprecision(1) << (100.0 - compression_ratio) << "%)" << std::endl;
        
        // 保存到文件
        std::ofstream output("large_archive.zip", std::ios::binary);
        output.write(reinterpret_cast<const char*>(result.data()), result.size());
        output.close();
        
        std::cout << "归档已保存为: large_archive.zip" << std::endl;
        
        // 清理测试文件
        std::filesystem::remove("large_test.txt");
        
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

// 演示流式读取大文件
Task<void> demo_stream_read_large_file() {
    std::cout << "\n=== 演示：流式读取大文件 ===" << std::endl;
    
    try {
        // 打开之前创建的归档
        auto archiver = co_await OpenXmlArchiver::open_from_file("large_archive.zip");
        
        // 列出文件
        auto files = co_await archiver.list_files();
        std::cout << "归档包含 " << files.size() << " 个文件:" << std::endl;
        for (const auto& file : files) {
            std::cout << "  - " << file << std::endl;
        }
        
        // 使用流式接口读取大文件
        std::cout << "\n使用流式接口提取大文件..." << std::endl;
        std::ofstream extracted_file("extracted_large_file.txt", std::ios::binary);
        if (!extracted_file) {
            throw std::runtime_error("无法创建输出文件");
        }
        
        // 流式读取文件
        co_await archiver.read_file_stream("data/large_file.txt", extracted_file);
        extracted_file.close();
        
        // 验证文件大小
        std::ifstream check_file("extracted_large_file.txt", std::ios::binary | std::ios::ate);
        size_t extracted_size = check_file.tellg();
        check_file.close();
        
        std::cout << "提取完成!" << std::endl;
        if (extracted_size >= 1024 * 1024) {
            std::cout << "提取文件大小: " << (extracted_size / 1024 / 1024) << "MB (" << extracted_size << " 字节)" << std::endl;
        } else if (extracted_size >= 1024) {
            std::cout << "提取文件大小: " << (extracted_size / 1024) << "KB (" << extracted_size << " 字节)" << std::endl;
        } else {
            std::cout << "提取文件大小: " << extracted_size << " 字节" << std::endl;
        }
        
        // 清理文件
        std::filesystem::remove("extracted_large_file.txt");
        std::filesystem::remove("large_archive.zip");
        
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

// 演示内存效率对比
Task<void> demo_memory_efficiency() {
    std::cout << "\n=== 演示：内存效率对比 ===" << std::endl;
    
    try {
        // 创建一个较小的测试文件用于对比
        co_await create_large_test_file("medium_test.txt", 5);
        
        auto archiver = OpenXmlArchiver::create_in_memory_writer();
        
        std::cout << "\n方法1: 传统方式（全部加载到内存）" << std::endl;
        {
            std::ifstream file("medium_test.txt", std::ios::binary);
            std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
            file.close();
            
            std::vector<std::byte> byte_buffer;
            byte_buffer.reserve(buffer.size());
            for (char c : buffer) {
                byte_buffer.push_back(static_cast<std::byte>(c));
            }
            
            std::cout << "内存中缓冲区大小: " << (byte_buffer.size() / 1024 / 1024) << "MB" << std::endl;
            co_await archiver.add_file("traditional/file.txt", std::move(byte_buffer));
        }
        
        std::cout << "\n方法2: 流式方式（分块处理）" << std::endl;
        {
            std::ifstream file("medium_test.txt", std::ios::binary);
            file.seekg(0, std::ios::end);
            size_t file_size = file.tellg();
            file.seekg(0, std::ios::beg);
            
            std::cout << "文件大小: " << (file_size / 1024 / 1024) << "MB" << std::endl;
            std::cout << "使用流式处理，内存使用量显著降低" << std::endl;
            
            co_await archiver.add_file_stream("streaming/file.txt", file, file_size);
            file.close();
        }
        
        // 保存并显示结果
        auto result = co_await archiver.save_to_memory();
        if (result.size() >= 1024 * 1024) {
            std::cout << "\n最终归档大小: " << (result.size() / 1024 / 1024) << "MB (" << result.size() << " 字节)" << std::endl;
        } else if (result.size() >= 1024) {
            std::cout << "\n最终归档大小: " << (result.size() / 1024) << "KB (" << result.size() << " 字节)" << std::endl;
        } else {
            std::cout << "\n最终归档大小: " << result.size() << " 字节" << std::endl;
        }
        
        // 清理
        std::filesystem::remove("medium_test.txt");
        
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

Task<void> main_async() {
    std::cout << "OpenXmlArchiver 流式读写演示" << std::endl;
    std::cout << "==============================" << std::endl;
    
    // 演示流式添加大文件
    co_await demo_stream_add_large_file();
    
    // 演示流式读取大文件
    co_await demo_stream_read_large_file();
    
    // 演示内存效率对比
    co_await demo_memory_efficiency();
    
    std::cout << "\n所有演示完成!" << std::endl;
}

int main() {
    try {
        sync_wait(main_async());
        return 0;
    } catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << std::endl;
        return 1;
    }
}
