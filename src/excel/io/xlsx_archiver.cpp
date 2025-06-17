//
// Created by wuxianggujun on 2025/6/17.
//

#include "tinakit/excel/io/xlsx_archiver.hpp"


#include "tinakit/excel/io/io.hpp"
#include "tinakit/core/exceptions.hpp"

// extern "C" {
#include <mz_zip_rw.h>
#include <mz_strm.h>
#include <mz_strm_mem.h>
#include <mz_zip.h>
#include <mz.h>
// }

namespace tinakit::io
{
    XlsxArchiver::~XlsxArchiver()
    {
        close_handles();
    }

    async::Task<XlsxArchiver> XlsxArchiver::open_from_file(const std::string& path)
    {
        XlsxArchiver archiver;
        archiver.reader_handle_.reset(mz_zip_reader_create());
        if (!archiver.reader_handle_)
        {
            throw TinaKitException("Failed to create zip reader handle.", "XlsxArchiver::open_from_file");
        }

        auto buffer = co_await io::read_file_binary(path);
        archiver.source_buffer_ = std::move(buffer);

        void* stream = mz_stream_mem_create();
        mz_stream_mem_set_buffer(stream, archiver.source_buffer_.data(), archiver.source_buffer_.size());

        if (int32_t status = mz_zip_reader_open(archiver.reader_handle_.get(), stream); status != MZ_OK)
        {
            mz_stream_mem_delete(&stream);
            throw TinaKitException("Failed to open zip archive from file stream. Status: " + std::to_string(status),
                                   "XlsxArchiver");
        }

        mz_stream_mem_delete(&stream);

        co_await archiver.list_files();
        co_return archiver;
    }

    XlsxArchiver XlsxArchiver::open_from_memory(std::vector<std::byte> buffer)
    {
        XlsxArchiver archiver;
        archiver.source_buffer_ = std::move(buffer);
        if (archiver.source_buffer_.empty())
        {
            return create_in_memory_writer();
        }

        archiver.reader_handle_.reset(mz_zip_reader_create());
        if (!archiver.reader_handle_)
        {
            throw TinaKitException("Failed to create zip reader handle.", "XlsxArchiver::open_from_memory");
        }

        void* stream = mz_stream_mem_create();

        mz_stream_mem_set_buffer(stream, archiver.source_buffer_.data(), archiver.source_buffer_.size());

        if (int32_t status = mz_zip_reader_open(archiver.reader_handle_.get(), stream); status != MZ_OK)
        {
            mz_stream_mem_delete(&stream);
            throw TinaKitException("Failed to open zip archive from memory stream. Status: " + std::to_string(status),
                                   "XlsxArchiver");
        }

        mz_stream_mem_delete(&stream);
        mz_stream_set_base(stream, nullptr);
        async::sync_wait(archiver.list_files());

        return archiver;
    }

    XlsxArchiver XlsxArchiver::create_in_memory_writer()
    {
        XlsxArchiver archiver;
        archiver.writer_handle_.reset(mz_zip_writer_create());
        if (!archiver.writer_handle_)
        {
            throw TinaKitException("Failed to create zip writer handle.", "XlsxArchiver::create_in_memory_writer");
        }

        archiver.memory_stream_handle_.reset(mz_stream_mem_create());
        if (!archiver.memory_stream_handle_)
        {
            throw TinaKitException("Failed to create memory stream for writer.",
                                   "XlsxArchiver::create_in_memory_writer");
        }

        if (int32_t status = mz_zip_writer_open(archiver.writer_handle_.get(), archiver.memory_stream_handle_.get(), 0);
            status != MZ_OK)
        {
            throw TinaKitException("Failed to open zip writer on memory stream. Status: " + std::to_string(status),
                                   "XlsxArchiver::create_in_memory_writer");
        }

        return archiver;
    }

    async::Task<std::vector<std::string>> XlsxArchiver::list_files() const
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

                if (mz_zip_reader_entry_get_info(reader_handle_.get(), &file_info) == MZ_OK)
                {
                    files.insert(file_info->filename);
                }
            }
            while (mz_zip_reader_goto_next_entry(reader_handle_.get()) == MZ_OK);
        }

        const_cast<XlsxArchiver*>(this)->current_files_ = files;
        co_return std::vector<std::string>(files.begin(), files.end());
    }

    async::Task<bool> XlsxArchiver::has_file(const std::string& filename) const
    {
        if (current_files_.empty())
        {
            co_await list_files();
        }
        co_return current_files_.count(filename) > 0;
    }

    async::Task<std::vector<std::byte>> XlsxArchiver::read_file(const std::string& filename)
    {
        co_await transition_to_writer_mode_if_needed();

        // 如果文件是新添加且尚未保存的，则从待处理映射中读取
        if (auto it = pending_new_files_.find(filename); it != pending_new_files_.end())
        {
            co_return it->second;
        }

        // 无法从写入器句柄读取。如果处于写入模式，则原始读取器已消失。
        if (writer_handle_)
        {
            throw TinaKitException("Cannot read file content after archive has been modified.",
                                   "XlsxArchiver.read_file");
        }

        if (!reader_handle_)
        {
            throw TinaKitException("Archive is not open for reading.", "XlsxArchiver.read_file");
        }

        if (int32_t status = mz_zip_reader_locate_entry(reader_handle_.get(), filename.c_str(), 0); status != MZ_OK)
        {
            throw TinaKitException("File not found in archive: " + filename, "XlsxArchiver.read_file");
        }

        mz_zip_file* file_info = nullptr;
        mz_zip_reader_entry_get_info(reader_handle_.get(), &file_info);

        std::vector<std::byte> buffer(file_info->uncompressed_size);
        if (int32_t status = mz_zip_reader_entry_read(reader_handle_.get(), buffer.data(), buffer.size()); status < 0)
        {
            throw TinaKitException(
                "Failed to read file content for: " + filename + ". Status: " + std::to_string(status),
                "XlsxArchiver.read_file");
        }

        co_return buffer;
    }

    async::Task<std::vector<void>> XlsxArchiver::add_file(const std::string& filename, std::vector<std::byte> content)
    {
        co_await transition_to_writer_mode_if_needed();

        pending_new_files_[filename] = std::move(content);
        current_files_.insert(filename);
        files_to_remove_.erase(filename);
    }

    async::Task<void> XlsxArchiver::remove_file(const std::string& filename)
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
    }

    async::Task<void> XlsxArchiver::save_to_file(const std::string& path)
    {
        auto memory_buffer = co_await save_to_memory();
        
        co_await io::write_file_binary(path, memory_buffer);
    }


    async::Task<std::vector<std::byte>> XlsxArchiver::save_to_memory()
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

        for (auto const& [filename, content] : pending_new_files_) {
            mz_zip_file file_info = {0};
            file_info.filename = filename.c_str();
            file_info.uncompressed_size = content.size();
            file_info.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
            file_info.flag = MZ_ZIP_FLAG_UTF8;

            if (int32_t status = mz_zip_writer_add_buffer(writer_handle_.get(), (void*)content.data(), content.size(), &file_info); status != MZ_OK) {
                throw TinaKitException("Failed to add file '" + filename + "' to archive. Status: " + std::to_string(status), "XlsxArchiver.save_to_memory");
            }
        }
        pending_new_files_.clear();
        
        if (int32_t status = mz_zip_writer_close(writer_handle_.get()); status != MZ_OK) {
            throw TinaKitException("Failed to close zip writer. Status: " + std::to_string(status), "XlsxArchiver.save_to_memory");
        }
        
        // 现在，从内存流句柄中提取最终的存档。
        void* buffer_ptr = nullptr;
        mz_stream_mem_get_buffer(memory_stream_handle_.get(), &buffer_ptr);
        int32_t buffer_len; = mz_stream_mem_get_buffer_length(memory_stream_handle_.get());

        if (!buffer_ptr || buffer_len < 0) {
            throw TinaKitException("Failed to retrieve buffer from memory stream after saving.", "XlsxArchiver.save_to_memory");
        }

        const auto* byte_ptr = static_cast<const std::byte*>(buffer_ptr);
        std::vector<std::byte> result(byte_ptr, byte_ptr + buffer_len);

        co_return result;

        
    }

    void XlsxArchiver::ZipReaderDeleter::operator()(void* handle) const
    {
        if (handle)
        {
            mz_zip_reader_delete(&handle);
        }
    }

    void XlsxArchiver::ZipWriterDeleter::operator()(void* handle) const
    {
        if (handle)
        {
            mz_zip_writer_delete(&handle);
        }
    }

    void XlsxArchiver::StreamDeleter::operator()(void* handle) const
    {
        if (handle)
        {
            mz_stream_mem_delete(&handle);
        }
    }

    async::Task<void> XlsxArchiver::transition_to_writer_mode_if_needed()
    {
        if (writer_handle_)
        {
            co_return;
        }

        writer_handle_.reset(mz_zip_writer_create());
        if (!writer_handle_)
        {
            throw TinaKitException("Failed to create zip writer handle for transition.",
                                   "XlsxArchiver.transition_to_writer_mode_if_needed");
        }

        memory_stream_handle_.reset(mz_stream_mem_create());
        if (!memory_stream_handle_)
        {
            throw TinaKitException("Failed to create memory stream for writer for transition.",
                                   "XlsxArchiver.transition_to_writer_mode_if_needed");
        }

        if (int32_t status = mz_zip_writer_open(writer_handle_.get(), memory_stream_handle_.get(), 0); status != MZ_OK)
        {
            throw TinaKitException(
                "Failed to open zip writer on memory stream for transition. Status: " + std::to_string(status),
                "XlsxArchiver.transition_to_writer_mode_if_needed");
        }

        if (reader_handle_)
        {
            if (int32_t status = mz_zip_reader_goto_first_entry(reader_handle_.get()); status == MZ_OK)
            {
                do
                {
                    mz_zip_file* file_info = nullptr;
                    mz_zip_reader_entry_get_info(reader_handle_.get(), &file_info);
                    if (files_to_remove_.find(file_info->filename) == files_to_remove_.end())
                    {
                        if (int32_t copy_status = mz_zip_writer_copy_from_reader(
                            writer_handle_.get(), reader_handle_.get()); copy_status != MZ_OK)
                        {
                            throw TinaKitException(
                                "Failed to copy entry '" + std::string(file_info->filename) + "' during transition.",
                                "XlsxArchiver.transition_to_writer_mode_if_needed");
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

    void XlsxArchiver::close_handles()
    {
        writer_handle_.reset(nullptr);
        reader_handle_.reset(nullptr);
        memory_stream_handle_.reset(nullptr);
    }
}
