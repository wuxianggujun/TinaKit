/**
 * @file task.hpp
 * @brief Async task and coroutine support
 * @author TinaKit Team
 * @date 2024-12-16
 */

#pragma once

#include <coroutine>
#include <future>
#include <exception>
#include <memory>
#include <functional>
#include <thread>
#include <chrono>

namespace tinakit {

/**
 * @class Task
 * @brief Async task class based on C++20 coroutines
 *
 * Provides abstraction for async operations, supporting co_await, co_return and other coroutine operations.
 *
 * @tparam T Task return value type
 */
template<typename T = void>
class Task {
public:
    /**
     * @brief Promise type for internal coroutine use
     */
    struct promise_type {
        /**
         * @brief Get return object
         * @return Task object
         */
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        /**
         * @brief Initial suspend point
         * @return Don't suspend
         */
        std::suspend_never initial_suspend() noexcept { return {}; }

        /**
         * @brief Final suspend point
         * @return Suspend
         */
        std::suspend_always final_suspend() noexcept { return {}; }

        /**
         * @brief Set return value
         * @param value Return value
         */
        void return_value(T value) requires (!std::is_void_v<T>) {
            result_ = std::move(value);
        }

        /**
         * @brief Handle void return case
         */
        void return_void() requires std::is_void_v<T> {}

        /**
         * @brief Handle unhandled exception
         * @param exception Exception pointer
         */
        void unhandled_exception() {
            exception_ = std::current_exception();
        }
        
        /**
         * @brief Get result
         * @return Task result
         */
        T get_result() requires (!std::is_void_v<T>) {
            if (exception_) {
                std::rethrow_exception(exception_);
            }
            return std::move(result_);
        }

        /**
         * @brief Get result (void specialization)
         */
        void get_result() requires std::is_void_v<T> {
            if (exception_) {
                std::rethrow_exception(exception_);
            }
        }

        /**
         * @brief Check if has exception
         * @return True if has exception
         */
        bool has_exception() const noexcept {
            return exception_ != nullptr;
        }

    private:
        T result_{};                           ///< Task result
        std::exception_ptr exception_{};       ///< Exception pointer
    };

public:
    /**
     * @brief Constructor
     * @param handle Coroutine handle
     */
    explicit Task(std::coroutine_handle<promise_type> handle)
        : handle_(handle) {}

    /**
     * @brief Destructor
     */
    ~Task() {
        if (handle_) {
            handle_.destroy();
        }
    }

    /**
     * @brief Move constructor
     */
    Task(Task&& other) noexcept : handle_(std::exchange(other.handle_, {})) {}

    /**
     * @brief Move assignment operator
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

    // Disable copy
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

public:
    /**
     * @brief Wait for task completion
     * @return Task result
     */
    T wait() requires (!std::is_void_v<T>) {
        if (!handle_) {
            throw std::runtime_error("Invalid task handle");
        }

        // Wait for coroutine completion
        while (!handle_.done()) {
            std::this_thread::yield();
        }

        return handle_.promise().get_result();
    }

    /**
     * @brief Wait for task completion (void specialization)
     */
    void wait() requires std::is_void_v<T> {
        if (!handle_) {
            throw std::runtime_error("Invalid task handle");
        }

        // Wait for coroutine completion
        while (!handle_.done()) {
            std::this_thread::yield();
        }

        handle_.promise().get_result();
    }

    /**
     * @brief Check if task is completed
     * @return True if completed
     */
    bool is_ready() const noexcept {
        return handle_ && handle_.done();
    }

    /**
     * @brief Check if task is valid
     * @return True if valid
     */
    bool valid() const noexcept {
        return handle_ != nullptr;
    }

public:
    /**
     * @brief Coroutine awaiter
     */
    struct awaiter {
        std::coroutine_handle<promise_type> handle_;

        /**
         * @brief Check if ready
         * @return True if ready
         */
        bool await_ready() const noexcept {
            return handle_.done();
        }

        /**
         * @brief Suspend current coroutine
         * @param awaiting_coroutine Awaiting coroutine
         */
        void await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept {
            // Simple implementation: directly resume awaiting coroutine
            // Actual implementation may need more complex scheduling logic
        }

        /**
         * @brief Get await result
         * @return Task result
         */
        T await_resume() requires (!std::is_void_v<T>) {
            return handle_.promise().get_result();
        }

        /**
         * @brief Get await result (void specialization)
         */
        void await_resume() requires std::is_void_v<T> {
            handle_.promise().get_result();
        }
    };

    /**
     * @brief Coroutine await operator
     * @return Awaiter object
     */
    awaiter operator co_await() {
        return awaiter{handle_};
    }

public:
    /**
     * @brief Create completed task
     * @param value Task result
     * @return Completed task
     */
    static Task<T> from_result(T value) requires (!std::is_void_v<T>) {
        co_return value;
    }

    /**
     * @brief Create completed task (void specialization)
     * @return Completed task
     */
    static Task<void> from_result() requires std::is_void_v<T> {
        co_return;
    }

    /**
     * @brief Create failed task
     * @param exception Exception pointer
     * @return Failed task
     */
    static Task<T> from_exception(std::exception_ptr exception) {
        std::rethrow_exception(exception);
        co_return;  // Never reached
    }

private:
    std::coroutine_handle<promise_type> handle_;  ///< Coroutine handle
};

/**
 * @brief Wait for all tasks to complete
 * @tparam Tasks Task type pack
 * @param tasks Task list
 * @return Tuple containing all results
 */
template<typename... Tasks>
auto when_all(Tasks&&... tasks) -> Task<std::tuple<typename Tasks::value_type...>> {
    // Simplified implementation, actual implementation needs more complex logic
    auto results = std::make_tuple((co_await tasks)...);
    co_return results;
}

/**
 * @brief Wait for any task to complete
 * @tparam T Task result type
 * @param tasks Task list
 * @return Result of first completed task
 */
template<typename T>
Task<T> when_any(std::vector<Task<T>> tasks) {
    // Simplified implementation
    for (auto& task : tasks) {
        if (task.is_ready()) {
            co_return co_await task;
        }
    }

    // Wait for first completion
    co_return co_await tasks[0];
}

/**
 * @brief Delay execution
 * @param duration Delay duration
 * @return Delay task
 */
Task<void> delay(std::chrono::milliseconds duration);

} // namespace tinakit

/**
 * @example task_usage.cpp
 * Task usage example:
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
 *     std::cout << "Result: " << result << std::endl;
 * }
 *
 * int main() {
 *     auto task = process_data();
 *     task.wait();
 *     return 0;
 * }
 * @endcode
 */
