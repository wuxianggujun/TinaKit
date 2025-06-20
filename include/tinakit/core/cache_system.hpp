/**
 * @file cache_system.hpp
 * @brief TinaKit 多级缓存系统
 * @author TinaKit Team
 * @date 2025-6-20
 */

#pragma once

#include "performance_optimizations.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <atomic>
#include <shared_mutex>
#include <string>
#include <variant>
#include <chrono>
#include <optional>
#include <iomanip>

namespace tinakit::core {

/**
 * @brief 单元格数据缓存
 */
class CellDataCache {
public:
    struct CachedCellData {
        std::variant<std::string, double, int, bool> value;
        std::uint32_t style_id = 0;
        std::optional<std::string> formula;
        std::chrono::steady_clock::time_point last_access;
        bool dirty = false;
    };

    bool get(const FastPosition& pos, CachedCellData& data) {
        std::shared_lock lock(mutex_);
        auto it = cache_.find(pos);
        if (it != cache_.end()) {
            data = it->second;
            data.last_access = std::chrono::steady_clock::now();
            hit_count_++;
            return true;
        }
        miss_count_++;
        return false;
    }

    void put(const FastPosition& pos, const CachedCellData& data) {
        std::unique_lock lock(mutex_);
        if (cache_.size() >= MAX_CACHE_SIZE) {
            evict_old_entries();
        }

        auto cached_data = data;
        cached_data.last_access = std::chrono::steady_clock::now();
        cache_[pos] = cached_data;
    }

    void mark_dirty(const FastPosition& pos) {
        std::unique_lock lock(mutex_);
        auto it = cache_.find(pos);
        if (it != cache_.end()) {
            it->second.dirty = true;
        }
    }

    void flush_dirty() {
        std::unique_lock lock(mutex_);
        for (auto& [pos, data] : cache_) {
            if (data.dirty) {
                // 这里应该将数据写回到实际存储
                data.dirty = false;
            }
        }
    }

    void clear() {
        std::unique_lock lock(mutex_);
        cache_.clear();
        hit_count_ = 0;
        miss_count_ = 0;
    }
    
    // 统计信息
    std::size_t hit_count() const { return hit_count_.load(); }
    std::size_t miss_count() const { return miss_count_.load(); }
    double hit_ratio() const;

private:
    static constexpr std::size_t MAX_CACHE_SIZE = 10000;
    static constexpr auto MAX_AGE = std::chrono::minutes(5);
    
    mutable std::shared_mutex mutex_;
    std::unordered_map<FastPosition, CachedCellData, FastPositionHash> cache_;
    std::atomic<std::size_t> hit_count_{0};
    std::atomic<std::size_t> miss_count_{0};
    
    void evict_old_entries();
};

/**
 * @brief 样式缓存系统
 */
class StyleCache {
public:
    struct StyleKey {
        std::string font_name;
        double font_size;
        bool bold;
        bool italic;
        std::uint32_t color;
        std::uint32_t background_color;

        bool operator==(const StyleKey& other) const noexcept {
            return font_name == other.font_name &&
                   font_size == other.font_size &&
                   bold == other.bold &&
                   italic == other.italic &&
                   color == other.color &&
                   background_color == other.background_color;
        }
    };

    struct StyleKeyHash {
        std::size_t operator()(const StyleKey& key) const noexcept {
            std::size_t h1 = std::hash<std::string>{}(key.font_name);
            std::size_t h2 = std::hash<double>{}(key.font_size);
            std::size_t h3 = std::hash<bool>{}(key.bold);
            std::size_t h4 = std::hash<bool>{}(key.italic);
            std::size_t h5 = std::hash<std::uint32_t>{}(key.color);
            std::size_t h6 = std::hash<std::uint32_t>{}(key.background_color);

            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5);
        }
    };

    std::uint32_t get_or_create_style(const StyleKey& key) {
        std::shared_lock read_lock(mutex_);
        auto it = style_cache_.find(key);
        if (it != style_cache_.end()) {
            return it->second;
        }
        read_lock.unlock();

        std::unique_lock write_lock(mutex_);
        // 双重检查锁定模式
        it = style_cache_.find(key);
        if (it != style_cache_.end()) {
            return it->second;
        }

        std::uint32_t style_id = next_style_id_++;
        style_cache_[key] = style_id;
        return style_id;
    }

    void clear() {
        std::unique_lock lock(mutex_);
        style_cache_.clear();
        next_style_id_ = 1;
    }

    std::size_t size() const {
        std::shared_lock lock(mutex_);
        return style_cache_.size();
    }

private:
    mutable std::shared_mutex mutex_;  // 添加线程安全保护
    std::unordered_map<StyleKey, std::uint32_t, StyleKeyHash> style_cache_;
    std::uint32_t next_style_id_ = 1;
};

/**
 * @brief 字符串缓存（优化共享字符串）
 */
class StringCache {
public:
    struct StringStats {
        std::size_t usage_count = 0;
        std::size_t total_size = 0;
        bool should_share = false;
    };

    StringPool::StringId intern_string(std::string_view str);
    std::string_view get_string(StringPool::StringId id) const;
    void record_usage(std::string_view str);
    bool should_use_shared_string(std::string_view str) const;
    void optimize_shared_strings();

    // 线程安全的访问接口
    std::size_t size() const;
    void clear();

public:
    StringPool string_pool_;  // 改为public以便访问

private:
    mutable std::shared_mutex mutex_;  // 添加线程安全保护
    std::unordered_map<std::string, StringStats> string_stats_;

    static constexpr std::size_t MIN_SHARE_LENGTH = 3;
    static constexpr std::size_t MIN_USAGE_COUNT = 2;
};

/**
 * @brief 工作表数据缓存
 */
class WorksheetCache {
public:
    struct WorksheetData {
        std::unordered_map<FastPosition, CellDataCache::CachedCellData, FastPositionHash> cells;
        std::string name;
        bool loaded = false;
        bool dirty = false;
        std::chrono::steady_clock::time_point last_access;
    };

    WorksheetData* get_worksheet(std::uint32_t sheet_id);
    void put_worksheet(std::uint32_t sheet_id, WorksheetData data);
    void mark_dirty(std::uint32_t sheet_id);
    void unload_unused_worksheets();
    void clear();

private:
    static constexpr std::size_t MAX_LOADED_WORKSHEETS = 5;
    static constexpr auto WORKSHEET_TIMEOUT = std::chrono::minutes(10);
    
    std::unordered_map<std::uint32_t, WorksheetData> worksheets_;
    mutable std::shared_mutex mutex_;
};

/**
 * @brief 全局缓存管理器
 */
class CacheManager {
public:
    static CacheManager& instance();
    
    CellDataCache& cell_cache() { return cell_cache_; }
    StyleCache& style_cache() { return style_cache_; }
    StringCache& string_cache() { return string_cache_; }
    WorksheetCache& worksheet_cache() { return worksheet_cache_; }
    
    void clear_all_caches();
    void optimize_all_caches();
    void print_cache_stats() const;
    
    // 配置选项
    void set_cell_cache_size(std::size_t size);
    void set_string_cache_enabled(bool enabled);
    void set_auto_optimization_enabled(bool enabled);

private:
    CacheManager() = default;
    
    CellDataCache cell_cache_;
    StyleCache style_cache_;
    StringCache string_cache_;
    WorksheetCache worksheet_cache_;
    
    bool auto_optimization_enabled_ = true;
    std::atomic<std::size_t> operation_count_{0};
    static constexpr std::size_t AUTO_OPTIMIZE_THRESHOLD = 10000;
    
    void auto_optimize_if_needed();
};

/**
 * @brief 预取策略
 */
class PrefetchStrategy {
public:
    enum class Pattern {
        Sequential,    // 顺序访问
        Random,        // 随机访问
        Block,         // 块状访问
        Adaptive       // 自适应
    };

    void record_access(const FastPosition& pos);
    std::vector<FastPosition> predict_next_accesses(const FastPosition& current, std::size_t count = 10);
    void set_pattern(Pattern pattern) { pattern_ = pattern; }
    Pattern detect_pattern() const;

private:
    Pattern pattern_ = Pattern::Adaptive;
    std::vector<FastPosition> access_history_;
    static constexpr std::size_t MAX_HISTORY_SIZE = 1000;
    
    std::vector<FastPosition> predict_sequential(const FastPosition& current, std::size_t count);
    std::vector<FastPosition> predict_block(const FastPosition& current, std::size_t count);
};

/**
 * @brief 缓存预热器
 */
class CacheWarmer {
public:
    void warm_range(const FastPosition& start, const FastPosition& end);
    void warm_worksheet(std::uint32_t sheet_id);
    void warm_frequently_used();
    
    void set_enabled(bool enabled) { enabled_ = enabled; }
    bool is_enabled() const { return enabled_; }

private:
    bool enabled_ = true;
    std::vector<FastPosition> frequently_used_positions_;
};

} // namespace tinakit::core
