/**
 * @file debug_tools.hpp
 * @brief PDF调试和可视化工具
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

namespace tinakit::pdf::core {
    class Writer;
    class PdfObject;
}

namespace tinakit::pdf::utils {

/**
 * @brief PDF对象统计信息
 */
struct ObjectStats {
    std::string type;               ///< 对象类型
    std::size_t count = 0;          ///< 对象数量
    std::size_t total_size = 0;     ///< 总大小（字节）
    std::size_t avg_size = 0;       ///< 平均大小
    std::size_t min_size = 0;       ///< 最小大小
    std::size_t max_size = 0;       ///< 最大大小
};

/**
 * @brief PDF性能统计信息
 */
struct PerformanceStats {
    std::size_t total_objects = 0;          ///< 总对象数
    std::size_t total_pages = 0;            ///< 总页面数
    std::size_t total_size = 0;             ///< 总文件大小
    std::size_t uncompressed_size = 0;      ///< 未压缩大小
    double compression_ratio = 0.0;         ///< 压缩比
    
    std::map<std::string, ObjectStats> object_stats;  ///< 按类型的对象统计
    
    // 内存使用情况
    std::size_t peak_memory_usage = 0;      ///< 峰值内存使用
    std::size_t current_memory_usage = 0;   ///< 当前内存使用
    
    // 时间统计
    double generation_time = 0.0;           ///< 生成时间（秒）
    double compression_time = 0.0;          ///< 压缩时间（秒）
    double writing_time = 0.0;              ///< 写入时间（秒）
};

/**
 * @brief PDF验证结果
 */
struct ValidationResult {
    bool is_valid = true;                   ///< 是否有效
    std::vector<std::string> errors;        ///< 错误列表
    std::vector<std::string> warnings;      ///< 警告列表
    std::vector<std::string> suggestions;   ///< 建议列表
    
    // 结构检查
    bool has_catalog = false;               ///< 是否有目录对象
    bool has_pages = false;                 ///< 是否有页面树
    bool xref_valid = false;                ///< 交叉引用表是否有效
    bool trailer_valid = false;             ///< 尾部是否有效
    
    // 内容检查
    std::size_t orphaned_objects = 0;       ///< 孤立对象数量
    std::size_t circular_references = 0;    ///< 循环引用数量
    std::size_t missing_references = 0;     ///< 缺失引用数量
};

/**
 * @class DebugTools
 * @brief PDF调试和可视化工具类
 * 
 * 提供PDF文档结构分析、性能统计、验证和可视化功能
 */
class DebugTools {
public:
    // ========================================
    // 结构分析和可视化
    // ========================================
    
    /**
     * @brief 生成PDF结构的DOT图（Graphviz格式）
     * @param writer PDF写入器
     * @param include_content 是否包含内容详情
     * @return DOT格式的图描述
     */
    static std::string generateStructureDot(const core::Writer& writer, bool include_content = false);
    
    /**
     * @brief 生成对象关系图
     * @param writer PDF写入器
     * @return DOT格式的对象关系图
     */
    static std::string generateObjectGraph(const core::Writer& writer);
    
    /**
     * @brief 生成页面结构图
     * @param writer PDF写入器
     * @return DOT格式的页面结构图
     */
    static std::string generatePageStructure(const core::Writer& writer);
    
    /**
     * @brief 生成资源依赖图
     * @param writer PDF写入器
     * @return DOT格式的资源依赖图
     */
    static std::string generateResourceDependencies(const core::Writer& writer);

    // ========================================
    // 性能分析
    // ========================================
    
    /**
     * @brief 分析PDF性能统计
     * @param writer PDF写入器
     * @return 性能统计信息
     */
    static PerformanceStats analyzePerformance(const core::Writer& writer);
    
    /**
     * @brief 分析对象大小分布
     * @param writer PDF写入器
     * @return 对象大小统计
     */
    static std::map<std::string, ObjectStats> analyzeObjectSizes(const core::Writer& writer);
    
    /**
     * @brief 分析压缩效率
     * @param writer PDF写入器
     * @return 压缩效率报告
     */
    static std::string analyzeCompressionEfficiency(const core::Writer& writer);
    
    /**
     * @brief 生成性能报告
     * @param stats 性能统计信息
     * @param format 报告格式（"text", "html", "json"）
     * @return 格式化的性能报告
     */
    static std::string generatePerformanceReport(const PerformanceStats& stats, 
                                                const std::string& format = "text");

    // ========================================
    // 验证和检查
    // ========================================
    
    /**
     * @brief 验证PDF结构
     * @param writer PDF写入器
     * @return 验证结果
     */
    static ValidationResult validateStructure(const core::Writer& writer);
    
    /**
     * @brief 检查对象引用完整性
     * @param writer PDF写入器
     * @return 引用检查结果
     */
    static std::vector<std::string> checkReferenceIntegrity(const core::Writer& writer);
    
    /**
     * @brief 检查PDF标准兼容性
     * @param writer PDF写入器
     * @param pdf_version 目标PDF版本
     * @return 兼容性检查结果
     */
    static std::vector<std::string> checkStandardCompliance(const core::Writer& writer,
                                                           const std::string& pdf_version = "1.4");
    
    /**
     * @brief 查找优化建议
     * @param writer PDF写入器
     * @return 优化建议列表
     */
    static std::vector<std::string> findOptimizationSuggestions(const core::Writer& writer);

    // ========================================
    // 调试信息导出
    // ========================================
    
    /**
     * @brief 导出完整的调试信息
     * @param writer PDF写入器
     * @param output_dir 输出目录
     * @param include_visualizations 是否包含可视化文件
     */
    static void exportDebugInfo(const core::Writer& writer, 
                               const std::string& output_dir,
                               bool include_visualizations = true);
    
    /**
     * @brief 导出对象详情
     * @param writer PDF写入器
     * @param output_file 输出文件
     * @param format 输出格式（"text", "json", "xml"）
     */
    static void exportObjectDetails(const core::Writer& writer,
                                   const std::string& output_file,
                                   const std::string& format = "text");
    
    /**
     * @brief 导出内容流分析
     * @param writer PDF写入器
     * @param output_file 输出文件
     */
    static void exportContentStreamAnalysis(const core::Writer& writer,
                                           const std::string& output_file);

    // ========================================
    // 交互式调试
    // ========================================
    
    /**
     * @brief 启动交互式调试会话
     * @param writer PDF写入器
     */
    static void startInteractiveDebugSession(const core::Writer& writer);
    
    /**
     * @brief 生成调试Web界面
     * @param writer PDF写入器
     * @param output_dir 输出目录
     * @return 生成的HTML文件路径
     */
    static std::string generateDebugWebInterface(const core::Writer& writer,
                                                const std::string& output_dir);

    // ========================================
    // 比较和差异分析
    // ========================================
    
    /**
     * @brief 比较两个PDF文档结构
     * @param writer1 第一个PDF写入器
     * @param writer2 第二个PDF写入器
     * @return 差异报告
     */
    static std::string comparePdfStructures(const core::Writer& writer1,
                                           const core::Writer& writer2);
    
    /**
     * @brief 分析PDF版本间的差异
     * @param old_version 旧版本PDF数据
     * @param new_version 新版本PDF数据
     * @return 版本差异报告
     */
    static std::string analyzeVersionDifferences(const std::vector<std::uint8_t>& old_version,
                                                const std::vector<std::uint8_t>& new_version);

    // ========================================
    // 工具方法
    // ========================================
    
    /**
     * @brief 格式化字节大小
     * @param bytes 字节数
     * @return 格式化的大小字符串（如"1.5 MB"）
     */
    static std::string formatBytes(std::size_t bytes);
    
    /**
     * @brief 格式化百分比
     * @param ratio 比率（0.0-1.0）
     * @param precision 小数位数
     * @return 格式化的百分比字符串
     */
    static std::string formatPercentage(double ratio, int precision = 1);
    
    /**
     * @brief 生成时间戳
     * @return 当前时间戳字符串
     */
    static std::string generateTimestamp();
    
    /**
     * @brief 创建输出目录
     * @param path 目录路径
     * @return 是否成功创建
     */
    static bool createOutputDirectory(const std::string& path);

private:
    // ========================================
    // 内部辅助方法
    // ========================================
    
    /**
     * @brief 分析单个对象
     * @param object PDF对象
     * @return 对象分析结果
     */
    static std::string analyzeObject(const core::PdfObject& object);
    
    /**
     * @brief 生成DOT节点
     * @param id 节点ID
     * @param label 节点标签
     * @param shape 节点形状
     * @param color 节点颜色
     * @return DOT节点定义
     */
    static std::string generateDotNode(const std::string& id,
                                      const std::string& label,
                                      const std::string& shape = "box",
                                      const std::string& color = "lightblue");
    
    /**
     * @brief 生成DOT边
     * @param from 起始节点
     * @param to 目标节点
     * @param label 边标签
     * @param style 边样式
     * @return DOT边定义
     */
    static std::string generateDotEdge(const std::string& from,
                                      const std::string& to,
                                      const std::string& label = "",
                                      const std::string& style = "solid");
    
    /**
     * @brief 转义DOT标签
     * @param label 原始标签
     * @return 转义后的标签
     */
    static std::string escapeDotLabel(const std::string& label);
};

} // namespace tinakit::pdf::utils
