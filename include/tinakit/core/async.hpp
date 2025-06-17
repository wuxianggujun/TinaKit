#pragma once

#include <coroutine>
#include <exception>
#include <optional>
#include <utility>
#include <tuple>
#include <type_traits>
#include <stdexcept>
#include <memory>
#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace tinakit::async {

// Forward declarations
template<typename T>
class Task;
class Executor;
class CancellationToken;
class CancellationTokenSource;
class ThreadPoolExecutor;

//==================================================================================================
// 1. Executor Model
// Defines the "where" and "how" of coroutine execution.
//==================================================================================================

/**
 * @brief Abstract base class for execution contexts.
 *
 * An Executor is responsible for scheduling a coroutine handle for resumption.
 * This allows controlling on which thread or in what manner a piece of work continues.
 */
class Executor {
public:
    virtual ~Executor() = default;
    virtual void execute(std::coroutine_handle<> handle) = 0;
};

/**
 * @brief An executor that runs the coroutine inline on the current thread.
 */
class InlineExecutor final : public Executor {
public:
    void execute(std::coroutine_handle<> handle) override {
        handle.resume();
    }
};

/**
 * @brief A simple thread pool executor for running tasks on background threads.
 */
class ThreadPoolExecutor final : public Executor {
public:
    explicit ThreadPoolExecutor(size_t thread_count = std::thread::hardware_concurrency()) {
        if (thread_count == 0) thread_count = 1;
        for (size_t i = 0; i < thread_count; ++i) {
            threads_.emplace_back([this] { worker_thread(); });
        }
    }

    ~ThreadPoolExecutor() override
    {
        shutdown_ = true;
        cv_.notify_all();
        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    ThreadPoolExecutor(const ThreadPoolExecutor&) = delete;
    ThreadPoolExecutor& operator=(const ThreadPoolExecutor&) = delete;

    void execute(std::coroutine_handle<> handle) override {
        if (!handle) return;
        {
            std::lock_guard lock(mutex_);
            tasks_.push(handle);
        }
        cv_.notify_one();
    }

private:
    void worker_thread() {
        while (true) {
            std::coroutine_handle<> handle;
            {
                std::unique_lock lock(mutex_);
                cv_.wait(lock, [this] { return shutdown_ || !tasks_.empty(); });
                if (shutdown_ && tasks_.empty()) {
                    return;
                }
                handle = tasks_.front();
                tasks_.pop();
            }
            handle.resume();
        }
    }

    std::vector<std::thread> threads_;
    std::queue<std::coroutine_handle<>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> shutdown_{false};
};

/**
 * @brief Creates an awaitable that schedules the coroutine on the specified executor.
 *
 * Usage: `co_await fcoro::schedule_on(my_executor);`
 */
auto schedule_on(Executor& executor) noexcept {
    struct Awaiter {
        Executor& executor_;
        bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<> handle) const noexcept {
            executor_.execute(handle);
        }
        void await_resume() const noexcept {}
    };
    return Awaiter{executor};
}


//==================================================================================================
// 2. Cancellation Mechanism
//==================================================================================================

class OperationCanceledException : public std::runtime_error {
public:
    OperationCanceledException() : std::runtime_error("Operation was canceled") {}
};

class CancellationState {
public:
    void cancel() noexcept { canceled_.store(true, std::memory_order_release); }
    bool is_cancellation_requested() const noexcept { return canceled_.load(std::memory_order_acquire); }
private:
    std::atomic<bool> canceled_{false};
};

/**
 * @brief Represents a token that can be checked for a cancellation request.
 */
class CancellationToken {
public:
    CancellationToken() = default;
    explicit CancellationToken(std::shared_ptr<CancellationState> state) : state_(std::move(state)) {}

    bool is_cancellation_requested() const noexcept {
        return state_ && state_->is_cancellation_requested();
    }

    void throwIfCancellationRequested() const {
        if (is_cancellation_requested()) {
            throw OperationCanceledException();
        }
    }

private:
    std::shared_ptr<CancellationState> state_;
};

/**
 * @brief Creates and manages a CancellationToken.
 */
class CancellationTokenSource {
public:
    CancellationTokenSource() : state_(std::make_shared<CancellationState>()) {}

    void cancel() noexcept {
        if (state_) {
            state_->cancel();
        }
    }

    CancellationToken token() const noexcept {
        return CancellationToken(state_);
    }

private:
    std::shared_ptr<CancellationState> state_;
};


//==================================================================================================
// 3. Core Task Implementation
//==================================================================================================

namespace detail {
    template<typename T>
    struct Promise;

    // Awaiter for the final suspend point of a Task.
    // This is responsible for resuming the waiting coroutine (the continuation).
    struct FinalAwaiter {
        bool await_ready() const noexcept { return false; }

        template<typename PromiseType>
        std::coroutine_handle<> await_suspend(std::coroutine_handle<PromiseType> h) noexcept {
            // Resume the continuation. If no one is awaiting, this is a no-op handle.
            // If an executor is specified, resume via the executor.
            auto& promise = h.promise();
            if (promise.executor_ && promise.continuation_) {
                 promise.executor_->execute(promise.continuation_);
                 return std::noop_coroutine();
            }
            return promise.continuation_ ? promise.continuation_ : std::noop_coroutine();
        }
        void await_resume() noexcept {}
    };

    // Base promise type with common logic for cancellation and continuation.
    struct PromiseBase {
        std::coroutine_handle<> continuation_ = nullptr;
        std::exception_ptr exception_ = nullptr;
        CancellationToken cancellation_token_;
        Executor* executor_ = nullptr; // Optional executor for scheduling the continuation

        // A coroutine is always "lazy", starting only when awaited.
        std::suspend_always initial_suspend() const noexcept { return {}; }
        FinalAwaiter final_suspend() const noexcept { return {}; }

        void unhandled_exception() noexcept {
            exception_ = std::current_exception();
        }

        void set_cancellation_token(CancellationToken token) {
            cancellation_token_ = std::move(token);
        }
    };

    // Promise specialization for Tasks that return a value.
    template<typename T>
    struct Promise final : PromiseBase {
        std::optional<T> result_;

        Task<T> get_return_object();

        template<typename U>
        void return_value(U&& value) noexcept(std::is_nothrow_constructible_v<T, U&&>) {
            result_.emplace(std::forward<U>(value));
        }

        T& result() & {
            if (exception_) std::rethrow_exception(exception_);
            return *result_;
        }
        
        T&& result() && {
            if (exception_) std::rethrow_exception(exception_);
            return std::move(*result_);
        }
    };

    // Promise specialization for Task<void>.
    template<>
    struct Promise<void> final : PromiseBase {
        Task<void> get_return_object();
        void return_void() noexcept {}

        void result() {
            if (exception_) std::rethrow_exception(exception_);
        }
    };
} // namespace detail


/**
 * @brief Represents an asynchronous operation that may produce a result.
 *
 * A Task is lazy (cold-started) and only executes when `co_await`ed.
 * It is a move-only type, representing unique ownership of the async operation.
 */
template<typename T = void>
class Task {
public:
    using promise_type = detail::Promise<T>;
    friend struct detail::Promise<T>;
    friend struct detail::Promise<void>;

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}
    
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (handle_) handle_.destroy();
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }

    ~Task() {
        if (handle_) {
            handle_.destroy();
        }
    }

    // Awaiter implementation for `co_await my_task;`
    auto operator co_await() & noexcept {
        struct Awaiter {
            std::coroutine_handle<promise_type> handle_;

            bool await_ready() const noexcept {
                // If the handle is done, we can get the result immediately.
                return !handle_ || handle_.done();
            }

            std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaiting_coro) noexcept {
                // Store the waiting coroutine's handle as the continuation.
                // It will be resumed in FinalAwaiter.
                handle_.promise().continuation_ = awaiting_coro;
                
                // Return the task's handle to transfer execution to it.
                return handle_;
            }

            T await_resume() {
                // This is called after the task is complete.
                // The result() method will rethrow exceptions if any.
                if constexpr (std::is_void_v<T>) {
                    handle_.promise().result();
                } else {
                    return std::move(handle_.promise()).result();
                }
            }
        };
        return Awaiter{handle_};
    }

    // Configure task with a cancellation token.
    Task<T> with_cancellation(CancellationToken token) && {
        handle_.promise().set_cancellation_token(std::move(token));
        return std::move(*this);
    }
    
    // Configure task to schedule its continuation on a specific executor.
    Task<T> via(Executor& executor) && {
        handle_.promise().executor_ = &executor;
        return std::move(*this);
    }

private:
    explicit Task(std::coroutine_handle<promise_type> handle) noexcept : handle_(handle) {}
    std::coroutine_handle<promise_type> handle_;
};

// Connect promise types back to the Task class
namespace detail {
    template<typename T>
    Task<T> Promise<T>::get_return_object() {
        return Task<T>{std::coroutine_handle<Promise<T>>::from_promise(*this)};
    }
    inline Task<void> Promise<void>::get_return_object() {
        return Task<void>{std::coroutine_handle<Promise<void>>::from_promise(*this)};
    }
} // namespace detail


//==================================================================================================
// 4. Concurrency Combinators (`when_all`)
//==================================================================================================
namespace detail {
    // Helper to get the result type T from a Task<T>.
    // For Task<void>, we use a placeholder type `void_result` because tuples cannot contain `void`.
    struct void_result {};
    
    template<typename T> struct task_result_impl { using type = T; };
    template<> struct task_result_impl<void> { using type = void_result; };
    template<typename T> using task_result_t = typename task_result_impl<T>::type;

    template<typename T> struct task_value_type;
    template<typename T> struct task_value_type<Task<T>> { using type = T; };
    template<typename T> using task_value_t = typename task_value_type<T>::type;


    // Implementation of when_all using compile-time index sequences.
    template <typename Tuple, std::size_t... Is>
    auto when_all_impl(Tuple&& tasks, std::index_sequence<Is...>) -> 
        Task<std::tuple<task_result_t<task_value_t<std::decay_t<decltype(std::get<Is>(tasks))>>>...>> 
    {
        // Use a fold expression with the comma operator to `co_await` all tasks concurrently.
        // The results are then collected into a tuple.
        co_return std::make_tuple((co_await std::get<Is>(std::forward<Tuple>(tasks)))...);
    }
} // namespace detail

/**
 * @brief Concurrently awaits multiple tasks and returns a task whose result is a tuple of all individual task results.
 *
 * For any `Task<void>` inputs, the corresponding tuple element will be of type `fcoro::detail::void_result`.
 *
 * @param tasks A parameter pack of `Task` objects to await.
 * @return A new `Task` that completes when all input tasks are complete.
 */
template <typename... Tasks>
auto when_all(Tasks&&... tasks) {
    return detail::when_all_impl(
        std::make_tuple(std::forward<Tasks>(tasks)...),
        std::index_sequence_for<Tasks...>{}
    );
}

//==================================================================================================
// 5. Utility Functions (`sync_wait`)
//==================================================================================================

/**
 * @brief Blocks the current thread until the task is complete and returns its result.
 * 
 * This is primarily useful for bridging the async world with a blocking context,
 * such as in a `main` function or in unit tests.
 * 
 * @warning Do not use this within a coroutine; it will lead to deadlocks.
 *
 * @param task The task to wait for.
 * @return The result of the task.
 */
template <typename T>
T sync_wait(Task<T>&& task) {
    auto task_runner = [](Task<T> t) -> Task<std::optional<T>> {
        // co_await the task and capture its result
        if constexpr (std::is_void_v<T>) {
            co_await std::move(t);
            co_return std::nullopt; // Placeholder for void
        } else {
            co_return co_await std::move(t);
        }
    };
    
    auto wrapper_task = task_runner(std::move(task));
    
    // Manually start the wrapper task and block until it completes
    std::mutex m;
    std::condition_variable cv;
    bool done = false;

    // Use a final action lambda to signal completion
    auto final_action = [&]() -> Task<> {
        // This code will run when wrapper_task completes
        {
            std::lock_guard lock(m);
            done = true;
        }
        cv.notify_one();
        co_return;
    };
    
    // Chain the signaling task to the end of our task
    auto final_task_runner = [](Task<std::optional<T>> t, auto final_act) -> Task<std::optional<T>> {
        auto result = co_await std::move(t);
        co_await final_act();
        co_return result;
    };

    auto task_with_signal = final_task_runner(std::move(wrapper_task), final_action);

    // This is the only place we manually resume. 
    // We create a coroutine from the task but don't await it.
    // Instead, we resume it to kick it off.
    auto handle = std::get<0>(task_with_signal.operator co_await().await_suspend(std::noop_coroutine()));
    handle.resume();

    // Block until the task is done
    std::unique_lock lock(m);
    cv.wait(lock, [&done] { return done; });

    // Retrieve and return the result
    if constexpr (std::is_void_v<T>) {
        // In the void case, we already know it completed.
        // We just need to check for exceptions.
        task_with_signal.operator co_await().await_resume();
        return;
    } else {
        return *task_with_signal.operator co_await().await_resume();
    }
}

} // namespace fcoro
