/**
 * @file performance_optimizations.cpp
 * @brief TinaKit 性能优化实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/core/performance_optimizations.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>

namespace tinakit::core {

// 全局性能计数器实例
PerformanceCounter g_perf_counter;

// ========================================
// PerformanceCounter 实现
// ========================================

void PerformanceCounter::start_timer(std::string_view name) {
    std::string key(name);
    auto& timer = timers_[key];
    timer.start = std::chrono::high_resolution_clock::now();
}

void PerformanceCounter::end_timer(std::string_view name) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::string key(name);
    
    auto it = timers_.find(key);
    if (it != timers_.end()) {
        auto duration = end_time - it->second.start;
        it->second.total += duration;
        it->second.count++;
    }
}

void PerformanceCounter::increment_counter(std::string_view name) {
    std::string key(name);
    counters_[key]++;
}

void PerformanceCounter::print_stats() const {
    std::cout << "\n=== TinaKit 性能统计 ===" << std::endl;
    
    if (!timers_.empty()) {
        std::cout << "\n计时器统计:" << std::endl;
        std::cout << std::left << std::setw(25) << "操作名称" 
                  << std::setw(12) << "调用次数" 
                  << std::setw(15) << "总时间(ms)" 
                  << std::setw(15) << "平均时间(ms)" 
                  << std::setw(15) << "操作/秒" << std::endl;
        std::cout << std::string(82, '-') << std::endl;
        
        for (const auto& [name, timer] : timers_) {
            if (timer.count > 0) {
                auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timer.total).count();
                auto avg_ms = static_cast<double>(total_ms) / timer.count;
                auto ops_per_sec = timer.count * 1000.0 / total_ms;
                
                std::cout << std::left << std::setw(25) << name
                          << std::setw(12) << timer.count
                          << std::setw(15) << total_ms
                          << std::setw(15) << std::fixed << std::setprecision(2) << avg_ms
                          << std::setw(15) << std::fixed << std::setprecision(0) << ops_per_sec << std::endl;
            }
        }
    }
    
    if (!counters_.empty()) {
        std::cout << "\n计数器统计:" << std::endl;
        std::cout << std::left << std::setw(25) << "计数器名称" 
                  << std::setw(15) << "计数值" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        for (const auto& [name, count] : counters_) {
            std::cout << std::left << std::setw(25) << name
                      << std::setw(15) << count << std::endl;
        }
    }
}

void PerformanceCounter::reset() {
    timers_.clear();
    counters_.clear();
}

// ========================================
// SIMD 优化函数实现
// ========================================

namespace simd {

void copy_doubles(const double* src, double* dst, std::size_t count) {
    // 简化实现，实际应该使用SIMD指令
    std::copy(src, src + count, dst);
}

void add_doubles(const double* a, const double* b, double* result, std::size_t count) {
    // 简化实现，实际应该使用SIMD指令
    for (std::size_t i = 0; i < count; ++i) {
        result[i] = a[i] + b[i];
    }
}

bool compare_strings_fast(const char* a, const char* b, std::size_t len) {
    // 简化实现，实际应该使用SIMD指令进行字符串比较
    return std::memcmp(a, b, len) == 0;
}

} // namespace simd

// ========================================
// BatchOptimizer 实现
// ========================================

void BatchOptimizer::add_update(const CellUpdate& update) {
    pending_updates_.push_back(update);
    
    if (pending_updates_.size() >= batch_size_) {
        flush_updates();
    }
}

void BatchOptimizer::flush_updates() {
    if (pending_updates_.empty()) return;
    
    TINAKIT_PROFILE("BatchOptimizer::flush_updates");
    
    // 按位置排序以提高缓存局部性
    std::sort(pending_updates_.begin(), pending_updates_.end(),
              [](const CellUpdate& a, const CellUpdate& b) {
                  return a.position.packed() < b.position.packed();
              });
    
    // 批量处理更新
    for (const auto& update : pending_updates_) {
        // 这里应该调用实际的单元格更新逻辑
        // workbook_impl->set_cell_data(update.position, update.value, update.style_id);
    }
    
    pending_updates_.clear();
    g_perf_counter.increment_counter("batch_updates_flushed");
}

} // namespace tinakit::core
