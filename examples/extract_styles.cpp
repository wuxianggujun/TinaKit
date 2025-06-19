#include <iostream>
#include <fstream>
#include <string>
#include <memory>

// minizip-ng headers
#include "mz.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"
#include "mz_strm.h"
#include "mz_strm_mem.h"

void extract_styles_xml(const std::string& xlsx_file) {
    std::cout << "=== 提取 " << xlsx_file << " 中的 styles.xml ===" << std::endl;
    
    // 读取XLSX文件
    std::ifstream file(xlsx_file, std::ios::binary);
    if (!file) {
        std::cout << "无法打开文件: " << xlsx_file << std::endl;
        return;
    }
    
    // 读取文件内容到内存
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<char> file_data(file_size);
    file.read(file_data.data(), file_size);
    file.close();
    
    // 创建ZIP读取器
    void* reader = mz_zip_reader_create();
    void* mem_stream = mz_stream_mem_create();
    
    if (mz_stream_mem_set_buffer(mem_stream, file_data.data(), file_size) != MZ_OK) {
        std::cout << "设置内存缓冲区失败" << std::endl;
        return;
    }
    
    if (mz_stream_mem_open(mem_stream, nullptr, MZ_OPEN_MODE_READ) != MZ_OK) {
        std::cout << "打开内存流失败" << std::endl;
        return;
    }
    
    if (mz_zip_reader_open(reader, mem_stream) != MZ_OK) {
        std::cout << "打开ZIP读取器失败" << std::endl;
        return;
    }
    
    // 查找styles.xml
    if (mz_zip_reader_locate_entry(reader, "xl/styles.xml", 0) != MZ_OK) {
        std::cout << "找不到 xl/styles.xml" << std::endl;
        return;
    }
    
    // 获取文件信息
    mz_zip_file* file_info = nullptr;
    if (mz_zip_reader_entry_get_info(reader, &file_info) != MZ_OK || !file_info) {
        std::cout << "获取文件信息失败" << std::endl;
        return;
    }
    
    // 打开条目
    if (mz_zip_reader_entry_open(reader) != MZ_OK) {
        std::cout << "打开条目失败" << std::endl;
        return;
    }
    
    // 读取内容
    std::vector<char> content(file_info->uncompressed_size + 1, 0);
    int32_t bytes_read = mz_zip_reader_entry_read(reader, content.data(), file_info->uncompressed_size);
    
    if (bytes_read > 0) {
        std::cout << "\n=== styles.xml 内容 ===" << std::endl;
        std::cout << content.data() << std::endl;
        std::cout << "\n=== 内容结束 ===" << std::endl;
    } else {
        std::cout << "读取内容失败" << std::endl;
    }
    
    // 清理
    mz_zip_reader_entry_close(reader);
    mz_zip_reader_delete(&reader);
    mz_stream_mem_delete(&mem_stream);
}

int main() {
    try {
        extract_styles_xml("test_data_modified.xlsx");
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
