//
// Created by wuxianggujun on 2025/6/17.
//

#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/core/io.hpp"
#include "tinakit/core/exceptions.hpp"
#include <iostream>

extern "C" {
#include <mz.h>
#include <mz_zip.h>
#include <mz_strm.h>
#include <mz_strm_mem.h>
#include <mz_zip_rw.h>
}

namespace tinakit::core
{
    OpenXmlArchiver::~OpenXmlArchiver()
    {
        close_handles();
    }

    async::Task<OpenXmlArchiver> OpenXmlArchiver::open_from_file(const std::string& path)
    {
        OpenXmlArchiver archiver;
        archiver.reader_handle_.reset(mz_zip_reader_create());
        if (!archiver.reader_handle_)
        {
            throw TinaKitException("Failed to create zip reader handle.", "OpenXmlArchiver::open_from_file");
        }

        archiver.source_buffer_ = co_await core::read_file_binary(path);

        // 使用 mz_zip_reader_open_buffer 而不是创建临时流
        if (int32_t status = mz_zip_reader_open_buffer(archiver.reader_handle_.get(),
                                                       reinterpret_cast<uint8_t*>(archiver.source_buffer_.data()),
                                                       static_cast<int32_t>(archiver.source_buffer_.size()), 0);
            status != MZ_OK)
        {
            throw TinaKitException("Failed to open zip archive from buffer. Status: " + std::to_string(status),
                                   "OpenXmlArchiver::open_from_file");
        }

        co_await archiver.list_files();
        co_return archiver;
    }

    OpenXmlArchiver OpenXmlArchiver::open_from_memory(std::vector<std::byte> buffer)
    {
        OpenXmlArchiver archiver;
        archiver.source_buffer_ = std::move(buffer);
        if (archiver.source_buffer_.empty())
        {
            return create_in_memory_writer();
        }

        archiver.reader_handle_.reset(mz_zip_reader_create());
        if (!archiver.reader_handle_)
        {
            throw TinaKitException("Failed to create zip reader handle.", "OpenXmlArchiver::open_from_memory");
        }

        // 使用 mz_zip_reader_open_buffer 而不是创建临时流
        if (int32_t status = mz_zip_reader_open_buffer(archiver.reader_handle_.get(),
                                                       reinterpret_cast<uint8_t*>(archiver.source_buffer_.data()),
                                                       static_cast<int32_t>(archiver.source_buffer_.size()), 0);
            status != MZ_OK)
        {
            throw TinaKitException("Failed to open zip archive from buffer. Status: " + std::to_string(status),
                                   "OpenXmlArchiver::open_from_memory");
        }
        async::sync_wait(archiver.list_files());

        return archiver;
    }

    OpenXmlArchiver OpenXmlArchiver::create_in_memory_writer()
    {
        OpenXmlArchiver archiver;
        archiver.writer_handle_.reset(mz_zip_writer_create());
        if (!archiver.writer_handle_)
        {
            throw TinaKitException("Failed to create zip writer handle.", "OpenXmlArchiver::create_in_memory_writer");
        }

        archiver.memory_stream_handle_.reset(mz_stream_mem_create());
        if (!archiver.memory_stream_handle_)
        {
            throw TinaKitException("Failed to create memory stream for writer.",
                                   "OpenXmlArchiver::create_in_memory_writer");
        }

        // 关键修复：先打开内存流
        if (int32_t status = mz_stream_mem_open(archiver.memory_stream_handle_.get(), nullptr, MZ_OPEN_MODE_CREATE);
            status != MZ_OK)
        {
            throw TinaKitException("Failed to open memory stream. Status: " + std::to_string(status),
                                   "OpenXmlArchiver::create_in_memory_writer");
        }

        if (int32_t status = mz_zip_writer_open(archiver.writer_handle_.get(), archiver.memory_stream_handle_.get(), 0);
            status != MZ_OK)
        {
            throw TinaKitException("Failed to open zip writer on memory stream. Status: " + std::to_string(status),
                                   "OpenXmlArchiver::create_in_memory_writer");
        }

        return archiver;
    }

    async::Task<std::vector<std::string>> OpenXmlArchiver::list_files() const
    {
        if (writer_handle_ || !files_to_remove_.empty() || !pending_new_files_.empty())
        {
            std::vector<std::string> files(current_files_.begin(), current_files_.end());
            co_return files;
        }

        if (!reader_handle_)
        {
            co_return std::vector<std::string>();
        }


        std::set<std::string> files;
        if (int32_t status = mz_zip_reader_goto_first_entry(reader_handle_.get()); status == MZ_OK)
        {
            do
            {
                mz_zip_file* file_info = nullptr;

                if (mz_zip_reader_entry_get_info(reader_handle_.get(), &file_info) == MZ_OK && file_info)
                {
                    files.insert(file_info->filename);
                }
            }
            while (mz_zip_reader_goto_next_entry(reader_handle_.get()) == MZ_OK);
        }

        const_cast<OpenXmlArchiver*>(this)->current_files_ = files;
        co_return std::vector<std::string>(files.begin(), files.end());
    }

    async::Task<bool> OpenXmlArchiver::has_file(const std::string& filename) const
    {
        if (current_files_.empty())
        {
            co_await list_files();
        }
        co_return current_files_.count(filename) > 0;
    }

    async::Task<std::vector<std::byte>> OpenXmlArchiver::read_file(const std::string& filename)
    {
        // 如果文件是新添加且尚未保存的，则从待处理映射中读取
        if (auto it = pending_new_files_.find(filename); it != pending_new_files_.end())
        {
            co_return it->second;
        }

        // 无法从写入器句柄读取。如果处于写入模式，则原始读取器已消失。
        if (writer_handle_)
        {
            throw TinaKitException("Cannot read file content after archive has been modified.",
                                   "OpenXmlArchiver.read_file");
        }

        if (!reader_handle_)
        {
            throw TinaKitException("Archive is not open for reading.", "OpenXmlArchiver.read_file");
        }

        if (int32_t status = mz_zip_reader_locate_entry(reader_handle_.get(), filename.c_str(), 0); status != MZ_OK)
        {
            throw TinaKitException("File not found in archive: " + filename, "OpenXmlArchiver.read_file");
        }

        mz_zip_file* file_info = nullptr;
        if (mz_zip_reader_entry_get_info(reader_handle_.get(), &file_info) != MZ_OK || !file_info) {
            throw TinaKitException("Failed to get file info for: " + filename, "OpenXmlArchiver.read_file");
        }

        // 打开条目进行读取
        if (int32_t status = mz_zip_reader_entry_open(reader_handle_.get()); status != MZ_OK) {
            throw TinaKitException("Failed to open entry for reading: " + filename + ". Status: " + std::to_string(status), "OpenXmlArchiver.read_file");
        }

        std::vector<std::byte> buffer(file_info->uncompressed_size);

        try {
            if (int32_t status = mz_zip_reader_entry_read(reader_handle_.get(), buffer.data(), static_cast<int32_t>(buffer.size())); status < 0)
            {
                throw TinaKitException(
                    "Failed to read file content for: " + filename + ". Status: " + std::to_string(status),
                    "OpenXmlArchiver.read_file");
            }
        } catch (...) {
            // 确保关闭条目
            mz_zip_reader_entry_close(reader_handle_.get());
            throw;
        }

        // 关闭条目
        if (int32_t status = mz_zip_reader_entry_close(reader_handle_.get()); status != MZ_OK) {
            throw TinaKitException("Failed to close entry after reading: " + filename + ". Status: " + std::to_string(status), "OpenXmlArchiver.read_file");
        }

        co_return buffer;
    }

    async::Task<void> OpenXmlArchiver::add_file(const std::string& filename, std::vector<std::byte> content)
    {
        // 只有在没有写入器的情况下才需要转换模式
        // 如果已经有写入器（比如通过 create_in_memory_writer 创建），就不要重新创建
        if (!writer_handle_) {
            co_await transition_to_writer_mode_if_needed();
        }

        pending_new_files_[filename] = std::move(content);
        current_files_.insert(filename);
        files_to_remove_.erase(filename);
        co_return;
    }

    async::Task<void> OpenXmlArchiver::add_file_stream(const std::string& filename, std::istream& stream, size_t estimated_size)
    {
        // 只有在没有写入器的情况下才需要转换模式
        if (!writer_handle_) {
            co_await transition_to_writer_mode_if_needed();
        }

        // 读取流内容到缓冲区
        std::vector<std::byte> content;
        if (estimated_size > 0) {
            content.reserve(estimated_size);
        }

        // 分块读取流内容
        constexpr size_t chunk_size = 64 * 1024; // 64KB 块
        std::vector<char> buffer(chunk_size);

        while (stream.good()) {
            stream.read(buffer.data(), chunk_size);
            std::streamsize bytes_read = stream.gcount();
            if (bytes_read > 0) {
                const auto* byte_data = reinterpret_cast<const std::byte*>(buffer.data());
                content.insert(content.end(), byte_data, byte_data + bytes_read);
            }
        }

        // 使用现有的 add_file 方法
        pending_new_files_[filename] = std::move(content);
        current_files_.insert(filename);
        files_to_remove_.erase(filename);

        co_return;
    }

    async::Task<void> OpenXmlArchiver::read_file_stream(const std::string& filename, std::ostream& stream)
    {
        // 如果文件是新添加且尚未保存的，则从待处理映射中读取
        if (auto it = pending_new_files_.find(filename); it != pending_new_files_.end())
        {
            const auto& content = it->second;
            const auto* char_data = reinterpret_cast<const char*>(content.data());
            stream.write(char_data, static_cast<std::streamsize>(content.size()));
            co_return;
        }

        // 无法从写入器句柄读取
        if (writer_handle_)
        {
            throw TinaKitException("Cannot read file content after archive has been modified.",
                                   "OpenXmlArchiver.read_file_stream");
        }

        if (!reader_handle_)
        {
            throw TinaKitException("Archive is not open for reading.", "OpenXmlArchiver.read_file_stream");
        }

        if (int32_t status = mz_zip_reader_locate_entry(reader_handle_.get(), filename.c_str(), 0); status != MZ_OK)
        {
            throw TinaKitException("File not found in archive: " + filename, "OpenXmlArchiver.read_file_stream");
        }

        mz_zip_file* file_info = nullptr;
        if (mz_zip_reader_entry_get_info(reader_handle_.get(), &file_info) != MZ_OK || !file_info) {
            throw TinaKitException("Failed to get file info for: " + filename, "OpenXmlArchiver.read_file_stream");
        }

        // 打开条目进行读取
        if (int32_t status = mz_zip_reader_entry_open(reader_handle_.get()); status != MZ_OK) {
            throw TinaKitException("Failed to open entry for reading: " + filename + ". Status: " + std::to_string(status), "OpenXmlArchiver.read_file_stream");
        }

        try {
            // 分块读取并写入流
            constexpr size_t chunk_size = 64 * 1024; // 64KB 块
            std::vector<std::byte> buffer(chunk_size);

            size_t total_read = 0;
            size_t remaining = file_info->uncompressed_size;

            while (remaining > 0) {
                size_t to_read = std::min(chunk_size, remaining);

                int32_t bytes_read = mz_zip_reader_entry_read(reader_handle_.get(), buffer.data(), static_cast<int32_t>(to_read));
                if (bytes_read < 0) {
                    throw TinaKitException(
                        "Failed to read file content for: " + filename + ". Status: " + std::to_string(bytes_read),
                        "OpenXmlArchiver.read_file_stream");
                }

                if (bytes_read == 0) {
                    break; // 文件结束
                }

                // 写入到输出流
                const auto* char_data = reinterpret_cast<const char*>(buffer.data());
                stream.write(char_data, bytes_read);

                total_read += bytes_read;
                remaining -= bytes_read;
            }
        } catch (...) {
            // 确保关闭条目
            mz_zip_reader_entry_close(reader_handle_.get());
            throw;
        }

        // 关闭条目
        if (int32_t status = mz_zip_reader_entry_close(reader_handle_.get()); status != MZ_OK) {
            throw TinaKitException("Failed to close entry after reading: " + filename + ". Status: " + std::to_string(status), "OpenXmlArchiver.read_file_stream");
        }

        co_return;
    }

    async::Task<void> OpenXmlArchiver::remove_file(const std::string& filename)
    {
        if (current_files_.empty())
        {
            co_await list_files();
        }

        if (current_files_.count(filename))
        {
            files_to_remove_.insert(filename);
            current_files_.erase(filename);
            pending_new_files_.erase(filename);
        }
        co_return;
    }

    async::Task<void> OpenXmlArchiver::save_to_file(const std::string& path)
    {
        auto memory_buffer = co_await save_to_memory();

        co_await core::write_file_binary(path, memory_buffer);

        // 保存后重新初始化 reader，以便继续读取
        if (writer_handle_) {
            // 释放 writer
            writer_handle_.reset();

            // 清空待处理文件映射，因为它们已经保存到文件中
            pending_new_files_.clear();

            // 清空当前文件列表，强制重新扫描
            current_files_.clear();

            // 清空其他状态
            files_to_remove_.clear();

            // 使用刚保存的内容重新创建 reader
            reader_handle_.reset(mz_zip_reader_create());
            if (reader_handle_) {
                source_buffer_ = std::move(memory_buffer);  // 更新源缓冲区

                // 重新打开 reader
                if (int32_t status = mz_zip_reader_open_buffer(reader_handle_.get(),
                                                               reinterpret_cast<uint8_t*>(source_buffer_.data()),
                                                               static_cast<int32_t>(source_buffer_.size()), 0);
                    status != MZ_OK) {
                    reader_handle_.reset();  // 失败时清理
                    throw TinaKitException("Failed to reinitialize reader after save. Status: " + std::to_string(status),
                                           "OpenXmlArchiver::save_to_file");
                }

                // 重新扫描文件列表
                co_await list_files();
            } else {
                throw TinaKitException("Failed to create reader handle after save.", "OpenXmlArchiver::save_to_file");
            }
        }
    }


    async::Task<std::vector<std::byte>> OpenXmlArchiver::save_to_memory()
    {
        if (!writer_handle_ && pending_new_files_.empty() && files_to_remove_.empty())
        {
            if (reader_handle_)
            {
                co_return source_buffer_;
            }
            co_return std::vector<std::byte>();
        }

        co_await transition_to_writer_mode_if_needed();

        // 检查写入器状态
        if (!writer_handle_) {
            throw TinaKitException("Writer handle is null", "OpenXmlArchiver.save_to_memory");
        }

        if (!memory_stream_handle_) {
            throw TinaKitException("Memory stream handle is null", "OpenXmlArchiver.save_to_memory");
        }

        for (auto const& [filename, content] : pending_new_files_) {
            // 设置文件信息
            mz_zip_file file_info = {};
            file_info.filename = filename.c_str();
            file_info.uncompressed_size = static_cast<uint64_t>(content.size());
            file_info.compression_method = MZ_COMPRESS_METHOD_DEFLATE;

            int32_t status = mz_zip_writer_entry_open(writer_handle_.get(), &file_info);
            if (status != MZ_OK) {
                if (status == -109 && file_info.compression_method == MZ_COMPRESS_METHOD_DEFLATE) {
                    // DEFLATE 不支持，回退到 STORE 模式
                    file_info.compression_method = MZ_COMPRESS_METHOD_STORE;
                    status = mz_zip_writer_entry_open(writer_handle_.get(), &file_info);
                }

                if (status != MZ_OK) {
                    throw TinaKitException("Failed to open entry for file '" + filename + "'. Status: " + std::to_string(status), "OpenXmlArchiver.save_to_memory");
                }
            }

            // 写入文件内容
            int32_t bytes_written = mz_zip_writer_entry_write(writer_handle_.get(), content.data(), static_cast<int32_t>(content.size()));
            if (bytes_written < 0) {
                mz_zip_writer_entry_close(writer_handle_.get());
                throw TinaKitException("Failed to write content for file '" + filename + "'. Status: " + std::to_string(bytes_written), "OpenXmlArchiver.save_to_memory");
            }

            // 关闭文件条目
            if (int32_t close_status = mz_zip_writer_entry_close(writer_handle_.get()); close_status != MZ_OK) {
                throw TinaKitException("Failed to close entry for file '" + filename + "'. Status: " + std::to_string(close_status), "OpenXmlArchiver.save_to_memory");
            }
        }
        pending_new_files_.clear();
        
        if (int32_t status = mz_zip_writer_close(writer_handle_.get()); status != MZ_OK) {
            throw TinaKitException("Failed to close zip writer. Status: " + std::to_string(status), "OpenXmlArchiver.save_to_memory");
        }

        // 现在，从内存流句柄中提取最终的存档。
        const void* buffer_ptr = nullptr;
        mz_stream_mem_get_buffer(memory_stream_handle_.get(), &buffer_ptr);
        int32_t buffer_len = 0;
        mz_stream_mem_get_buffer_length(memory_stream_handle_.get(), &buffer_len);

        if (!buffer_ptr || buffer_len < 0) {
            throw TinaKitException("Failed to retrieve buffer from memory stream after saving.", "OpenXmlArchiver.save_to_memory");
        }

        const auto* byte_ptr = static_cast<const std::byte*>(buffer_ptr);
        std::vector<std::byte> result(byte_ptr, byte_ptr + buffer_len);

        co_return result;

        
    }

    void OpenXmlArchiver::ZipReaderDeleter::operator()(void* handle) const
    {
        if (handle)
        {
            mz_zip_reader_delete(&handle);
        }
    }

    void OpenXmlArchiver::ZipWriterDeleter::operator()(void* handle) const
    {
        if (handle)
        {
            mz_zip_writer_delete(&handle);
        }
    }

    void OpenXmlArchiver::StreamDeleter::operator()(void* handle) const
    {
        if (handle)
        {
            mz_stream_mem_delete(&handle);
        }
    }

    async::Task<void> OpenXmlArchiver::transition_to_writer_mode_if_needed()
    {
        if (writer_handle_)
        {
            co_return;
        }

        writer_handle_.reset(mz_zip_writer_create());
        if (!writer_handle_)
        {
            throw TinaKitException("Failed to create zip writer handle for transition.",
                                   "OpenXmlArchiver.transition_to_writer_mode_if_needed");
        }

        memory_stream_handle_.reset(mz_stream_mem_create());
        if (!memory_stream_handle_)
        {
            throw TinaKitException("Failed to create memory stream for writer for transition.",
                                   "OpenXmlArchiver.transition_to_writer_mode_if_needed");
        }

        // 关键修复：先打开内存流
        if (int32_t status = mz_stream_mem_open(memory_stream_handle_.get(), nullptr, MZ_OPEN_MODE_CREATE);
            status != MZ_OK)
        {
            throw TinaKitException("Failed to open memory stream for transition. Status: " + std::to_string(status),
                                   "OpenXmlArchiver.transition_to_writer_mode_if_needed");
        }

        if (int32_t status = mz_zip_writer_open(writer_handle_.get(), memory_stream_handle_.get(), 0); status != MZ_OK)
        {
            throw TinaKitException(
                "Failed to open zip writer on memory stream for transition. Status: " + std::to_string(status),
                "OpenXmlArchiver.transition_to_writer_mode_if_needed");
        }

        if (reader_handle_)
        {
            if (int32_t status = mz_zip_reader_goto_first_entry(reader_handle_.get()); status == MZ_OK)
            {
                do
                {
                    mz_zip_file* file_info = nullptr;
                    if (mz_zip_reader_entry_get_info(reader_handle_.get(), &file_info) != MZ_OK || !file_info) {
                        continue;
                    }
                    if (files_to_remove_.find(file_info->filename) == files_to_remove_.end())
                    {
                        if (int32_t copy_status = mz_zip_writer_copy_from_reader(
                            writer_handle_.get(), reader_handle_.get()); copy_status != MZ_OK)
                        {
                            throw TinaKitException(
                                "Failed to copy entry '" + std::string(file_info->filename) + "' during transition.",
                                "OpenXmlArchiver.transition_to_writer_mode_if_needed");
                        }
                    }
                }
                while (mz_zip_reader_goto_next_entry(reader_handle_.get()) == MZ_OK);
            }

            reader_handle_.reset(nullptr);
            source_buffer_.clear();
        }

        files_to_remove_.clear();

        co_return;
    }

    void OpenXmlArchiver::close_handles()
    {
        writer_handle_.reset(nullptr);
        reader_handle_.reset(nullptr);
        memory_stream_handle_.reset(nullptr);
    }
}
