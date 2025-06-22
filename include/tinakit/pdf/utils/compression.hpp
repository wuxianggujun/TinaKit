/**
 * @file compression.hpp
 * @brief PDF压缩工具 - 基于zlib-ng的FlateDecode压缩支持
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <memory>

// 使用项目中已有的zlib-ng库进行FlateDecode压缩
// zlib-ng提供zlib兼容的API
#include "zlib.h"

namespace tinakit::pdf::utils {

/**
 * @brief 压缩算法类型
 */
enum class CompressionType {
    None,           ///< 无压缩
    FlateDecode,    ///< zlib/deflate压缩（PDF标准）
    LZWDecode,      ///< LZW压缩
    RunLengthDecode ///< 行程长度编码
};

/**
 * @brief 压缩参数
 */
struct CompressionParams {
    CompressionType type = CompressionType::FlateDecode;
    int compression_level = 6;      ///< 压缩级别 (0-9, 0=无压缩, 9=最大压缩)
    bool use_predictor = false;     ///< 是否使用预测器
    int predictor = 1;              ///< 预测器类型
    int columns = 1;                ///< 列数（用于预测器）
    int colors = 1;                 ///< 颜色分量数
    int bits_per_component = 8;     ///< 每分量位数
};

/**
 * @brief 压缩结果
 */
struct CompressionResult {
    std::vector<std::uint8_t> data;     ///< 压缩后的数据
    CompressionType type;               ///< 使用的压缩类型
    std::size_t original_size;          ///< 原始大小
    std::size_t compressed_size;        ///< 压缩后大小
    double compression_ratio;           ///< 压缩比 (compressed_size / original_size)
    bool success;                       ///< 是否成功
    std::string error_message;          ///< 错误信息
};

/**
 * @class Compression
 * @brief PDF压缩工具类
 *
 * 基于项目中的zlib-ng库提供PDF标准的FlateDecode压缩
 * zlib-ng是zlib的高性能替代品，提供更好的压缩速度和效率
 */
class Compression {
public:
    // ========================================
    // FlateDecode压缩（基于zlib-ng）
    // ========================================

    /**
     * @brief FlateDecode压缩（使用zlib-ng）
     * @param data 原始数据
     * @param compression_level 压缩级别 (0-9, Z_DEFAULT_COMPRESSION=-1)
     * @return 压缩结果
     */
    static CompressionResult compressFlateDecode(const std::vector<std::uint8_t>& data,
                                                int compression_level = Z_DEFAULT_COMPRESSION);
    
    /**
     * @brief FlateDecode压缩（字符串版本，使用zlib-ng）
     * @param data 原始字符串
     * @param compression_level 压缩级别 (0-9, Z_DEFAULT_COMPRESSION=-1)
     * @return 压缩结果
     */
    static CompressionResult compressFlateDecode(const std::string& data,
                                                int compression_level = Z_DEFAULT_COMPRESSION);

    /**
     * @brief FlateDecode解压缩（使用zlib-ng）
     * @param compressed_data 压缩数据
     * @return 解压缩的数据
     */
    static std::vector<std::uint8_t> decompressFlateDecode(const std::vector<std::uint8_t>& compressed_data);

    /**
     * @brief FlateDecode解压缩（返回字符串，使用zlib-ng）
     * @param compressed_data 压缩数据
     * @return 解压缩的字符串
     */
    static std::string decompressFlateDecodeToString(const std::vector<std::uint8_t>& compressed_data);

    // ========================================
    // 通用压缩接口
    // ========================================
    
    /**
     * @brief 通用压缩方法
     * @param data 原始数据
     * @param params 压缩参数
     * @return 压缩结果
     */
    static CompressionResult compress(const std::vector<std::uint8_t>& data, 
                                    const CompressionParams& params = {});
    
    /**
     * @brief 通用压缩方法（字符串版本）
     * @param data 原始字符串
     * @param params 压缩参数
     * @return 压缩结果
     */
    static CompressionResult compress(const std::string& data, 
                                    const CompressionParams& params = {});
    
    /**
     * @brief 通用解压缩方法
     * @param compressed_data 压缩数据
     * @param type 压缩类型
     * @return 解压缩的数据
     */
    static std::vector<std::uint8_t> decompress(const std::vector<std::uint8_t>& compressed_data,
                                               CompressionType type);

    // ========================================
    // 压缩策略和优化
    // ========================================
    
    /**
     * @brief 估算压缩比
     * @param data 数据
     * @param type 压缩类型
     * @return 估算的压缩比
     */
    static double estimateCompressionRatio(const std::vector<std::uint8_t>& data, 
                                         CompressionType type = CompressionType::FlateDecode);
    
    /**
     * @brief 判断是否应该压缩
     * @param data_size 数据大小
     * @param threshold 压缩阈值（小于此大小不压缩）
     * @param min_ratio 最小压缩比（压缩比低于此值不压缩）
     * @return 如果应该压缩返回true
     */
    static bool shouldCompress(std::size_t data_size, 
                             std::size_t threshold = 100, 
                             double min_ratio = 0.9);
    
    /**
     * @brief 自适应压缩（自动选择是否压缩）
     * @param data 原始数据
     * @param params 压缩参数
     * @return 压缩结果（可能未压缩）
     */
    static CompressionResult adaptiveCompress(const std::vector<std::uint8_t>& data,
                                            const CompressionParams& params = {});
    
    /**
     * @brief 选择最佳压缩算法
     * @param data 数据
     * @param available_types 可用的压缩类型
     * @return 最佳压缩类型
     */
    static CompressionType selectBestCompression(const std::vector<std::uint8_t>& data,
                                               const std::vector<CompressionType>& available_types);

    // ========================================
    // 流式压缩（用于大文件）
    // ========================================
    
    /**
     * @brief 流式压缩器（基于zlib-ng流式API）
     */
    class StreamCompressor {
    public:
        explicit StreamCompressor(CompressionType type = CompressionType::FlateDecode,
                                int compression_level = Z_DEFAULT_COMPRESSION);
        ~StreamCompressor();
        
        /**
         * @brief 添加数据到压缩流
         * @param data 数据
         * @return 是否成功
         */
        bool addData(const std::vector<std::uint8_t>& data);
        bool addData(const std::string& data);
        
        /**
         * @brief 完成压缩并获取结果
         * @return 压缩结果
         */
        CompressionResult finish();
        
        /**
         * @brief 重置压缩器
         */
        void reset();
        
    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };

    // ========================================
    // 预测器支持（用于图像数据）
    // ========================================
    
    /**
     * @brief 应用PNG预测器
     * @param data 原始数据
     * @param width 图像宽度
     * @param height 图像高度
     * @param components 颜色分量数
     * @param bits_per_component 每分量位数
     * @return 预测器处理后的数据
     */
    static std::vector<std::uint8_t> applyPNGPredictor(const std::vector<std::uint8_t>& data,
                                                      int width, int height,
                                                      int components, int bits_per_component);
    
    /**
     * @brief 移除PNG预测器
     * @param data 预测器处理的数据
     * @param width 图像宽度
     * @param height 图像高度
     * @param components 颜色分量数
     * @param bits_per_component 每分量位数
     * @return 原始数据
     */
    static std::vector<std::uint8_t> removePNGPredictor(const std::vector<std::uint8_t>& data,
                                                       int width, int height,
                                                       int components, int bits_per_component);

    // ========================================
    // 工具方法
    // ========================================
    
    /**
     * @brief 获取压缩类型的PDF过滤器名称
     * @param type 压缩类型
     * @return PDF过滤器名称
     */
    static std::string getFilterName(CompressionType type);
    
    /**
     * @brief 从PDF过滤器名称获取压缩类型
     * @param filter_name PDF过滤器名称
     * @return 压缩类型
     */
    static CompressionType getCompressionType(const std::string& filter_name);
    
    /**
     * @brief 检查压缩类型是否可用
     * @param type 压缩类型
     * @return 如果可用返回true
     */
    static bool isCompressionAvailable(CompressionType type);
    
    /**
     * @brief 获取所有可用的压缩类型
     * @return 可用压缩类型列表
     */
    static std::vector<CompressionType> getAvailableCompressions();

private:
    // ========================================
    // 内部实现（基于zlib-ng）
    // ========================================

    /**
     * @brief 内部压缩实现
     * @param input 输入数据
     * @param output 输出缓冲区
     * @param level 压缩级别
     * @param strategy 压缩策略
     * @return 压缩是否成功
     */
    static bool compressInternal(const std::vector<std::uint8_t>& input,
                               std::vector<std::uint8_t>& output,
                               int level, int strategy = Z_DEFAULT_STRATEGY);

    /**
     * @brief 内部解压缩实现
     * @param input 输入数据
     * @param output 输出缓冲区
     * @return 解压缩是否成功
     */
    static bool decompressInternal(const std::vector<std::uint8_t>& input,
                                 std::vector<std::uint8_t>& output);
};

// ========================================
// 便利函数
// ========================================

/**
 * @brief 快速FlateDecode压缩
 * @param data 数据
 * @return 压缩后的数据
 */
inline std::vector<std::uint8_t> quickCompress(const std::string& data) {
    auto result = Compression::compressFlateDecode(data);
    return result.success ? result.data : std::vector<std::uint8_t>();
}

/**
 * @brief 快速FlateDecode解压缩
 * @param compressed_data 压缩数据
 * @return 解压缩的字符串
 */
inline std::string quickDecompress(const std::vector<std::uint8_t>& compressed_data) {
    return Compression::decompressFlateDecodeToString(compressed_data);
}

} // namespace tinakit::pdf::utils
