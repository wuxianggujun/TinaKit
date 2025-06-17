//
// Created by wuxianggujun on 2025/6/17.
//

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <cstddef>
#include <map>
#include <set>
#include <span>

#include "tinakit/core/async.hpp"

struct mz_zip_reader_s;
struct mz_zip_writer_s;
struct mz_stream_s;


namespace tinakit::core
{
    /**
     * @brief Manages reading from and writing to OpenXML archives, encapsulating minizip-ng.
     *
     * This class provides a high-level C++ interface for manipulating OpenXML format files
     * (XLSX, DOCX, PPTX, etc.) which are essentially ZIP archives containing XML files and resources.
     * It supports both in-memory and filesystem operations with seamless async/await integration.
     *
     * Supported formats:
     * - Excel: .xlsx, .xlsm, .xltx, .xltm
     * - Word: .docx, .docm, .dotx, .dotm
     * - PowerPoint: .pptx, .pptm, .potx, .potm
     * - Any other OpenXML-based format
     *
     * The class handles the transition from read-only to write mode transparently.
     * In read-only mode, it uses a reader handle to access archive contents. When a modification
     * is requested, it seamlessly transitions to write mode by creating a new in-memory archive
     * and copying existing entries (excluding those marked for deletion).
     */
    class OpenXmlArchiver
    {
    public:
        OpenXmlArchiver() = default;
        ~OpenXmlArchiver();

        OpenXmlArchiver(const OpenXmlArchiver&) = delete;
        OpenXmlArchiver& operator=(const OpenXmlArchiver&) = delete;
        OpenXmlArchiver(OpenXmlArchiver&&) = default;
        OpenXmlArchiver& operator=(OpenXmlArchiver&&) = default;

        static async::Task<OpenXmlArchiver> open_from_file(const std::string& path);
        static OpenXmlArchiver open_from_memory(std::vector<std::byte> buffer);
        static OpenXmlArchiver create_in_memory_writer();

        [[nodiscard]] async::Task<std::vector<std::string>> list_files() const;
        [[nodiscard]] async::Task<bool> has_file(const std::string& filename) const;
        
        [[nodiscard]] async::Task<std::vector<std::byte>> read_file(const std::string& filename);
        async::Task<void> add_file(const std::string& filename, std::vector<std::byte> content);
        [[nodiscard]] async::Task<void> remove_file(const std::string& filename);

        async::Task<void> save_to_file(const std::string& path);
        async::Task<std::vector<std::byte>> save_to_memory();
    
    private:

        // Deleters for minizip-ng handles wrapped in unique_ptr
        struct ZipReaderDeleter
        {
            void operator()(void* handle) const;
        };

        struct ZipWriterDeleter
        {
            void operator()(void* handle) const;
        };

        struct StreamDeleter
        {
            void operator()(void* handle) const;
        };

        using unique_zip_reader_ptr = std::unique_ptr<void, ZipReaderDeleter>;
        using unique_zip_writer_ptr = std::unique_ptr<void, ZipWriterDeleter>;
        using unique_stream_ptr = std::unique_ptr<void, StreamDeleter>;

        // Private helper to manage state transitions
        async::Task<void> transition_to_writer_mode_if_needed();
        void close_handles();

        unique_zip_reader_ptr reader_handle_;
        unique_zip_writer_ptr writer_handle_;
        
        // The memory stream handle is kept separately because it's needed after the writer is closed
        unique_stream_ptr memory_stream_handle_;

        std::set<std::string> current_files_;
        std::set<std::string> files_to_remove_;
        std::map<std::string,std::vector<std::byte>> pending_new_files_;

        // Buffer for archives opened from memory,to support read->write transitions
        std::vector<std::byte> source_buffer_;
    };
}
