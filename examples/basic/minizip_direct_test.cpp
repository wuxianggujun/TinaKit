/**
 * @file minizip_direct_test.cpp
 * @brief 直接测试 minizip-ng API 的基本功能
 */

#include <iostream>
#include <string>
#include <vector>
#include <ctime>

extern "C" {
#include <mz.h>
#include <mz_zip.h>
#include <mz_strm.h>
#include <mz_strm_mem.h>
#include <mz_zip_rw.h>
}

int main() {
    std::cout << "=== 直接测试 minizip-ng API ===" << std::endl;
    
    try {
        // 1. 创建内存流
        std::cout << "1. 创建内存流..." << std::endl;
        void* mem_stream = mz_stream_mem_create();
        if (!mem_stream) {
            std::cout << "❌ 创建内存流失败" << std::endl;
            return 1;
        }
        std::cout << "   ✓ 内存流创建成功: " << mem_stream << std::endl;

        // 1.5. 打开内存流
        std::cout << "1.5. 打开内存流..." << std::endl;
        int32_t open_status = mz_stream_mem_open(mem_stream, nullptr, MZ_OPEN_MODE_CREATE);
        if (open_status != MZ_OK) {
            std::cout << "❌ 打开内存流失败，状态: " << open_status << std::endl;
            mz_stream_mem_delete(&mem_stream);
            return 1;
        }
        std::cout << "   ✓ 内存流打开成功" << std::endl;
        
        // 2. 创建写入器
        std::cout << "2. 创建写入器..." << std::endl;
        void* writer = mz_zip_writer_create();
        if (!writer) {
            std::cout << "❌ 创建写入器失败" << std::endl;
            mz_stream_mem_delete(&mem_stream);
            return 1;
        }
        std::cout << "   ✓ 写入器创建成功: " << writer << std::endl;
        
        // 3. 打开写入器
        std::cout << "3. 打开写入器..." << std::endl;
        int32_t status = mz_zip_writer_open(writer, mem_stream, 0);
        if (status != MZ_OK) {
            std::cout << "❌ 打开写入器失败，状态: " << status << std::endl;
            mz_zip_writer_delete(&writer);
            mz_stream_mem_delete(&mem_stream);
            return 1;
        }
        std::cout << "   ✓ 写入器打开成功" << std::endl;
        
        // 4. 准备文件数据
        std::cout << "4. 准备文件数据..." << std::endl;
        std::string content = "Hello, minizip-ng!";
        std::cout << "   ✓ 文件内容: \"" << content << "\" (" << content.size() << " 字节)" << std::endl;
        
        // 5. 设置文件信息（最简配置）
        std::cout << "5. 设置文件信息..." << std::endl;
        mz_zip_file file_info = {};

        // 只设置最基本的信息
        file_info.filename = "test.txt";
        file_info.uncompressed_size = content.size();
        file_info.compression_method = MZ_COMPRESS_METHOD_STORE; // 先尝试不压缩

        std::cout << "   文件名: " << file_info.filename << std::endl;
        std::cout << "   大小: " << file_info.uncompressed_size << std::endl;
        std::cout << "   压缩方法: " << file_info.compression_method << " (STORE)" << std::endl;
        
        // 6. 打开条目
        std::cout << "6. 打开条目..." << std::endl;
        status = mz_zip_writer_entry_open(writer, &file_info);
        if (status != MZ_OK) {
            std::cout << "❌ 打开条目失败，状态: " << status << std::endl;

            // 尝试获取更多错误信息
            std::cout << "   调试信息:" << std::endl;
            std::cout << "   - 写入器指针: " << writer << std::endl;
            std::cout << "   - 文件信息指针: " << &file_info << std::endl;
            std::cout << "   - 文件名指针: " << file_info.filename << std::endl;

            mz_zip_writer_close(writer);
            mz_zip_writer_delete(&writer);
            mz_stream_mem_delete(&mem_stream);
            return 1;
        }
        std::cout << "   ✓ 条目打开成功" << std::endl;
        std::cout << "   ✓ 条目打开成功" << std::endl;
        
        // 7. 写入数据
        std::cout << "7. 写入数据..." << std::endl;
        int32_t bytes_written = mz_zip_writer_entry_write(writer, content.data(), static_cast<int32_t>(content.size()));
        if (bytes_written < 0) {
            std::cout << "❌ 写入数据失败，状态: " << bytes_written << std::endl;
            mz_zip_writer_entry_close(writer);
            mz_zip_writer_close(writer);
            mz_zip_writer_delete(&writer);
            mz_stream_mem_delete(&mem_stream);
            return 1;
        }
        std::cout << "   ✓ 数据写入成功，写入了 " << bytes_written << " 字节" << std::endl;
        
        // 8. 关闭条目
        std::cout << "8. 关闭条目..." << std::endl;
        status = mz_zip_writer_entry_close(writer);
        if (status != MZ_OK) {
            std::cout << "❌ 关闭条目失败，状态: " << status << std::endl;
            mz_zip_writer_close(writer);
            mz_zip_writer_delete(&writer);
            mz_stream_mem_delete(&mem_stream);
            return 1;
        }
        std::cout << "   ✓ 条目关闭成功" << std::endl;
        
        // 9. 关闭写入器
        std::cout << "9. 关闭写入器..." << std::endl;
        status = mz_zip_writer_close(writer);
        if (status != MZ_OK) {
            std::cout << "❌ 关闭写入器失败，状态: " << status << std::endl;
            mz_zip_writer_delete(&writer);
            mz_stream_mem_delete(&mem_stream);
            return 1;
        }
        std::cout << "   ✓ 写入器关闭成功" << std::endl;
        
        // 10. 获取结果
        std::cout << "10. 获取结果..." << std::endl;
        const void* buffer_ptr = nullptr;
        mz_stream_mem_get_buffer(mem_stream, &buffer_ptr);
        int32_t buffer_len = 0;
        mz_stream_mem_get_buffer_length(mem_stream, &buffer_len);
        
        if (!buffer_ptr || buffer_len <= 0) {
            std::cout << "❌ 获取缓冲区失败" << std::endl;
            std::cout << "   缓冲区指针: " << buffer_ptr << std::endl;
            std::cout << "   缓冲区长度: " << buffer_len << std::endl;
        } else {
            std::cout << "   ✓ 获取缓冲区成功" << std::endl;
            std::cout << "   缓冲区指针: " << buffer_ptr << std::endl;
            std::cout << "   缓冲区长度: " << buffer_len << " 字节" << std::endl;
        }
        
        // 11. 清理资源
        std::cout << "11. 清理资源..." << std::endl;
        mz_zip_writer_delete(&writer);
        mz_stream_mem_delete(&mem_stream);
        std::cout << "   ✓ 资源清理完成" << std::endl;
        
        std::cout << "\n=== 测试成功完成 ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "❌ 异常: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "❌ 未知异常" << std::endl;
        return 1;
    }
}
