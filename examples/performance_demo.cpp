/**
 * @file performance_demo.cpp
 * @brief TinaKit 性能优化演示
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

using namespace tinakit;
using namespace std::chrono;

void demo_basic_optimizations() {
    std::cout << "=== 基础性能优化演示 ===" << std::endl;
    
    // 创建工作簿
    auto workbook = excel::workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 演示1: 快速单元格访问
    std::cout << "\n1. 快速单元格访问演示" << std::endl;
    auto start = high_resolution_clock::now();
    
    for (int i = 1; i <= 1000; ++i) {
        sheet.cell(i, 1).value("Cell " + std::to_string(i));
        sheet.cell(i, 2).value(i * 1.5);
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << "创建1000个单元格耗时: " << duration.count() << "ms" << std::endl;
    std::cout << "缓存命中率: " << std::fixed << std::setprecision(2) 
              << (sheet.cache_hit_ratio() * 100) << "%" << std::endl;
}

void demo_batch_operations() {
    std::cout << "\n=== 批量操作优化演示 ===" << std::endl;

    auto workbook = excel::workbook::create();
    auto sheet = workbook.active_sheet();

    // 简化的批量操作演示
    const std::size_t batch_size = 5000;

    std::cout << "\n批量设置 " << batch_size << " 个单元格..." << std::endl;
    auto start = high_resolution_clock::now();

    // 使用传统方式进行批量操作
    for (std::size_t i = 0; i < batch_size; ++i) {
        sheet.cell(i + 1, 1).value("Batch item " + std::to_string(i));
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    std::cout << "批量操作耗时: " << duration.count() << "ms" << std::endl;
    std::cout << "操作速度: " << std::fixed << std::setprecision(0)
              << (static_cast<double>(batch_size) / duration.count() * 1000) << " ops/sec" << std::endl;
}

void demo_string_optimization() {
    std::cout << "\n=== 字符串优化演示 ===" << std::endl;
    
    auto& cache_manager = core::CacheManager::instance();
    auto& string_cache = cache_manager.string_cache();
    
    // 模拟重复字符串使用
    std::vector<std::string> common_strings = {
        "Product Name", "Price", "Quantity", "Total", "Category",
        "Description", "SKU", "Supplier", "Date", "Status"
    };
    
    std::cout << "模拟字符串使用模式..." << std::endl;
    auto start = high_resolution_clock::now();
    
    // 重复使用相同字符串
    for (int i = 0; i < 1000; ++i) {
        for (const auto& str : common_strings) {
            string_cache.intern_string(str);
        }
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    std::cout << "字符串处理耗时: " << duration.count() << "ms" << std::endl;
    std::cout << "字符串池大小: " << string_cache.string_pool_.size() << std::endl;
    
    // 优化字符串缓存
    string_cache.optimize_shared_strings();
}

void demo_cache_performance() {
    std::cout << "\n=== 缓存性能演示 ===" << std::endl;
    
    auto workbook = excel::workbook::create();
    auto sheet = workbook.active_sheet();
    
    // 预填充数据
    for (int i = 1; i <= 100; ++i) {
        sheet.cell(i, 1).value("Data " + std::to_string(i));
        sheet.cell(i, 2).value(i * 2.5);
    }
    
    // 测试随机访问性能
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);
    
    std::cout << "测试随机访问性能..." << std::endl;
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        int row = dist(gen);
        auto value = sheet.cell(row, 1).as<std::string>();
        (void)value; // 避免未使用变量警告
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    std::cout << "10000次随机访问耗时: " << duration.count() << "ms" << std::endl;
    std::cout << "缓存命中率: " << std::fixed << std::setprecision(2) 
              << (sheet.cache_hit_ratio() * 100) << "%" << std::endl;
}

void demo_memory_optimization() {
    std::cout << "\n=== 内存优化演示 ===" << std::endl;
    
    // 演示FastPosition的内存效率
    std::cout << "数据结构大小比较:" << std::endl;
    std::cout << "std::pair<size_t, size_t>: " << sizeof(std::pair<std::size_t, std::size_t>) << " bytes" << std::endl;
    std::cout << "FastPosition: " << sizeof(core::FastPosition) << " bytes" << std::endl;
    
    // 演示内存池的使用
    core::MemoryPool<int> int_pool;
    
    std::cout << "\n内存池分配测试..." << std::endl;
    auto start = high_resolution_clock::now();
    
    std::vector<int*> allocated;
    for (int i = 0; i < 10000; ++i) {
        int* ptr = int_pool.allocate();
        *ptr = i;
        allocated.push_back(ptr);
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    
    std::cout << "内存池分配10000个int耗时: " << duration.count() << "μs" << std::endl;
    
    // 清理
    for (auto ptr : allocated) {
        int_pool.deallocate(ptr);
    }
}

int main() {
    try {
        std::cout << "TinaKit 性能优化演示程序" << std::endl;
        std::cout << "==============================" << std::endl;
        
        // 启用性能监控
        core::g_perf_counter.reset();
        
        // 运行各种演示
        demo_basic_optimizations();
        demo_batch_operations();
        demo_string_optimization();
        demo_cache_performance();
        demo_memory_optimization();
        
        // 打印性能统计
        std::cout << "\n" << std::endl;
        core::g_perf_counter.print_stats();
        
        // 打印缓存统计
        core::CacheManager::instance().print_cache_stats();
        
        std::cout << "\n演示完成！" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "演示程序出错: " << e.what() << std::endl;
        return 1;
    }
}
