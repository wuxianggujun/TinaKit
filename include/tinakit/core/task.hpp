/**
 * @file task.hpp
 * @brief 异步任务和协程支持
 * @author TinaKit Team
 * @date 2024-12-16
 */

#pragma once

#include <coroutine>
#include <future>
#include <exception>
#include <memory>
#include <functional>

namespace tinakit {

/**
 * @class Task
 * @brief 异步任务类，基于 C++20 协程
 * 
 * 提供异步操作的抽象，支持 co_await、co_return 等协程操作。
 * 
 * @tparam T 任务返回值类型
 */
template<typename T = void>
class Task {
public:
    /**
     * @brief Promise 类型，协程内部使用
     */
    struct promise_type {
        /**
         * @brief 获取返回对象
         * @return Task 对象
         */
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        /**
         * @brief 初始挂起点
         * @return 不挂起
         */
        std::suspend_never initial_suspend() noexcept { return {}; }
        
        /**
         * @brief 最终挂起点
         * @return 挂起
         */
        std::suspend_always final_suspend() noexcept { return {}; }
        
        /**
         * @brief 设置返回值
         * @param value 返回值
         */
        void return_value(T value) requires (!std::is_void_v<T>) {
            result_ = std::move(value);
        }
        
        /**
         * @brief 无返回值的情况
         */
        void return_void() requires std::is_void_v<T> {}
        
        /**
         * @brief 处理未捕获的异常
         * @param exception 异常指针
         */
        void unhandled_exception() {
            exception_ = std::current_exception();
        }
        
        /**
         * @brief 获取结果
         * @return 任务结果
         */
        T get_result() requires (!std::is_void_v<T>) {
            if (exception_) {
                std::rethrow_exception(exception_);
            }
            return std::move(result_);
        }
        
        /**
         * @brief 获取结果（void 特化）
         */
        void get_result() requires std::is_void_v<T> {
            if (exception_) {
                std::rethrow_exception(exception_);
            }
        }
        
        /**
         * @brief 检查是否有异常
         * @return 如果有异常返回 true
         */
        bool has_exception() const noexcept {
            return exception_ != nullptr;
        }

    private:
        T result_{};                           ///< 任务结果
        std::exception_ptr exception_{};       ///< 异常指针
    };

public:
    /**
     * @brief 构造函数
     * @param handle 协程句柄
     */
    explicit Task(std::coroutine_handle<promise_type> handle) 
        : handle_(handle) {}
    
    /**
     * @brief 析构函数
     */
    ~Task() {
        if (handle_) {
            handle_.destroy();
        }
    }
    
    /**
     * @brief 移动构造函数
     */
    Task(Task&& other) noexcept : handle_(std::exchange(other.handle_, {})) {}
    
    /**
     * @brief 移动赋值运算符
     */
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = std::exchange(other.handle_, {});
        }
        return *this;
    }
    
    // 禁止拷贝
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

public:
    /**
     * @brief 等待任务完成
     * @return 任务结果
     */
    T wait() requires (!std::is_void_v<T>) {
        if (!handle_) {
            throw std::runtime_error("Invalid task handle");
        }
        
        // 等待协程完成
        while (!handle_.done()) {
            std::this_thread::yield();
        }
        
        return handle_.promise().get_result();
    }
    
    /**
     * @brief 等待任务完成（void 特化）
     */
    void wait() requires std::is_void_v<T> {
        if (!handle_) {
            throw std::runtime_error("Invalid task handle");
        }
        
        // 等待协程完成
        while (!handle_.done()) {
            std::this_thread::yield();
        }
        
        handle_.promise().get_result();
    }
    
    /**
     * @brief 检查任务是否完成
     * @return 如果完成返回 true
     */
    bool is_ready() const noexcept {
        return handle_ && handle_.done();
    }
    
    /**
     * @brief 检查任务是否有效
     * @return 如果有效返回 true
     */
    bool valid() const noexcept {
        return handle_ != nullptr;
    }

public:
    /**
     * @brief 协程等待器
     */
    struct awaiter {
        std::coroutine_handle<promise_type> handle_;
        
        /**
         * @brief 检查是否准备好
         * @return 如果准备好返回 true
         */
        bool await_ready() const noexcept {
            return handle_.done();
        }
        
        /**
         * @brief 挂起当前协程
         * @param awaiting_coroutine 等待的协程
         */
        void await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept {
            // 简单实现：直接恢复等待的协程
            // 实际实现中可能需要更复杂的调度逻辑
        }
        
        /**
         * @brief 获取等待结果
         * @return 任务结果
         */
        T await_resume() requires (!std::is_void_v<T>) {
            return handle_.promise().get_result();
        }
        
        /**
         * @brief 获取等待结果（void 特化）
         */
        void await_resume() requires std::is_void_v<T> {
            handle_.promise().get_result();
        }
    };
    
    /**
     * @brief 协程等待操作符
     * @return 等待器对象
     */
    awaiter operator co_await() {
        return awaiter{handle_};
    }

public:
    /**
     * @brief 创建已完成的任务
     * @param value 任务结果
     * @return 已完成的任务
     */
    static Task<T> from_result(T value) requires (!std::is_void_v<T>) {
        co_return value;
    }
    
    /**
     * @brief 创建已完成的任务（void 特化）
     * @return 已完成的任务
     */
    static Task<void> from_result() requires std::is_void_v<T> {
        co_return;
    }
    
    /**
     * @brief 创建失败的任务
     * @param exception 异常指针
     * @return 失败的任务
     */
    static Task<T> from_exception(std::exception_ptr exception) {
        std::rethrow_exception(exception);
        co_return;  // 永远不会执行到这里
    }

private:
    std::coroutine_handle<promise_type> handle_;  ///< 协程句柄
};

/**
 * @brief 等待所有任务完成
 * @tparam Tasks 任务类型包
 * @param tasks 任务列表
 * @return 包含所有结果的元组
 */
template<typename... Tasks>
auto when_all(Tasks&&... tasks) -> Task<std::tuple<typename Tasks::value_type...>> {
    // 简化实现，实际中需要更复杂的逻辑
    auto results = std::make_tuple((co_await tasks)...);
    co_return results;
}

/**
 * @brief 等待任意一个任务完成
 * @tparam T 任务结果类型
 * @param tasks 任务列表
 * @return 第一个完成的任务结果
 */
template<typename T>
Task<T> when_any(std::vector<Task<T>> tasks) {
    // 简化实现
    for (auto& task : tasks) {
        if (task.is_ready()) {
            co_return co_await task;
        }
    }
    
    // 等待第一个完成
    co_return co_await tasks[0];
}

/**
 * @brief 延迟执行
 * @param duration 延迟时间
 * @return 延迟任务
 */
Task<void> delay(std::chrono::milliseconds duration);

} // namespace tinakit

/**
 * @example task_usage.cpp
 * Task 使用示例：
 * @code
 * #include <tinakit/core/task.hpp>
 * 
 * using namespace tinakit;
 * 
 * Task<int> async_calculation() {
 *     co_await delay(std::chrono::milliseconds(100));
 *     co_return 42;
 * }
 * 
 * Task<void> process_data() {
 *     auto result = co_await async_calculation();
 *     std::cout << "结果: " << result << std::endl;
 * }
 * 
 * int main() {
 *     auto task = process_data();
 *     task.wait();
 *     return 0;
 * }
 * @endcode
 */
