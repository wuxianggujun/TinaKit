/**
 * @file test_framework.hpp
 * @brief TinaKit 简单实用测试框架
 * @author TinaKit Team
 * @date 2025-6-20
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <sstream>
#include <exception>
#include <map>

namespace tinakit::test {

/**
 * @brief 测试结果
 */
struct TestResult {
    std::string test_name;
    bool passed;
    std::string error_message;
    std::chrono::milliseconds duration;
};

/**
 * @brief 测试套件结果
 */
struct TestSuiteResult {
    std::string suite_name;
    std::vector<TestResult> test_results;
    int passed_count = 0;
    int failed_count = 0;
    std::chrono::milliseconds total_duration{0};
};

/**
 * @brief 简单实用的测试框架
 */
class TestFramework {
public:
    /**
     * @brief 获取全局测试框架实例
     */
    static TestFramework& instance() {
        static TestFramework instance;
        return instance;
    }

    /**
     * @brief 注册测试用例
     */
    void register_test(const std::string& suite_name, 
                      const std::string& test_name, 
                      std::function<void()> test_func) {
        tests_[suite_name].emplace_back(test_name, std::move(test_func));
    }

    /**
     * @brief 运行所有测试
     */
    void run_all_tests() {
        std::cout << "🧪 TinaKit Test Framework\n";
        std::cout << "========================\n\n";

        auto start_time = std::chrono::high_resolution_clock::now();
        
        for (auto& [suite_name, tests] : tests_) {
            run_test_suite(suite_name, tests);
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        print_summary(total_duration);
    }

    /**
     * @brief 运行指定测试套件
     */
    void run_test_suite(const std::string& suite_name) {
        if (tests_.find(suite_name) != tests_.end()) {
            std::cout << "🧪 Running Test Suite: " << suite_name << "\n";
            std::cout << "========================\n\n";
            
            auto start_time = std::chrono::high_resolution_clock::now();
            run_test_suite(suite_name, tests_[suite_name]);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            print_suite_summary(suite_name, duration);
        } else {
            std::cout << "❌ Test suite '" << suite_name << "' not found!\n";
        }
    }

    /**
     * @brief 获取测试结果统计
     */
    void get_statistics(int& total_tests, int& passed_tests, int& failed_tests) const {
        total_tests = 0;
        passed_tests = 0;
        failed_tests = 0;
        
        for (const auto& [suite_name, suite_result] : suite_results_) {
            total_tests += static_cast<int>(suite_result.test_results.size());
            passed_tests += suite_result.passed_count;
            failed_tests += suite_result.failed_count;
        }
    }

private:
    struct TestCase {
        std::string name;
        std::function<void()> func;
        
        TestCase(std::string n, std::function<void()> f) 
            : name(std::move(n)), func(std::move(f)) {}
    };

    std::map<std::string, std::vector<TestCase>> tests_;
    std::map<std::string, TestSuiteResult> suite_results_;

    void run_test_suite(const std::string& suite_name, const std::vector<TestCase>& tests) {
        TestSuiteResult suite_result;
        suite_result.suite_name = suite_name;

        std::cout << "📁 Test Suite: " << suite_name << "\n";
        std::cout << "─────────────────────────────────\n";

        for (const auto& test : tests) {
            auto result = run_single_test(test);
            suite_result.test_results.push_back(result);
            suite_result.total_duration += result.duration;
            
            if (result.passed) {
                suite_result.passed_count++;
                std::cout << "✅ " << result.test_name << " (" << result.duration.count() << "ms)\n";
            } else {
                suite_result.failed_count++;
                std::cout << "❌ " << result.test_name << " (" << result.duration.count() << "ms)\n";
                std::cout << "   Error: " << result.error_message << "\n";
            }
        }

        std::cout << "\n";
        suite_results_[suite_name] = std::move(suite_result);
    }

    TestResult run_single_test(const TestCase& test) {
        TestResult result;
        result.test_name = test.name;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        try {
            test.func();
            result.passed = true;
        } catch (const std::exception& e) {
            result.passed = false;
            result.error_message = e.what();
        } catch (...) {
            result.passed = false;
            result.error_message = "Unknown exception";
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        return result;
    }

    void print_summary(std::chrono::milliseconds total_duration) {
        int total_tests, passed_tests, failed_tests;
        get_statistics(total_tests, passed_tests, failed_tests);

        std::cout << "📊 Test Summary\n";
        std::cout << "===============\n";
        std::cout << "Total Tests:  " << total_tests << "\n";
        std::cout << "Passed:       " << passed_tests << " ✅\n";
        std::cout << "Failed:       " << failed_tests << " ❌\n";
        std::cout << "Success Rate: " << (total_tests > 0 ? (passed_tests * 100 / total_tests) : 0) << "%\n";
        std::cout << "Total Time:   " << total_duration.count() << "ms\n\n";

        if (failed_tests > 0) {
            std::cout << "❌ Some tests failed. Check the output above for details.\n";
        } else {
            std::cout << "🎉 All tests passed!\n";
        }
    }

    void print_suite_summary(const std::string& suite_name, std::chrono::milliseconds duration) {
        const auto& suite_result = suite_results_[suite_name];
        std::cout << "📊 Suite '" << suite_name << "' Summary:\n";
        std::cout << "  Passed: " << suite_result.passed_count << " ✅\n";
        std::cout << "  Failed: " << suite_result.failed_count << " ❌\n";
        std::cout << "  Time:   " << duration.count() << "ms\n\n";
    }
};

// ========================================
// 便利宏定义
// ========================================

/**
 * @brief 注册测试用例
 */
#define TEST_CASE(suite_name, test_name) \
    void test_##suite_name##_##test_name(); \
    namespace { \
        struct TestRegistrar_##suite_name##_##test_name { \
            TestRegistrar_##suite_name##_##test_name() { \
                tinakit::test::TestFramework::instance().register_test( \
                    #suite_name, #test_name, test_##suite_name##_##test_name); \
            } \
        }; \
        static TestRegistrar_##suite_name##_##test_name registrar_##suite_name##_##test_name; \
    } \
    void test_##suite_name##_##test_name()

/**
 * @brief 断言宏
 */
#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error("Assertion failed: " #condition); \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            throw std::runtime_error("Assertion failed: !(" #condition ")"); \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            std::ostringstream oss; \
            oss << "Assertion failed: expected " << (expected) << ", got " << (actual); \
            throw std::runtime_error(oss.str()); \
        } \
    } while(0)

#define ASSERT_NE(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            std::ostringstream oss; \
            oss << "Assertion failed: expected not equal to " << (expected); \
            throw std::runtime_error(oss.str()); \
        } \
    } while(0)

#define ASSERT_THROWS(expression, exception_type) \
    do { \
        bool caught = false; \
        try { \
            expression; \
        } catch (const exception_type&) { \
            caught = true; \
        } catch (...) { \
            throw std::runtime_error("Assertion failed: wrong exception type thrown"); \
        } \
        if (!caught) { \
            throw std::runtime_error("Assertion failed: expected exception not thrown"); \
        } \
    } while(0)

#define ASSERT_NO_THROW(expression) \
    do { \
        try { \
            expression; \
        } catch (...) { \
            throw std::runtime_error("Assertion failed: unexpected exception thrown"); \
        } \
    } while(0)

} // namespace tinakit::test
