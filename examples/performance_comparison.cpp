/**
 * @file performance_comparison.cpp
 * @brief TinaKit 性能对比测试
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/tinakit.hpp"
#include "tinakit/core/performance_optimizations.hpp"
#include "tinakit/core/cache_system.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <random>
#include <iomanip>

using namespace tinakit;
using namespace std::chrono;

class PerformanceComparison {
public:
    void run_all_tests() {
        std::cout << "=== TinaKit 性能对比测试 ===" << std::endl;
        
        // 重置性能计数器
        core::g_perf_counter.reset();
        
        test_cache_effectiveness();
        test_memory_efficiency();
        test_batch_vs_individual();
        test_string_optimization();
        
        // 打印最终统计
        std::cout << "\n" << std::endl;
        core::g_perf_counter.print_stats();
        core::CacheManager::instance().print_cache_stats();
    }

private:
    void test_cache_effectiveness() {
        std::cout << "\n=== 缓存效果测试 ===" << std::endl;
        
        auto workbook = excel::workbook::create();
        auto sheet = workbook.active_sheet();
        
        // 第一次访问 - 应该全部缓存未命中
        std::cout << "第一次访问100个单元格..." << std::endl;
        auto start = high_resolution_clock::now();
        
        for (int i = 1; i <= 100; ++i) {
            sheet.cell(i, 1).value("First access " + std::to_string(i));
        }
        
        auto end = high_resolution_clock::now();
        auto first_duration = duration_cast<microseconds>(end - start);
        auto first_hit_ratio = sheet.cache_hit_ratio();
        
        std::cout << "首次访问耗时: " << first_duration.count() << "μs" << std::endl;
        std::cout << "缓存命中率: " << std::fixed << std::setprecision(2) 
                  << (first_hit_ratio * 100) << "%" << std::endl;
        
        // 第二次访问相同单元格 - 应该有高缓存命中率
        std::cout << "\n第二次访问相同单元格..." << std::endl;
        start = high_resolution_clock::now();
        
        for (int i = 1; i <= 100; ++i) {
            auto value = sheet.cell(i, 1).as<std::string>();
            (void)value; // 避免未使用变量警告
        }
        
        end = high_resolution_clock::now();
        auto second_duration = duration_cast<microseconds>(end - start);
        auto second_hit_ratio = sheet.cache_hit_ratio();
        
        std::cout << "二次访问耗时: " << second_duration.count() << "μs" << std::endl;
        std::cout << "缓存命中率: " << std::fixed << std::setprecision(2) 
                  << (second_hit_ratio * 100) << "%" << std::endl;
        
        // 计算性能提升
        double speedup = static_cast<double>(first_duration.count()) / second_duration.count();
        std::cout << "性能提升: " << std::fixed << std::setprecision(2) 
                  << speedup << "x" << std::endl;
    }
    
    void test_memory_efficiency() {
        std::cout << "\n=== 内存效率测试 ===" << std::endl;
        
        // 测试FastPosition vs std::pair
        std::cout << "数据结构内存对比:" << std::endl;
        std::cout << "  std::pair<size_t, size_t>: " << sizeof(std::pair<std::size_t, std::size_t>) << " bytes" << std::endl;
        std::cout << "  FastPosition: " << sizeof(core::FastPosition) << " bytes" << std::endl;
        
        double memory_saving = 1.0 - static_cast<double>(sizeof(core::FastPosition)) / sizeof(std::pair<std::size_t, std::size_t>);
        std::cout << "  内存节省: " << std::fixed << std::setprecision(1) 
                  << (memory_saving * 100) << "%" << std::endl;
        
        // 测试内存池效率
        std::cout << "\n内存池效率测试:" << std::endl;
        
        const int alloc_count = 50000;
        
        // 标准分配
        auto start = high_resolution_clock::now();
        std::vector<int*> standard_ptrs;
        for (int i = 0; i < alloc_count; ++i) {
            standard_ptrs.push_back(new int(i));
        }
        for (auto ptr : standard_ptrs) {
            delete ptr;
        }
        auto end = high_resolution_clock::now();
        auto standard_time = duration_cast<microseconds>(end - start);
        
        // 内存池分配
        start = high_resolution_clock::now();
        core::MemoryPool<int> pool;
        std::vector<int*> pool_ptrs;
        for (int i = 0; i < alloc_count; ++i) {
            int* ptr = pool.allocate();
            *ptr = i;
            pool_ptrs.push_back(ptr);
        }
        for (auto ptr : pool_ptrs) {
            pool.deallocate(ptr);
        }
        end = high_resolution_clock::now();
        auto pool_time = duration_cast<microseconds>(end - start);
        
        std::cout << "  标准分配 " << alloc_count << " 个int: " << standard_time.count() << "μs" << std::endl;
        std::cout << "  内存池分配 " << alloc_count << " 个int: " << pool_time.count() << "μs" << std::endl;
        
        double pool_speedup = static_cast<double>(standard_time.count()) / pool_time.count();
        std::cout << "  内存池性能提升: " << std::fixed << std::setprecision(2) 
                  << pool_speedup << "x" << std::endl;
    }
    
    void test_batch_vs_individual() {
        std::cout << "\n=== 批量操作 vs 单个操作 ===" << std::endl;
        
        auto workbook = excel::workbook::create();
        auto sheet = workbook.active_sheet();
        
        const int operation_count = 10000;
        
        // 单个操作
        auto start = high_resolution_clock::now();
        for (int i = 1; i <= operation_count; ++i) {
            sheet.cell(i, 1).value("Individual " + std::to_string(i));
        }
        auto end = high_resolution_clock::now();
        auto individual_time = duration_cast<milliseconds>(end - start);
        
        // 清空缓存重新测试
        sheet.clear_cache();
        
        // 批量操作（模拟）
        start = high_resolution_clock::now();
        for (int i = 1; i <= operation_count; ++i) {
            sheet.cell(i, 2).value("Batch " + std::to_string(i));
        }
        end = high_resolution_clock::now();
        auto batch_time = duration_cast<milliseconds>(end - start);
        
        std::cout << "单个操作 " << operation_count << " 次: " << individual_time.count() << "ms" << std::endl;
        std::cout << "批量操作 " << operation_count << " 次: " << batch_time.count() << "ms" << std::endl;
        
        double individual_ops_per_sec = static_cast<double>(operation_count) / individual_time.count() * 1000;
        double batch_ops_per_sec = static_cast<double>(operation_count) / batch_time.count() * 1000;
        
        std::cout << "单个操作速度: " << std::fixed << std::setprecision(0) << individual_ops_per_sec << " ops/sec" << std::endl;
        std::cout << "批量操作速度: " << std::fixed << std::setprecision(0) << batch_ops_per_sec << " ops/sec" << std::endl;
    }
    
    void test_string_optimization() {
        std::cout << "\n=== 字符串优化测试 ===" << std::endl;
        
        auto& cache_manager = core::CacheManager::instance();
        auto& string_cache = cache_manager.string_cache();
        
        // 测试重复字符串的优化效果
        std::vector<std::string> test_strings = {
            "Common String 1", "Common String 2", "Common String 3",
            "Repeated Text", "Standard Value", "Default Content"
        };
        
        const int repeat_count = 2000;
        
        auto start = high_resolution_clock::now();
        
        // 模拟大量重复字符串使用
        for (int i = 0; i < repeat_count; ++i) {
            for (const auto& str : test_strings) {
                string_cache.intern_string(str);
            }
        }
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        
        std::cout << "处理 " << (repeat_count * test_strings.size()) << " 个字符串: " 
                  << duration.count() << "μs" << std::endl;
        std::cout << "字符串池大小: " << string_cache.string_pool_.size() << std::endl;
        std::cout << "去重效率: " << std::fixed << std::setprecision(1)
                  << (1.0 - static_cast<double>(string_cache.string_pool_.size()) / (repeat_count * test_strings.size())) * 100
                  << "%" << std::endl;
        
        // 优化字符串缓存
        string_cache.optimize_shared_strings();
    }
};

int main() {
    try {
        PerformanceComparison comparison;
        comparison.run_all_tests();
        
        std::cout << "\n性能对比测试完成！" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    }
}
