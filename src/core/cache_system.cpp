/**
 * @file cache_system.cpp
 * @brief TinaKit 缓存系统实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/core/cache_system.hpp"
#include "tinakit/core/performance_optimizations.hpp"
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace tinakit::core {

// ========================================
// CellDataCache 实现
// ========================================

double CellDataCache::hit_ratio() const {
    auto total = hit_count_.load() + miss_count_.load();
    return total > 0 ? static_cast<double>(hit_count_.load()) / total : 0.0;
}

void CellDataCache::reset_stats() {
    hit_count_.store(0);
    miss_count_.store(0);
}

void CellDataCache::evict_old_entries() {
    auto now = std::chrono::steady_clock::now();
    
    // 移除过期的条目
    for (auto it = cache_.begin(); it != cache_.end();) {
        if (now - it->second.last_access > MAX_AGE) {
            it = cache_.erase(it);
        } else {
            ++it;
        }
    }
    
    // 如果仍然超过最大大小，移除最旧的条目
    if (cache_.size() >= MAX_CACHE_SIZE) {
        std::vector<std::pair<FastPosition, std::chrono::steady_clock::time_point>> entries;
        entries.reserve(cache_.size());
        
        for (const auto& [pos, data] : cache_) {
            entries.emplace_back(pos, data.last_access);
        }
        
        // 按访问时间排序
        std::sort(entries.begin(), entries.end(),
                  [](const auto& a, const auto& b) {
                      return a.second < b.second;
                  });
        
        // 移除最旧的一半条目
        std::size_t to_remove = cache_.size() / 2;
        for (std::size_t i = 0; i < to_remove; ++i) {
            cache_.erase(entries[i].first);
        }
    }
}

// ========================================
// StringCache 实现
// ========================================

StringPool::StringId StringCache::intern_string(std::string_view str) {
    record_usage(str);
    return string_pool_.intern(str);
}

std::string_view StringCache::get_string(StringPool::StringId id) const {
    return string_pool_.get_string(id);
}

void StringCache::record_usage(std::string_view str) {
    std::unique_lock lock(mutex_);
    std::string str_copy(str);  // 转换为string避免string_view失效
    auto& stats = string_stats_[str_copy];
    stats.usage_count++;
    stats.total_size = str.size();

    // 避免递归锁定：直接在这里计算should_share
    stats.should_share = (str.length() >= MIN_SHARE_LENGTH && stats.usage_count >= MIN_USAGE_COUNT);
}

bool StringCache::should_use_shared_string(std::string_view str) const {
    if (str.length() < MIN_SHARE_LENGTH) {
        return false;
    }

    std::shared_lock lock(mutex_);
    std::string str_copy(str);  // 转换为string避免string_view失效
    auto it = string_stats_.find(str_copy);
    if (it != string_stats_.end()) {
        return it->second.usage_count >= MIN_USAGE_COUNT;
    }

    return false;
}

std::size_t StringCache::size() const {
    std::shared_lock lock(mutex_);
    return string_stats_.size();
}

void StringCache::clear() {
    std::unique_lock lock(mutex_);
    string_stats_.clear();
    string_pool_.clear();
}

void StringCache::optimize_shared_strings() {
    TINAKIT_PROFILE("StringCache::optimize_shared_strings");

    std::shared_lock lock(mutex_);

    // 分析字符串使用模式
    std::vector<std::pair<std::string, StringStats>> frequent_strings;

    for (const auto& [str, stats] : string_stats_) {
        if (stats.usage_count >= MIN_USAGE_COUNT && str.length() >= MIN_SHARE_LENGTH) {
            frequent_strings.emplace_back(str, stats);
        }
    }

    // 按使用频率排序
    std::sort(frequent_strings.begin(), frequent_strings.end(),
              [](const auto& a, const auto& b) {
                  return a.second.usage_count > b.second.usage_count;
              });

    std::cout << "字符串优化统计:" << std::endl;
    std::cout << "总字符串数: " << string_stats_.size() << std::endl;
    std::cout << "高频字符串数: " << frequent_strings.size() << std::endl;

    if (!frequent_strings.empty()) {
        std::cout << "前10个高频字符串:" << std::endl;
        for (std::size_t i = 0; i < std::min(frequent_strings.size(), std::size_t(10)); ++i) {
            const auto& [str, stats] = frequent_strings[i];
            std::cout << "  \"" << str << "\" - 使用次数: " << stats.usage_count
                      << ", 长度: " << stats.total_size << std::endl;
        }
    }
}

// ========================================
// WorksheetCache 实现
// ========================================

WorksheetCache::WorksheetData* WorksheetCache::get_worksheet(std::uint32_t sheet_id) {
    std::unique_lock lock(mutex_);  // 使用写锁，因为需要修改last_access
    auto it = worksheets_.find(sheet_id);
    if (it != worksheets_.end()) {
        it->second.last_access = std::chrono::steady_clock::now();
        return &it->second;
    }
    return nullptr;
}

void WorksheetCache::put_worksheet(std::uint32_t sheet_id, WorksheetData data) {
    std::unique_lock lock(mutex_);
    
    if (worksheets_.size() >= MAX_LOADED_WORKSHEETS) {
        unload_unused_worksheets();
    }
    
    data.last_access = std::chrono::steady_clock::now();
    worksheets_[sheet_id] = std::move(data);
}

void WorksheetCache::mark_dirty(std::uint32_t sheet_id) {
    std::unique_lock lock(mutex_);
    auto it = worksheets_.find(sheet_id);
    if (it != worksheets_.end()) {
        it->second.dirty = true;
    }
}

void WorksheetCache::unload_unused_worksheets() {
    auto now = std::chrono::steady_clock::now();
    
    // 收集可以卸载的工作表
    std::vector<std::uint32_t> to_unload;
    
    for (const auto& [sheet_id, data] : worksheets_) {
        if (!data.dirty && (now - data.last_access) > WORKSHEET_TIMEOUT) {
            to_unload.push_back(sheet_id);
        }
    }
    
    // 卸载工作表
    for (auto sheet_id : to_unload) {
        worksheets_.erase(sheet_id);
    }
    
    // 如果仍然太多，卸载最旧的非脏工作表
    if (worksheets_.size() >= MAX_LOADED_WORKSHEETS) {
        std::vector<std::pair<std::uint32_t, std::chrono::steady_clock::time_point>> candidates;
        
        for (const auto& [sheet_id, data] : worksheets_) {
            if (!data.dirty) {
                candidates.emplace_back(sheet_id, data.last_access);
            }
        }
        
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b) {
                      return a.second < b.second;
                  });
        
        std::size_t to_remove = worksheets_.size() - MAX_LOADED_WORKSHEETS + 1;
        for (std::size_t i = 0; i < std::min(to_remove, candidates.size()); ++i) {
            worksheets_.erase(candidates[i].first);
        }
    }
}

void WorksheetCache::clear() {
    std::unique_lock lock(mutex_);
    worksheets_.clear();
}

// ========================================
// CacheManager 实现
// ========================================

CacheManager& CacheManager::instance() {
    static CacheManager instance;
    return instance;
}

void CacheManager::clear_all_caches() {
    cell_cache_.clear();
    style_cache_.clear();
    string_cache_.string_pool_.clear();
    worksheet_cache_.clear();
}

void CacheManager::optimize_all_caches() {
    TINAKIT_PROFILE("CacheManager::optimize_all_caches");
    
    string_cache_.optimize_shared_strings();
    worksheet_cache_.unload_unused_worksheets();
    
    std::cout << "\n缓存优化完成!" << std::endl;
}

void CacheManager::print_cache_stats() const {
    std::cout << "\n=== 缓存统计信息 ===" << std::endl;
    std::cout << "单元格缓存命中率: " << std::fixed << std::setprecision(2) 
              << (cell_cache_.hit_ratio() * 100) << "%" << std::endl;
    std::cout << "样式缓存大小: " << style_cache_.size() << std::endl;
    std::cout << "字符串池大小: " << string_cache_.string_pool_.size() << std::endl;
}

void CacheManager::set_cell_cache_size(std::size_t size) {
    // 这里可以动态调整缓存大小
}

void CacheManager::set_string_cache_enabled(bool enabled) {
    // 这里可以启用/禁用字符串缓存
}

void CacheManager::set_auto_optimization_enabled(bool enabled) {
    auto_optimization_enabled_ = enabled;
}

void CacheManager::auto_optimize_if_needed() {
    if (!auto_optimization_enabled_) return;
    
    operation_count_++;
    if (operation_count_ >= AUTO_OPTIMIZE_THRESHOLD) {
        optimize_all_caches();
        operation_count_ = 0;
    }
}

} // namespace tinakit::core
