/**
 * @file compression.cpp
 * @brief PDF压缩工具实现 - 基于zlib-ng
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/pdf/utils/compression.hpp"
#include "tinakit/core/logger.hpp"
#include <algorithm>
#include <stdexcept>

namespace tinakit::pdf::utils {

// ========================================
// FlateDecode压缩实现
// ========================================

CompressionResult Compression::compressFlateDecode(const std::vector<std::uint8_t>& data, 
                                                   int compression_level) {
    CompressionResult result;
    result.type = CompressionType::FlateDecode;
    result.original_size = data.size();
    result.success = false;
    
    if (data.empty()) {
        result.success = true;
        result.compressed_size = 0;
        result.compression_ratio = 0.0;
        return result;
    }
    
    // 使用zlib-ng进行压缩
    std::vector<std::uint8_t> compressed;
    if (compressInternal(data, compressed, compression_level)) {
        result.data = std::move(compressed);
        result.compressed_size = result.data.size();
        result.compression_ratio = static_cast<double>(result.compressed_size) / result.original_size;
        result.success = true;
        
    } else {
        result.error_message = "zlib-ng压缩失败";
    }
    
    return result;
}

CompressionResult Compression::compressFlateDecode(const std::string& data, 
                                                   int compression_level) {
    std::vector<std::uint8_t> bytes(data.begin(), data.end());
    return compressFlateDecode(bytes, compression_level);
}

std::vector<std::uint8_t> Compression::decompressFlateDecode(const std::vector<std::uint8_t>& compressed_data) {
    std::vector<std::uint8_t> result;
    
    if (compressed_data.empty()) {
        return result;
    }
    
    if (decompressInternal(compressed_data, result)) {

    } else {
        result.clear();
    }
    
    return result;
}

std::string Compression::decompressFlateDecodeToString(const std::vector<std::uint8_t>& compressed_data) {
    auto decompressed = decompressFlateDecode(compressed_data);
    return std::string(decompressed.begin(), decompressed.end());
}

// ========================================
// 通用压缩接口
// ========================================

CompressionResult Compression::compress(const std::vector<std::uint8_t>& data, 
                                       const CompressionParams& params) {
    switch (params.type) {
        case CompressionType::FlateDecode:
            return compressFlateDecode(data, params.compression_level);
        case CompressionType::None:
            {
                CompressionResult result;
                result.data = data;
                result.type = CompressionType::None;
                result.original_size = data.size();
                result.compressed_size = data.size();
                result.compression_ratio = 1.0;
                result.success = true;
                return result;
            }
        default:
            {
                CompressionResult result;
                result.success = false;
                result.error_message = "不支持的压缩类型";
                return result;
            }
    }
}

CompressionResult Compression::compress(const std::string& data, 
                                       const CompressionParams& params) {
    std::vector<std::uint8_t> bytes(data.begin(), data.end());
    return compress(bytes, params);
}

std::vector<std::uint8_t> Compression::decompress(const std::vector<std::uint8_t>& compressed_data,
                                                 CompressionType type) {
    switch (type) {
        case CompressionType::FlateDecode:
            return decompressFlateDecode(compressed_data);
        case CompressionType::None:
            return compressed_data;
        default:
            return {};
    }
}

// ========================================
// 压缩策略和优化
// ========================================

double Compression::estimateCompressionRatio(const std::vector<std::uint8_t>& data, 
                                            CompressionType type) {
    if (data.empty()) return 0.0;
    
    // 简单的启发式估算
    switch (type) {
        case CompressionType::FlateDecode:
            {
                // 基于数据的重复性估算压缩比
                std::vector<std::uint8_t> sample;
                size_t sample_size = std::min(data.size(), size_t(1024));
                sample.assign(data.begin(), data.begin() + sample_size);
                
                auto result = compressFlateDecode(sample, Z_BEST_SPEED);
                return result.success ? result.compression_ratio : 1.0;
            }
        case CompressionType::None:
            return 1.0;
        default:
            return 1.0;
    }
}

bool Compression::shouldCompress(std::size_t data_size, 
                                std::size_t threshold, 
                                double min_ratio) {
    return data_size >= threshold;
}

CompressionResult Compression::adaptiveCompress(const std::vector<std::uint8_t>& data,
                                               const CompressionParams& params) {
    // 如果数据太小，不压缩
    if (!shouldCompress(data.size())) {
        CompressionResult result;
        result.data = data;
        result.type = CompressionType::None;
        result.original_size = data.size();
        result.compressed_size = data.size();
        result.compression_ratio = 1.0;
        result.success = true;
        return result;
    }
    
    // 尝试压缩
    auto result = compress(data, params);
    
    // 如果压缩效果不好，返回原始数据
    if (result.success && result.compression_ratio > 0.9) {
        result.data = data;
        result.type = CompressionType::None;
        result.compressed_size = data.size();
        result.compression_ratio = 1.0;
    }
    
    return result;
}

// ========================================
// 工具方法
// ========================================

std::string Compression::getFilterName(CompressionType type) {
    switch (type) {
        case CompressionType::FlateDecode:
            return "FlateDecode";
        case CompressionType::LZWDecode:
            return "LZWDecode";
        case CompressionType::RunLengthDecode:
            return "RunLengthDecode";
        case CompressionType::None:
        default:
            return "";
    }
}

CompressionType Compression::getCompressionType(const std::string& filter_name) {
    if (filter_name == "FlateDecode") return CompressionType::FlateDecode;
    if (filter_name == "LZWDecode") return CompressionType::LZWDecode;
    if (filter_name == "RunLengthDecode") return CompressionType::RunLengthDecode;
    return CompressionType::None;
}

bool Compression::isCompressionAvailable(CompressionType type) {
    switch (type) {
        case CompressionType::FlateDecode:
        case CompressionType::None:
            return true;
        default:
            return false;  // 其他压缩类型暂未实现
    }
}

std::vector<CompressionType> Compression::getAvailableCompressions() {
    return {CompressionType::None, CompressionType::FlateDecode};
}

// ========================================
// 内部实现
// ========================================

bool Compression::compressInternal(const std::vector<std::uint8_t>& input,
                                  std::vector<std::uint8_t>& output,
                                  int level, int strategy) {
    z_stream strm = {};
    
    // 初始化压缩流
    if (deflateInit2(&strm, level, Z_DEFLATED, 15, 8, strategy) != Z_OK) {
        return false;
    }
    
    // 预分配输出缓冲区
    output.resize(deflateBound(&strm, input.size()));
    
    strm.next_in = const_cast<Bytef*>(input.data());
    strm.avail_in = static_cast<uInt>(input.size());
    strm.next_out = output.data();
    strm.avail_out = static_cast<uInt>(output.size());
    
    // 执行压缩
    int ret = deflate(&strm, Z_FINISH);
    bool success = (ret == Z_STREAM_END);
    
    if (success) {
        output.resize(strm.total_out);
    }
    
    deflateEnd(&strm);
    return success;
}

bool Compression::decompressInternal(const std::vector<std::uint8_t>& input,
                                    std::vector<std::uint8_t>& output) {
    z_stream strm = {};
    
    // 初始化解压缩流
    if (inflateInit(&strm) != Z_OK) {
        return false;
    }
    
    strm.next_in = const_cast<Bytef*>(input.data());
    strm.avail_in = static_cast<uInt>(input.size());
    
    // 动态扩展输出缓冲区
    const size_t chunk_size = 16384;
    output.clear();
    
    int ret;
    do {
        size_t old_size = output.size();
        output.resize(old_size + chunk_size);
        
        strm.next_out = output.data() + old_size;
        strm.avail_out = chunk_size;
        
        ret = inflate(&strm, Z_NO_FLUSH);
        
        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
            inflateEnd(&strm);
            return false;
        }
        
        output.resize(old_size + chunk_size - strm.avail_out);
        
    } while (ret != Z_STREAM_END && strm.avail_out == 0);
    
    inflateEnd(&strm);
    return (ret == Z_STREAM_END);
}

} // namespace tinakit::pdf::utils
