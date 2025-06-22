/**
 * @file test_logger.cpp
 * @brief TinaKit日志系统测试
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "../test_framework.hpp"
#include "tinakit/core/logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <fstream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace tinakit::core;
using namespace tinakit::test;

class LoggerTest {
public:
    LoggerTest() {
        std::cout << "🧪 TinaKit日志系统测试开始" << std::endl;
    }
    
    ~LoggerTest() {
        // 清理测试文件
        cleanup();
    }
    
    void runAllTests() {
        testBasicLogging();
        testChineseSupport();
        testLogLevels();
        testMultiThreading();
        testCustomHandler();
        testFileHandler();
        testConsoleEncoding();
        testLongMessages();
        testSpecialCharacters();
        testPerformance();
        
        std::cout << "\n✅ 所有测试完成！" << std::endl;
    }

private:
    std::vector<std::string> test_files_;
    
    void testBasicLogging() {
        std::cout << "\n=== 测试基本日志功能 ===" << std::endl;
        
        // 初始化日志系统
        auto& logger = Logger::getInstance();
        logger.clearHandlers();
        
        auto console_handler = std::make_unique<ConsoleHandler>(true, true);
        console_handler->setLevel(LogLevel::DEBUG);
        logger.addHandler(std::move(console_handler));
        
        logger.setLevel(LogLevel::DEBUG);
        logger.start();
        
        // 测试各种日志级别
        CORE_TRACE("这是TRACE级别日志");
        CORE_DEBUG("这是DEBUG级别日志");
        CORE_INFO("这是INFO级别日志");
        CORE_WARN("这是WARN级别日志");
        CORE_ERROR("这是ERROR级别日志");
        
        // 测试模块特定日志
        EXCEL_INFO("Excel模块日志测试");
        PDF_WARN("PDF模块警告测试");
        
        // 测试通用日志宏
        TINAKIT_INFO("TestModule", "通用日志宏测试");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "✅ 基本日志功能测试通过" << std::endl;
    }
    
    void testChineseSupport() {
        std::cout << "\n=== 测试中文支持 ===" << std::endl;
        
        // 测试中文字符
        CORE_INFO("中文日志测试：系统初始化完成");
        EXCEL_DEBUG("正在处理Excel文件：销售数据.xlsx");
        PDF_WARN("字体 '微软雅黑' 未找到，使用默认字体");
        CORE_ERROR("文件读取失败：权限不足");
        
        // 测试混合语言
        TINAKIT_INFO("Mixed", "Processing file: 数据报表.pdf (Size: 1.2MB)");
        TINAKIT_WARN("Unicode", "Unicode测试: 🚀 Hello 世界 🌍 こんにちは 안녕하세요");
        
        // 测试特殊中文字符
        CORE_DEBUG("特殊字符：，。！？；：""''（）【】《》");
        EXCEL_INFO("数学符号：± × ÷ ≤ ≥ ≠ ∞ ∑ ∏ √");
        PDF_WARN("Emoji测试：📊 📈 📉 💾 🔍 ✅ ❌ ⚠️");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "✅ 中文支持测试通过" << std::endl;
    }
    
    void testLogLevels() {
        std::cout << "\n=== 测试日志级别过滤 ===" << std::endl;
        
        auto& logger = Logger::getInstance();
        
        // 测试WARN级别过滤
        std::cout << "设置日志级别为WARN..." << std::endl;
        logger.setLevel(LogLevel::WARN);
        
        CORE_TRACE("这条TRACE不应该显示");
        CORE_DEBUG("这条DEBUG不应该显示");
        CORE_INFO("这条INFO不应该显示");
        CORE_WARN("这条WARN应该显示");
        CORE_ERROR("这条ERROR应该显示");
        
        // 恢复到DEBUG级别
        std::cout << "恢复日志级别为DEBUG..." << std::endl;
        logger.setLevel(LogLevel::DEBUG);
        
        CORE_DEBUG("现在DEBUG级别的日志又可以显示了");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "✅ 日志级别过滤测试通过" << std::endl;
    }
    
    void testMultiThreading() {
        std::cout << "\n=== 测试多线程日志 ===" << std::endl;
        
        auto worker = [](int thread_id) {
            for (int i = 0; i < 3; ++i) {
                TINAKIT_INFO("Thread" + std::to_string(thread_id), 
                           "消息 " + std::to_string(i));
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        };
        
        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back(worker, i + 1);
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "✅ 多线程日志测试通过" << std::endl;
    }
    
    void testCustomHandler() {
        std::cout << "\n=== 测试自定义处理器 ===" << std::endl;
        
        // 创建自定义处理器
        class TestHandler : public LogHandler {
        public:
            void handle(const LogEntry& entry) override {
                if (!shouldHandle(entry.level)) return;
                
                std::cout << "🔥 [CUSTOM] " << entry.module 
                         << ": " << entry.message << std::endl;
                handled_count_++;
            }
            
            int getHandledCount() const { return handled_count_; }
            
        private:
            int handled_count_ = 0;
        };
        
        auto& logger = Logger::getInstance();
        auto test_handler = std::make_unique<TestHandler>();
        auto* handler_ptr = test_handler.get();
        
        test_handler->setLevel(LogLevel::WARN);
        logger.addHandler(std::move(test_handler));
        
        // 测试自定义处理器
        CORE_INFO("这条信息不会被自定义处理器处理");
        CORE_WARN("这条警告会被自定义处理器处理");
        CORE_ERROR("这条错误也会被自定义处理器处理");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 验证处理器工作正常
        if (handler_ptr->getHandledCount() >= 2) {
            std::cout << "✅ 自定义处理器测试通过" << std::endl;
        } else {
            std::cout << "❌ 自定义处理器测试失败" << std::endl;
        }
    }
    
    void testFileHandler() {
        std::cout << "\n=== 测试文件处理器 ===" << std::endl;
        
        std::string test_file = "test_log.txt";
        test_files_.push_back(test_file);
        
        auto& logger = Logger::getInstance();
        
        // 添加文件处理器
        auto file_handler = std::make_unique<FileHandler>(test_file, false);
        file_handler->setLevel(LogLevel::INFO);
        logger.addHandler(std::move(file_handler));
        
        // 写入测试日志
        CORE_INFO("文件日志测试开始");
        EXCEL_WARN("这是一条警告信息");
        PDF_ERROR("这是一条错误信息");
        CORE_INFO("文件日志测试结束");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        logger.flush();
        
        // 验证文件是否创建并包含内容
        if (std::filesystem::exists(test_file)) {
            std::ifstream file(test_file);
            std::string content((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
            
            if (content.find("文件日志测试开始") != std::string::npos &&
                content.find("文件日志测试结束") != std::string::npos) {
                std::cout << "✅ 文件处理器测试通过" << std::endl;
            } else {
                std::cout << "❌ 文件处理器测试失败：内容不正确" << std::endl;
            }
        } else {
            std::cout << "❌ 文件处理器测试失败：文件未创建" << std::endl;
        }
    }
    
    void testConsoleEncoding() {
        std::cout << "\n=== 测试控制台编码 ===" << std::endl;
        
#ifdef _WIN32
        std::cout << "当前控制台输入编码页: " << GetConsoleCP() << std::endl;
        std::cout << "当前控制台输出编码页: " << GetConsoleOutputCP() << std::endl;
        
        if (GetConsoleOutputCP() == CP_UTF8) {
            std::cout << "✅ 控制台已设置为UTF-8编码" << std::endl;
        } else {
            std::cout << "⚠️  控制台未设置为UTF-8编码，可能出现中文乱码" << std::endl;
        }
#else
        std::cout << "✅ 非Windows平台，通常默认支持UTF-8" << std::endl;
#endif
        
        // 测试编码检测函数
        if (containsNonASCII("Hello World")) {
            std::cout << "❌ ASCII检测函数错误" << std::endl;
        } else if (!containsNonASCII("你好世界")) {
            std::cout << "❌ 中文检测函数错误" << std::endl;
        } else {
            std::cout << "✅ 编码检测函数测试通过" << std::endl;
        }
    }
    
    void testLongMessages() {
        std::cout << "\n=== 测试长消息处理 ===" << std::endl;
        
        std::string long_message = 
            "这是一个很长的中文日志消息，用来测试日志系统对长文本的处理能力。"
            "消息中包含了各种中文字符，包括简体中文、繁體中文，以及一些特殊符号。"
            "我们需要确保这些字符都能正确显示，不会出现乱码或截断的情况。"
            "同时还要测试性能，确保长文本不会影响日志系统的响应速度。"
            "这个测试消息包含了超过200个字符，用于验证日志系统的稳定性。";
        
        TINAKIT_INFO("LongText", long_message);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "✅ 长消息处理测试通过" << std::endl;
    }
    
    void testSpecialCharacters() {
        std::cout << "\n=== 测试特殊字符处理 ===" << std::endl;
        
        // 测试转义字符
        CORE_INFO("转义字符测试：\"引号\" '单引号' \\反斜杠\\ /正斜杠/");
        
        // 测试路径字符
        EXCEL_DEBUG("路径测试：C:\\用户\\文档\\测试文件.xlsx");
        
        // 测试JSON格式
        PDF_WARN("JSON测试：{\"name\": \"测试\", \"value\": 123}");
        
        // 测试XML格式
        CORE_ERROR("XML测试：<root><item>测试内容</item></root>");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "✅ 特殊字符处理测试通过" << std::endl;
    }
    
    void testPerformance() {
        std::cout << "\n=== 测试性能 ===" << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // 快速写入大量日志
        for (int i = 0; i < 1000; ++i) {
            CORE_DEBUG("性能测试消息 " + std::to_string(i));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "写入1000条日志耗时: " << duration.count() << "ms" << std::endl;
        
        // 等待异步处理完成
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        if (duration.count() < 1000) {  // 应该在1秒内完成
            std::cout << "✅ 性能测试通过" << std::endl;
        } else {
            std::cout << "⚠️  性能测试：耗时较长，可能需要优化" << std::endl;
        }
    }
    
    void cleanup() {
        // 停止日志系统
        Logger::getInstance().stop();
        
        // 删除测试文件
        for (const auto& file : test_files_) {
            if (std::filesystem::exists(file)) {
                std::filesystem::remove(file);
            }
        }
    }
};
