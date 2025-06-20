/**
 * @file performance_benchmark.cpp
 * @brief TinaKit 性能基准测试
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/tinakit.hpp"
#include "tinakit/core/performance_optimizations.hpp"
#include "tinakit/core/cache_system.hpp"
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include <iomanip>

using namespace tinakit;
using namespace std::chrono;

class PerformanceBenchmark {
public:
    struct BenchmarkResult {
        std::string test_name;
        std::size_t operations_count;
        milliseconds duration;
        double ops_per_second;
        std::size_t memory_used_mb;
    };

    void run_all_benchmarks() {
        std::cout << "=== TinaKit 性能基准测试 ===" << std::endl;

        // 启用性能监控
        core::g_perf_counter.reset();

        // 基础操作测试
        run_cell_access_benchmark();
        run_batch_write_benchmark();
        run_large_file_benchmark();
        run_memory_usage_benchmark();
        run_cache_performance_benchmark();
        run_optimization_benchmark();

        // 打印性能统计
        core::g_perf_counter.print_stats();

        // 打印缓存统计
        core::CacheManager::instance().print_cache_stats();

        // 打印总结
        print_summary();
    }

private:
    std::vector<BenchmarkResult> results_;

    void run_cell_access_benchmark() {
        std::cout << "\n--- 单元格访问性能测试 ---" << std::endl;
        
        auto workbook = excel::workbook::create();
        auto sheet = workbook.active_sheet();
        
        const std::size_t test_count = 100000;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> row_dist(1, 1000);
        std::uniform_int_distribution<> col_dist(1, 100);
        
        // 测试随机访问
        auto start = high_resolution_clock::now();
        for (std::size_t i = 0; i < test_count; ++i) {
            auto row = row_dist(gen);
            auto col = col_dist(gen);
            sheet.cell(row, col).value(static_cast<int>(i));
        }
        auto end = high_resolution_clock::now();
        
        auto duration = duration_cast<milliseconds>(end - start);
        double ops_per_sec = static_cast<double>(test_count) / duration.count() * 1000.0;
        
        results_.push_back({
            "单元格随机访问",
            test_count,
            duration,
            ops_per_sec,
            get_memory_usage_mb()
        });
        
        std::cout << "随机访问 " << test_count << " 个单元格: " 
                  << duration.count() << "ms, " 
                  << ops_per_sec << " ops/sec" << std::endl;
    }

    void run_batch_write_benchmark() {
        std::cout << "\n--- 批量写入性能测试 ---" << std::endl;
        
        auto workbook = excel::workbook::create();
        auto sheet = workbook.active_sheet();
        
        const std::size_t rows = 1000;
        const std::size_t cols = 100;
        const std::size_t total_cells = rows * cols;
        
        // 测试顺序批量写入
        auto start = high_resolution_clock::now();
        for (std::size_t r = 1; r <= rows; ++r) {
            for (std::size_t c = 1; c <= cols; ++c) {
                sheet.cell(r, c).value(r * cols + c);
            }
        }
        auto end = high_resolution_clock::now();
        
        auto duration = duration_cast<milliseconds>(end - start);
        double ops_per_sec = static_cast<double>(total_cells) / duration.count() * 1000.0;
        
        results_.push_back({
            "批量顺序写入",
            total_cells,
            duration,
            ops_per_sec,
            get_memory_usage_mb()
        });
        
        std::cout << "批量写入 " << total_cells << " 个单元格: " 
                  << duration.count() << "ms, " 
                  << ops_per_sec << " ops/sec" << std::endl;
    }

    void run_large_file_benchmark() {
        std::cout << "\n--- 大文件处理性能测试 ---" << std::endl;
        
        const std::size_t rows = 10000;
        const std::size_t cols = 50;
        
        // 创建大文件
        auto start_create = high_resolution_clock::now();
        auto workbook = excel::workbook::create();
        auto sheet = workbook.active_sheet();
        
        for (std::size_t r = 1; r <= rows; ++r) {
            for (std::size_t c = 1; c <= cols; ++c) {
                if (c == 1) {
                    sheet.cell(r, c).value("Row " + std::to_string(r));
                } else {
                    sheet.cell(r, c).value(r * c * 1.5);
                }
            }
        }
        
        workbook.save("large_test_file.xlsx");
        auto end_create = high_resolution_clock::now();
        
        // 读取大文件
        auto start_read = high_resolution_clock::now();
        auto read_workbook = excel::workbook::load("large_test_file.xlsx");
        auto read_sheet = read_workbook.active_sheet();
        
        std::size_t read_count = 0;
        for (std::size_t r = 1; r <= rows; r += 10) { // 采样读取
            for (std::size_t c = 1; c <= cols; c += 5) {
                auto value = read_sheet.cell(r, c).as<std::string>();
                read_count++;
            }
        }
        auto end_read = high_resolution_clock::now();
        
        auto create_duration = duration_cast<milliseconds>(end_create - start_create);
        auto read_duration = duration_cast<milliseconds>(end_read - start_read);
        
        results_.push_back({
            "大文件创建",
            rows * cols,
            create_duration,
            static_cast<double>(rows * cols) / create_duration.count() * 1000.0,
            get_memory_usage_mb()
        });
        
        results_.push_back({
            "大文件读取",
            read_count,
            read_duration,
            static_cast<double>(read_count) / read_duration.count() * 1000.0,
            get_memory_usage_mb()
        });
        
        std::cout << "创建大文件 (" << rows << "x" << cols << "): " 
                  << create_duration.count() << "ms" << std::endl;
        std::cout << "读取大文件 (采样 " << read_count << " 个单元格): " 
                  << read_duration.count() << "ms" << std::endl;
    }

    void run_memory_usage_benchmark() {
        std::cout << "\n--- 内存使用测试 ---" << std::endl;
        
        auto initial_memory = get_memory_usage_mb();
        
        auto workbook = excel::workbook::create();
        auto sheet = workbook.active_sheet();
        
        const std::size_t test_sizes[] = {1000, 5000, 10000, 50000};
        
        for (auto size : test_sizes) {
            // 填充数据
            for (std::size_t i = 1; i <= size; ++i) {
                sheet.cell(i, 1).value("Test string " + std::to_string(i));
                sheet.cell(i, 2).value(i * 1.5);
                sheet.cell(i, 3).value(i % 2 == 0);
            }
            
            auto current_memory = get_memory_usage_mb();
            auto memory_per_cell = (current_memory - initial_memory) * 1024.0 / (size * 3);
            
            std::cout << "数据量: " << size << " 行, 内存使用: " 
                      << (current_memory - initial_memory) << "MB, "
                      << "每单元格: " << memory_per_cell << "KB" << std::endl;
        }
    }

    void run_cache_performance_benchmark() {
        std::cout << "\n--- 缓存性能测试 ---" << std::endl;
        
        auto workbook = excel::workbook::create();
        auto sheet = workbook.active_sheet();
        
        const std::size_t cache_test_count = 50000;
        
        // 预填充一些数据
        for (std::size_t i = 1; i <= 1000; ++i) {
            sheet.cell(i, 1).value("Cached value " + std::to_string(i));
        }
        
        // 测试缓存命中率
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> row_dist(1, 1000);
        
        auto start = high_resolution_clock::now();
        for (std::size_t i = 0; i < cache_test_count; ++i) {
            auto row = row_dist(gen);
            auto value = sheet.cell(row, 1).as<std::string>();
            (void)value; // 避免未使用变量警告
        }
        auto end = high_resolution_clock::now();
        
        auto duration = duration_cast<milliseconds>(end - start);
        double ops_per_sec = static_cast<double>(cache_test_count) / duration.count() * 1000.0;
        
        results_.push_back({
            "缓存访问测试",
            cache_test_count,
            duration,
            ops_per_sec,
            get_memory_usage_mb()
        });
        
        std::cout << "缓存访问 " << cache_test_count << " 次: "
                  << duration.count() << "ms, "
                  << ops_per_sec << " ops/sec" << std::endl;

        // 打印缓存命中率
        std::cout << "缓存命中率: " << std::fixed << std::setprecision(2)
                  << (sheet.cache_hit_ratio() * 100) << "%" << std::endl;
    }

    void run_optimization_benchmark() {
        std::cout << "\n--- 优化功能测试 ---" << std::endl;

        auto workbook = excel::workbook::create();
        auto sheet = workbook.active_sheet();

        // 简化的批量操作测试
        const std::size_t batch_size = 10000;

        auto start = high_resolution_clock::now();
        for (std::size_t i = 0; i < batch_size; ++i) {
            sheet.cell(i + 1, 1).value("Batch value " + std::to_string(i));
        }
        auto end = high_resolution_clock::now();

        auto duration = duration_cast<milliseconds>(end - start);
        double ops_per_sec = static_cast<double>(batch_size) / duration.count() * 1000.0;

        results_.push_back({
            "批量操作优化",
            batch_size,
            duration,
            ops_per_sec,
            get_memory_usage_mb()
        });

        std::cout << "批量操作 " << batch_size << " 个单元格: "
                  << duration.count() << "ms, "
                  << ops_per_sec << " ops/sec" << std::endl;

        // 测试字符串池优化
        auto& string_cache = core::CacheManager::instance().string_cache();

        start = high_resolution_clock::now();
        for (std::size_t i = 0; i < 1000; ++i) {
            std::string repeated_string = "Common String " + std::to_string(i % 10);
            string_cache.intern_string(repeated_string);
        }
        end = high_resolution_clock::now();

        duration = duration_cast<milliseconds>(end - start);
        std::cout << "字符串池测试: " << duration.count() << "ms" << std::endl;

        // 优化缓存
        core::CacheManager::instance().optimize_all_caches();
    }

    void print_summary() {
        std::cout << "\n=== 性能测试总结 ===" << std::endl;
        std::cout << std::left << std::setw(20) << "测试项目" 
                  << std::setw(12) << "操作数" 
                  << std::setw(12) << "耗时(ms)" 
                  << std::setw(15) << "操作/秒" 
                  << std::setw(12) << "内存(MB)" << std::endl;
        std::cout << std::string(71, '-') << std::endl;
        
        for (const auto& result : results_) {
            std::cout << std::left << std::setw(20) << result.test_name
                      << std::setw(12) << result.operations_count
                      << std::setw(12) << result.duration.count()
                      << std::setw(15) << std::fixed << std::setprecision(0) << result.ops_per_second
                      << std::setw(12) << result.memory_used_mb << std::endl;
        }
    }

    std::size_t get_memory_usage_mb() {
        // 简化的内存使用估算
        // 在实际实现中，应该使用平台特定的API获取准确的内存使用量
        return 0; // 占位符
    }
};

int main() {
    try {
        PerformanceBenchmark benchmark;
        benchmark.run_all_benchmarks();
        
        std::cout << "\n性能基准测试完成！" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "基准测试失败: " << e.what() << std::endl;
        return 1;
    }
}
