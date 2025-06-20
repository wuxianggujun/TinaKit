/**
 * @file performance_optimizations.hpp
 * @brief TinaKit 性能优化核心组件
 * @author TinaKit Team
 * @date 2025-6-20
 */

#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>
#include <string_view>
#include <string>
#include <variant>
#include <chrono>
#include <functional>
#include <optional>
#include <algorithm>
#include <cstring>

namespace tinakit::core {

/**
 * @brief 高性能位置类型，使用单个64位整数存储行列信息
 */
class FastPosition {
public:
    constexpr FastPosition() noexcept : packed_(0) {}
    constexpr FastPosition(std::uint32_t row, std::uint32_t col) noexcept 
        : packed_((static_cast<std::uint64_t>(row) << 32) | col) {}
    
    constexpr std::uint32_t row() const noexcept { return static_cast<std::uint32_t>(packed_ >> 32); }
    constexpr std::uint32_t column() const noexcept { return static_cast<std::uint32_t>(packed_ & 0xFFFFFFFF); }
    constexpr std::uint64_t packed() const noexcept { return packed_; }
    
    constexpr bool operator==(const FastPosition& other) const noexcept {
        return packed_ == other.packed_;
    }
    
    constexpr bool operator<(const FastPosition& other) const noexcept {
        return packed_ < other.packed_;
    }

private:
    std::uint64_t packed_;
};

/**
 * @brief FastPosition的哈希函数
 */
struct FastPositionHash {
    std::size_t operator()(const FastPosition& pos) const noexcept {
        return std::hash<std::uint64_t>{}(pos.packed());
    }
};

/**
 * @brief 高性能字符串池，减少字符串复制
 */
class StringPool {
public:
    using StringId = std::uint32_t;
    static constexpr StringId INVALID_ID = 0;

    StringId intern(std::string_view str) {
        if (str.empty()) return INVALID_ID;

        // 先转换为string以避免string_view失效问题
        std::string str_copy(str);
        auto it = string_to_id_.find(str_copy);
        if (it != string_to_id_.end()) {
            return it->second;
        }

        StringId id = static_cast<StringId>(strings_.size() + 1);
        strings_.emplace_back(std::move(str_copy));
        // 使用存储在vector中的字符串作为key
        string_to_id_[strings_.back()] = id;
        return id;
    }

    std::string_view get_string(StringId id) const noexcept {
        if (id == INVALID_ID || id > strings_.size()) {
            return {};
        }
        return strings_[id - 1];
    }

    void clear() {
        strings_.clear();
        string_to_id_.clear();
    }

    void reserve(std::size_t size) {
        strings_.reserve(size);
        string_to_id_.reserve(size);
    }

    std::size_t size() const noexcept { return strings_.size(); }

private:
    std::vector<std::string> strings_;
    std::unordered_map<std::string, StringId> string_to_id_;
};

/**
 * @brief 内存池分配器，减少小对象分配开销
 */
template<typename T, std::size_t BlockSize = 4096>
class MemoryPool {
public:
    MemoryPool() {
        allocate_new_block();
    }

    ~MemoryPool() {
        clear();
    }

    T* allocate() {
        if (!free_list_.empty()) {
            T* ptr = free_list_.back();
            free_list_.pop_back();
            return ptr;
        }

        if (current_block_ >= blocks_.size() ||
            blocks_[current_block_]->used >= BlockSize) {
            allocate_new_block();
        }

        auto& block = blocks_[current_block_];
        T* ptr = reinterpret_cast<T*>(block->data + block->used * sizeof(T));
        ++block->used;
        return ptr;
    }

    void deallocate(T* ptr) {
        if (ptr) {
            ptr->~T();
            free_list_.push_back(ptr);
        }
    }

    void clear() {
        for (auto& block : blocks_) {
            for (std::size_t i = 0; i < block->used; ++i) {
                T* ptr = reinterpret_cast<T*>(block->data + i * sizeof(T));
                ptr->~T();
            }
        }
        blocks_.clear();
        free_list_.clear();
        current_block_ = 0;
    }

private:
    struct Block {
        alignas(T) char data[BlockSize * sizeof(T)];
        std::size_t used = 0;
    };

    std::vector<std::unique_ptr<Block>> blocks_;
    std::vector<T*> free_list_;
    std::size_t current_block_ = 0;

    void allocate_new_block() {
        blocks_.push_back(std::make_unique<Block>());
        current_block_ = blocks_.size() - 1;
    }
};

/**
 * @brief 高性能缓存，使用LRU策略
 */
template<typename Key, typename Value, std::size_t MaxSize = 1024>
class LRUCache {
public:
    LRUCache() {
        head_ = new Node{};
        tail_ = new Node{};
        head_->next = tail_;
        tail_->prev = head_;
    }

    ~LRUCache() {
        clear();
        delete head_;
        delete tail_;
    }

    bool get(const Key& key, Value& value) {
        auto it = cache_.find(key);
        if (it == cache_.end()) {
            return false;
        }

        value = it->second->value;
        move_to_front(it->second.get());
        return true;
    }

    void put(const Key& key, const Value& value) {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            it->second->value = value;
            move_to_front(it->second.get());
            return;
        }

        if (cache_.size() >= MaxSize) {
            remove_tail();
        }

        auto node = std::make_unique<Node>();
        node->key = key;
        node->value = value;

        add_to_front(node.get());
        cache_[key] = std::move(node);
    }

    void clear() {
        cache_.clear();
        head_->next = tail_;
        tail_->prev = head_;
    }

    std::size_t size() const noexcept { return cache_.size(); }

private:
    struct Node {
        Key key{};
        Value value{};
        Node* prev = nullptr;
        Node* next = nullptr;
    };

    std::unordered_map<Key, std::unique_ptr<Node>> cache_;
    Node* head_ = nullptr;
    Node* tail_ = nullptr;

    void move_to_front(Node* node) {
        remove_node(node);
        add_to_front(node);
    }

    void add_to_front(Node* node) {
        node->prev = head_;
        node->next = head_->next;
        head_->next->prev = node;
        head_->next = node;
    }

    void remove_node(Node* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    void remove_tail() {
        Node* last = tail_->prev;
        if (last != head_) {
            cache_.erase(last->key);
            remove_node(last);
        }
    }
};

/**
 * @brief 批量操作优化器
 */
class BatchOptimizer {
public:
    /**
     * @brief 批量单元格更新
     */
    struct CellUpdate {
        FastPosition position;
        std::variant<std::string, double, int, bool> value;
        std::uint32_t style_id = 0;
    };
    
    void add_update(const CellUpdate& update);
    void flush_updates();
    void set_batch_size(std::size_t size) { batch_size_ = size; }

private:
    std::vector<CellUpdate> pending_updates_;
    std::size_t batch_size_ = 1000;
};

/**
 * @brief SIMD优化的数值操作
 */
namespace simd {
    void copy_doubles(const double* src, double* dst, std::size_t count);
    void add_doubles(const double* a, const double* b, double* result, std::size_t count);
    bool compare_strings_fast(const char* a, const char* b, std::size_t len);
}

/**
 * @brief 性能计数器
 */
class PerformanceCounter {
public:
    void start_timer(std::string_view name);
    void end_timer(std::string_view name);
    void increment_counter(std::string_view name);
    void print_stats() const;
    void reset();

private:
    struct TimerData {
        std::chrono::high_resolution_clock::time_point start;
        std::chrono::nanoseconds total{0};
        std::size_t count = 0;
    };
    
    std::unordered_map<std::string, TimerData> timers_;
    std::unordered_map<std::string, std::size_t> counters_;
};

// 全局性能计数器实例
extern PerformanceCounter g_perf_counter;

/**
 * @brief RAII性能计时器
 */
class ScopedTimer {
public:
    explicit ScopedTimer(std::string_view name) : name_(name) {
        g_perf_counter.start_timer(name_);
    }
    
    ~ScopedTimer() {
        g_perf_counter.end_timer(name_);
    }

private:
    std::string name_;
};

#define TINAKIT_PROFILE(name) ScopedTimer _timer(name)

} // namespace tinakit::core
