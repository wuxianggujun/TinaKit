/**
 * @file main.cpp
 * @brief TinaKit 测试主程序
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "test_framework.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        auto& framework = tinakit::test::TestFramework::instance();
        
        if (argc > 1) {
            // 运行指定的测试套件
            std::string suite_name = argv[1];
            framework.run_test_suite(suite_name);
        } else {
            // 运行所有测试
            framework.run_all_tests();
        }
        
        // 获取测试结果统计
        int total_tests, passed_tests, failed_tests;
        framework.get_statistics(total_tests, passed_tests, failed_tests);
        
        // 返回适当的退出码
        return (failed_tests > 0) ? 1 : 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test framework error: " << e.what() << std::endl;
        return 2;
    } catch (...) {
        std::cerr << "❌ Unknown test framework error" << std::endl;
        return 2;
    }
}
