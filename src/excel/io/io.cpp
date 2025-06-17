//
// Created by wuxianggujun on 2025/6/17.
//

#include "tinakit/excel/io/io.hpp"
#include <fstream>

tinakit::async::Task<void> tinakit::io::write_file_binary(const std::string& path, const std::vector<std::byte>& data)
{
    std::ofstream file(path, std::ios::binary);

    if (!file)
    {
        throw std::runtime_error("Failed to open file for writing: "+ path);
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (!file)
    {
        throw std::runtime_error("Failed to write data to file: " + path);
    }
    co_return;
}

tinakit::async::Task<std::vector<std::byte>> tinakit::io::read_file_binary(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file for reading: " + path);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<std::byte> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read data from file: " + path);
    }
    co_return buffer;
}
