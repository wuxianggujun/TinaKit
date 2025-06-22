/**
 * @file logger.hpp
 * @brief TinaKit全局日志系统
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <io.h>
#include <fcntl.h>
// 取消Windows定义的宏，避免与我们的枚举冲突
#ifdef ERROR
#undef ERROR
#endif
#endif

namespace tinakit {
namespace core {

/**
 * @enum LogLevel
 * @brief 日志级别
 */
enum class LogLevel {
    TRACE = 0,    ///< 跟踪信息
    DEBUG = 1,    ///< 调试信息
    INFO = 2,     ///< 一般信息
    WARN = 3,     ///< 警告信息
    ERROR = 4,    ///< 错误信息
    FATAL = 5     ///< 致命错误
};

/**
 * @struct LogEntry
 * @brief 日志条目
 */
struct LogEntry {
    LogLevel level;                                    ///< 日志级别
    std::string message;                              ///< 日志消息
    std::string module;                               ///< 模块名称
    std::string function;                             ///< 函数名称
    std::string file;                                 ///< 文件名
    int line;                                         ///< 行号
    std::thread::id thread_id;                        ///< 线程ID
    std::chrono::system_clock::time_point timestamp;  ///< 时间戳
    
    LogEntry(LogLevel lvl, const std::string& msg, const std::string& mod,
             const std::string& func, const std::string& f, int l)
        : level(lvl), message(msg), module(mod), function(func), file(f), line(l),
          thread_id(std::this_thread::get_id()), timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @class LogHandler
 * @brief 日志处理器基类
 */
class LogHandler {
public:
    virtual ~LogHandler() = default;
    
    /**
     * @brief 处理日志条目
     * @param entry 日志条目
     */
    virtual void handle(const LogEntry& entry) = 0;
    
    /**
     * @brief 设置日志级别过滤
     * @param level 最低日志级别
     */
    virtual void setLevel(LogLevel level) { min_level_ = level; }
    
    /**
     * @brief 检查是否应该处理此级别的日志
     * @param level 日志级别
     * @return 如果应该处理返回true
     */
    virtual bool shouldHandle(LogLevel level) const { return level >= min_level_; }

protected:
    LogLevel min_level_ = LogLevel::INFO;
};

/**
 * @class ConsoleHandler
 * @brief 控制台日志处理器
 */
class ConsoleHandler : public LogHandler {
public:
    /**
     * @brief 构造函数
     * @param colored 是否使用彩色输出
     * @param enable_unicode 是否启用Unicode支持（Windows下自动设置控制台编码）
     */
    explicit ConsoleHandler(bool colored = true, bool enable_unicode = true);

    /**
     * @brief 析构函数，恢复控制台设置
     */
    ~ConsoleHandler();

    void handle(const LogEntry& entry) override;

    /**
     * @brief 设置是否使用彩色输出
     * @param colored 是否彩色
     */
    void setColored(bool colored) { colored_ = colored; }

    /**
     * @brief 设置Unicode支持
     * @param enable 是否启用
     */
    void setUnicodeSupport(bool enable) { unicode_enabled_ = enable; }

private:
    bool colored_;
    bool unicode_enabled_;
    std::mutex mutex_;

#ifdef _WIN32
    UINT original_console_cp_;
    UINT original_console_output_cp_;
    bool console_setup_;
#endif

    void setupConsoleEncoding();
    void restoreConsoleEncoding();
    std::string getColorCode(LogLevel level) const;
    std::string formatEntry(const LogEntry& entry) const;
};

/**
 * @class FileHandler
 * @brief 文件日志处理器
 */
class FileHandler : public LogHandler {
public:
    /**
     * @brief 构造函数
     * @param filename 日志文件名
     * @param append 是否追加模式
     */
    explicit FileHandler(const std::string& filename, bool append = true);
    
    /**
     * @brief 析构函数
     */
    ~FileHandler();
    
    void handle(const LogEntry& entry) override;
    
    /**
     * @brief 设置文件轮转
     * @param max_size 最大文件大小（字节）
     * @param max_files 最大文件数量
     */
    void setRotation(size_t max_size, int max_files);
    
    /**
     * @brief 刷新缓冲区
     */
    void flush();

private:
    std::string filename_;
    std::ofstream file_;
    std::mutex mutex_;
    size_t max_file_size_ = 0;
    int max_files_ = 0;
    size_t current_size_ = 0;
    
    void rotateFile();
    std::string formatEntry(const LogEntry& entry) const;
};

/**
 * @class Logger
 * @brief 主日志器类
 */
class Logger {
public:
    /**
     * @brief 获取全局日志器实例
     * @return 日志器实例
     */
    static Logger& getInstance();
    
    /**
     * @brief 添加日志处理器
     * @param handler 处理器
     */
    void addHandler(std::unique_ptr<LogHandler> handler);
    
    /**
     * @brief 移除所有处理器
     */
    void clearHandlers();
    
    /**
     * @brief 设置全局日志级别
     * @param level 日志级别
     */
    void setLevel(LogLevel level);
    
    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param message 消息
     * @param module 模块名
     * @param function 函数名
     * @param file 文件名
     * @param line 行号
     */
    void log(LogLevel level, const std::string& message, const std::string& module,
             const std::string& function, const std::string& file, int line);
    
    /**
     * @brief 启动异步日志处理
     */
    void start();
    
    /**
     * @brief 停止异步日志处理
     */
    void stop();
    
    /**
     * @brief 刷新所有处理器
     */
    void flush();
    
    /**
     * @brief 检查是否启用了指定级别的日志
     * @param level 日志级别
     * @return 如果启用返回true
     */
    bool isEnabled(LogLevel level) const;

private:
    Logger() = default;
    ~Logger();
    
    // 禁用拷贝和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::vector<std::unique_ptr<LogHandler>> handlers_;
    std::queue<LogEntry> log_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::thread worker_thread_;
    bool running_ = false;
    LogLevel global_level_ = LogLevel::INFO;
    
    void workerLoop();
};

// ========================================
// 便利宏定义
// ========================================

#define TINAKIT_LOG(level, module, message) \
    do { \
        if (::tinakit::core::Logger::getInstance().isEnabled(level)) { \
            ::tinakit::core::Logger::getInstance().log(level, message, module, __FUNCTION__, __FILE__, __LINE__); \
        } \
    } while(0)

// 通用日志宏
#define TINAKIT_TRACE(module, message) TINAKIT_LOG(::tinakit::core::LogLevel::TRACE, module, message)
#define TINAKIT_DEBUG(module, message) TINAKIT_LOG(::tinakit::core::LogLevel::DEBUG, module, message)
#define TINAKIT_INFO(module, message)  TINAKIT_LOG(::tinakit::core::LogLevel::INFO, module, message)
#define TINAKIT_WARN(module, message)  TINAKIT_LOG(::tinakit::core::LogLevel::WARN, module, message)
#define TINAKIT_ERROR(module, message) TINAKIT_LOG(::tinakit::core::LogLevel::ERROR, module, message)
#define TINAKIT_FATAL(module, message) TINAKIT_LOG(::tinakit::core::LogLevel::FATAL, module, message)

// 模块特定的日志宏
#define EXCEL_TRACE(message) TINAKIT_TRACE("Excel", message)
#define EXCEL_DEBUG(message) TINAKIT_DEBUG("Excel", message)
#define EXCEL_INFO(message)  TINAKIT_INFO("Excel", message)
#define EXCEL_WARN(message)  TINAKIT_WARN("Excel", message)
#define EXCEL_ERROR(message) TINAKIT_ERROR("Excel", message)

#define PDF_TRACE(message) TINAKIT_TRACE("PDF", message)
#define PDF_DEBUG(message) TINAKIT_DEBUG("PDF", message)
#define PDF_INFO(message)  TINAKIT_INFO("PDF", message)
#define PDF_WARN(message)  TINAKIT_WARN("PDF", message)
#define PDF_ERROR(message) TINAKIT_ERROR("PDF", message)

#define CORE_TRACE(message) TINAKIT_TRACE("Core", message)
#define CORE_DEBUG(message) TINAKIT_DEBUG("Core", message)
#define CORE_INFO(message)  TINAKIT_INFO("Core", message)
#define CORE_WARN(message)  TINAKIT_WARN("Core", message)
#define CORE_ERROR(message) TINAKIT_ERROR("Core", message)

// ========================================
// 辅助函数
// ========================================

/**
 * @brief 日志级别转字符串
 * @param level 日志级别
 * @return 字符串表示
 */
std::string logLevelToString(LogLevel level);

/**
 * @brief 字符串转日志级别
 * @param str 字符串
 * @return 日志级别
 */
LogLevel stringToLogLevel(const std::string& str);

/**
 * @brief 格式化时间戳
 * @param timestamp 时间戳
 * @return 格式化的时间字符串
 */
std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp);

/**
 * @brief 初始化默认日志配置
 * @param console_level 控制台日志级别
 * @param file_path 日志文件路径（空字符串表示不写文件）
 * @param file_level 文件日志级别
 */
void initializeDefaultLogging(LogLevel console_level = LogLevel::INFO,
                             const std::string& file_path = "",
                             LogLevel file_level = LogLevel::DEBUG);

/**
 * @brief 设置控制台UTF-8支持（Windows专用）
 * @return 是否设置成功
 */
bool setupConsoleUTF8();

/**
 * @brief 恢复控制台原始编码（Windows专用）
 */
void restoreConsoleEncoding();

/**
 * @brief 检查字符串是否包含非ASCII字符
 * @param str 要检查的字符串
 * @return 如果包含非ASCII字符返回true
 */
bool containsNonASCII(const std::string& str);

} // namespace core
} // namespace tinakit
