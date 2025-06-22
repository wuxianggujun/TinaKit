/**
 * @file logger.cpp
 * @brief TinaKit全局日志系统实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/core/logger.hpp"
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace tinakit {
namespace core {

// ========================================
// ConsoleHandler 实现
// ========================================

ConsoleHandler::ConsoleHandler(bool colored, bool enable_unicode)
    : colored_(colored), unicode_enabled_(enable_unicode) {
#ifdef _WIN32
    console_setup_ = false;
    if (unicode_enabled_) {
        setupConsoleEncoding();
    }
#endif
}

void ConsoleHandler::setupConsoleEncoding() {
#ifdef _WIN32
    if (console_setup_) return;

    // 保存原始编码页
    original_console_cp_ = GetConsoleCP();
    original_console_output_cp_ = GetConsoleOutputCP();

    // 设置控制台为UTF-8编码
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    // 启用虚拟终端处理（支持ANSI转义序列）
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }

    console_setup_ = true;
#endif
}

void ConsoleHandler::restoreConsoleEncoding() {
#ifdef _WIN32
    if (!console_setup_) return;

    // 恢复原始编码页
    SetConsoleCP(original_console_cp_);
    SetConsoleOutputCP(original_console_output_cp_);

    console_setup_ = false;
#endif
}

ConsoleHandler::~ConsoleHandler() {
    restoreConsoleEncoding();
}

void ConsoleHandler::handle(const LogEntry& entry) {
    if (!shouldHandle(entry.level)) return;

    std::lock_guard<std::mutex> lock(mutex_);

#ifdef _WIN32
    if (unicode_enabled_ && !console_setup_) {
        setupConsoleEncoding();
    }
#endif

    std::cout << formatEntry(entry) << std::endl;
    std::cout.flush();  // 确保立即输出
}

std::string ConsoleHandler::getColorCode(LogLevel level) const {
    if (!colored_) return "";
    
    switch (level) {
        case LogLevel::TRACE: return "\033[37m";    // 白色
        case LogLevel::DEBUG: return "\033[36m";    // 青色
        case LogLevel::INFO:  return "\033[32m";    // 绿色
        case LogLevel::WARN:  return "\033[33m";    // 黄色
        case LogLevel::ERROR: return "\033[31m";    // 红色
        case LogLevel::FATAL: return "\033[35m";    // 紫色
        default: return "";
    }
}

std::string ConsoleHandler::formatEntry(const LogEntry& entry) const {
    std::ostringstream oss;
    
    if (colored_) {
        oss << getColorCode(entry.level);
    }
    
    oss << "[" << formatTimestamp(entry.timestamp) << "] "
        << "[" << logLevelToString(entry.level) << "] "
        << "[" << entry.module << "] "
        << entry.message;
    
    if (entry.level >= LogLevel::ERROR) {
        oss << " (" << entry.function << " at " 
            << std::filesystem::path(entry.file).filename().string() 
            << ":" << entry.line << ")";
    }
    
    if (colored_) {
        oss << "\033[0m";  // 重置颜色
    }
    
    return oss.str();
}

// ========================================
// FileHandler 实现
// ========================================

FileHandler::FileHandler(const std::string& filename, bool append) 
    : filename_(filename) {
    auto mode = append ? std::ios::app : std::ios::trunc;
    file_.open(filename_, mode);
    
    if (!file_.is_open()) {
        throw std::runtime_error("无法打开日志文件: " + filename_);
    }
    
    if (append) {
        file_.seekp(0, std::ios::end);
        current_size_ = file_.tellp();
    }
}

FileHandler::~FileHandler() {
    if (file_.is_open()) {
        file_.close();
    }
}

void FileHandler::handle(const LogEntry& entry) {
    if (!shouldHandle(entry.level)) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (max_file_size_ > 0 && current_size_ >= max_file_size_) {
        rotateFile();
    }
    
    std::string formatted = formatEntry(entry);
    file_ << formatted << std::endl;
    current_size_ += formatted.length() + 1;
}

void FileHandler::setRotation(size_t max_size, int max_files) {
    max_file_size_ = max_size;
    max_files_ = max_files;
}

void FileHandler::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    file_.flush();
}

void FileHandler::rotateFile() {
    file_.close();
    
    // 轮转文件
    for (int i = max_files_ - 1; i > 0; --i) {
        std::string old_name = filename_ + "." + std::to_string(i);
        std::string new_name = filename_ + "." + std::to_string(i + 1);
        
        if (std::filesystem::exists(old_name)) {
            if (i == max_files_ - 1) {
                std::filesystem::remove(new_name);
            }
            std::filesystem::rename(old_name, new_name);
        }
    }
    
    if (std::filesystem::exists(filename_)) {
        std::filesystem::rename(filename_, filename_ + ".1");
    }
    
    file_.open(filename_, std::ios::trunc);
    current_size_ = 0;
}

std::string FileHandler::formatEntry(const LogEntry& entry) const {
    std::ostringstream oss;
    
    oss << "[" << formatTimestamp(entry.timestamp) << "] "
        << "[" << logLevelToString(entry.level) << "] "
        << "[" << entry.module << "] "
        << "[" << entry.thread_id << "] "
        << entry.message;
    
    if (entry.level >= LogLevel::WARN) {
        oss << " (" << entry.function << " at " 
            << std::filesystem::path(entry.file).filename().string() 
            << ":" << entry.line << ")";
    }
    
    return oss.str();
}

// ========================================
// Logger 实现
// ========================================

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    stop();
}

void Logger::addHandler(std::unique_ptr<LogHandler> handler) {
    handlers_.push_back(std::move(handler));
}

void Logger::clearHandlers() {
    handlers_.clear();
}

void Logger::setLevel(LogLevel level) {
    global_level_ = level;
}

void Logger::log(LogLevel level, const std::string& message, const std::string& module,
                const std::string& function, const std::string& file, int line) {
    if (level < global_level_) return;
    
    LogEntry entry(level, message, module, function, file, line);
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        log_queue_.push(std::move(entry));
    }
    
    queue_cv_.notify_one();
}

void Logger::start() {
    if (running_) return;
    
    running_ = true;
    worker_thread_ = std::thread(&Logger::workerLoop, this);
}

void Logger::stop() {
    if (!running_) return;
    
    running_ = false;
    queue_cv_.notify_all();
    
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    
    // 处理剩余的日志
    while (!log_queue_.empty()) {
        LogEntry entry = log_queue_.front();
        log_queue_.pop();
        
        for (auto& handler : handlers_) {
            handler->handle(entry);
        }
    }
}

void Logger::flush() {
    for (auto& handler : handlers_) {
        if (auto* file_handler = dynamic_cast<FileHandler*>(handler.get())) {
            file_handler->flush();
        }
    }
}

bool Logger::isEnabled(LogLevel level) const {
    return level >= global_level_;
}

void Logger::workerLoop() {
    while (running_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_cv_.wait(lock, [this] { return !log_queue_.empty() || !running_; });
        
        while (!log_queue_.empty()) {
            LogEntry entry = log_queue_.front();
            log_queue_.pop();
            lock.unlock();
            
            for (auto& handler : handlers_) {
                handler->handle(entry);
            }
            
            lock.lock();
        }
    }
}

// ========================================
// 辅助函数实现
// ========================================

std::string logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

LogLevel stringToLogLevel(const std::string& str) {
    if (str == "TRACE") return LogLevel::TRACE;
    if (str == "DEBUG") return LogLevel::DEBUG;
    if (str == "INFO")  return LogLevel::INFO;
    if (str == "WARN")  return LogLevel::WARN;
    if (str == "ERROR") return LogLevel::ERROR;
    if (str == "FATAL") return LogLevel::FATAL;
    return LogLevel::INFO;
}

std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp) {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

void initializeDefaultLogging(LogLevel console_level, const std::string& file_path, LogLevel file_level) {
    auto& logger = Logger::getInstance();

    // 添加控制台处理器（启用Unicode支持）
    auto console_handler = std::make_unique<ConsoleHandler>(true, true);
    console_handler->setLevel(console_level);
    logger.addHandler(std::move(console_handler));

    // 添加文件处理器（如果指定了文件路径）
    if (!file_path.empty()) {
        auto file_handler = std::make_unique<FileHandler>(file_path, true);
        file_handler->setLevel(file_level);
        file_handler->setRotation(10 * 1024 * 1024, 5);  // 10MB, 5个文件
        logger.addHandler(std::move(file_handler));
    }

    logger.setLevel(std::min(console_level, file_level));
    logger.start();
}

bool setupConsoleUTF8() {
#ifdef _WIN32
    // 设置控制台为UTF-8编码
    if (!SetConsoleCP(CP_UTF8) || !SetConsoleOutputCP(CP_UTF8)) {
        return false;
    }

    // 启用虚拟终端处理（支持ANSI转义序列）
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }

    return true;
#else
    // 非Windows平台通常默认支持UTF-8
    return true;
#endif
}

void restoreConsoleEncoding() {
#ifdef _WIN32
    // 恢复为系统默认编码页
    SetConsoleCP(GetACP());
    SetConsoleOutputCP(GetACP());
#endif
}

bool containsNonASCII(const std::string& str) {
    for (unsigned char c : str) {
        if (c > 127) {
            return true;
        }
    }
    return false;
}

} // namespace core
} // namespace tinakit
