#pragma once

//==================================================================================================
// TinaKit Async: C++20 Coroutine Library
// File: async.hpp
//
// Description:
// This is a high-performance C++20 coroutine library header file that integrates the following core features:
// 1. **Lock-free thread pool executor**: Based on MPMC (Multi-Producer Multi-Consumer) lock-free queue
// 2. **Timeout and cancellation mechanism**: Efficient background timer wheel for coroutine task timeout
// 3. **Optional memory pool**: Enable high-performance memory pool via TINAKIT_ENABLE_MEMORY_POOL macro
//
// This library is designed as a single header file for easy integration and use.
//==================================================================================================

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
#include <atomic>
#include <semaphore>          // For std::counting_semaphore in lock-free executor
#include <numeric>            // For std::has_single_bit
#include <chrono>             // For timeout mechanism
#include <list>               // For timer implementation
#include <unordered_map>      // For timer implementation
#include <mutex>              // For timer implementation
#include <condition_variable> // For timer implementation

#if defined(TINAKIT_ENABLE_MEMORY_POOL)
#include <memory_resource>
#endif

// 包含 TinaKit 异常定义
#include "tinakit/core/exceptions.hpp"

namespace tinakit::async
{
    // 前向声明
    template <typename T>
    class Task;
    class Executor;
    class CancellationToken;
    class CancellationTokenSource;
    class ThreadPoolExecutor;

    template <typename T, typename Rep, typename Period>
    Task<T> with_timeout(Task<T>&& task, std::chrono::duration<Rep, Period> timeout);

    //==================================================================================================
    // 1. 内存池实现 (条件编译)
    //==================================================================================================

#if defined(TINAKIT_ENABLE_MEMORY_POOL)
namespace detail {
    class CoroutineMemoryPoolManager {
    public:
        static CoroutineMemoryPoolManager& get_instance() {
            static CoroutineMemoryPoolManager instance;
            return instance;
        }
        std::pmr::memory_resource* get_resource() {
            return &pool_resource_;
        }
        CoroutineMemoryPoolManager(const CoroutineMemoryPoolManager&) = delete;
        CoroutineMemoryPoolManager& operator=(const CoroutineMemoryPoolManager&) = delete;
    private:
        CoroutineMemoryPoolManager() = default;
        ~CoroutineMemoryPoolManager() = default;
        std::pmr::synchronized_pool_resource pool_resource_;
    };
} // namespace detail
#endif // TINAKIT_ENABLE_MEMORY_POOL


    //==================================================================================================
    // 2. 执行器模型与无锁实现
    //==================================================================================================

    class Executor
    {
    public:
        virtual ~Executor() = default;
        virtual void execute(std::coroutine_handle<> handle) = 0;
    };

    class InlineExecutor final : public Executor
    {
    public:
        void execute(std::coroutine_handle<> handle) override
        {
            handle.resume();
        }
    };

    namespace detail
    {
        /**
         * @brief 一个有界的、无锁的、多生产者多消费者（MPMC）环形缓冲区队列。
         */
        template <typename T>
        #pragma warning(push)
        #pragma warning(disable: 4324) // structure was padded due to alignment specifier
        class MpmcRingBufferQueue
        {
            static_assert(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>,
                          "T must be nothrow move constructible and assignable");

        public:
            explicit MpmcRingBufferQueue(size_t capacity)
                : capacity_(capacity), mask_(capacity - 1), buffer_(capacity)
            {
                if (!std::has_single_bit(capacity))
                {
                    throw ExecutorException("MpmcRingBufferQueue capacity must be a power of 2.");
                }
                for (size_t i = 0; i < capacity; ++i)
                {
                    buffer_[i].seq.store(i, std::memory_order_relaxed);
                }
            }

            MpmcRingBufferQueue(const MpmcRingBufferQueue&) = delete;
            MpmcRingBufferQueue& operator=(const MpmcRingBufferQueue&) = delete;

            void push(T&& item)
            {
                size_t head = head_seq_.fetch_add(1, std::memory_order_relaxed);
                Slot& slot = buffer_[head & mask_];
                while (slot.seq.load(std::memory_order_acquire) != head)
                {
                    std::this_thread::yield();
                }
                slot.data = std::move(item);
                slot.seq.store(head + 1, std::memory_order_release);
            }

            bool try_pop(T& item)
            {
                size_t tail = tail_seq_.load(std::memory_order_relaxed);
                Slot& slot = buffer_[tail & mask_];
                if (slot.seq.load(std::memory_order_acquire) != tail + 1)
                {
                    return false;
                }
                if (tail_seq_.compare_exchange_weak(tail, tail + 1, std::memory_order_relaxed))
                {
                    item = std::move(slot.data);
                    slot.seq.store(tail + capacity_, std::memory_order_release);
                    return true;
                }
                return false;
            }

        private:
            struct Slot
            {
                alignas(std::hardware_destructive_interference_size) std::atomic<size_t> seq;
                T data;
            };

            alignas(std::hardware_destructive_interference_size) std::atomic<size_t> head_seq_{0};
            alignas(std::hardware_destructive_interference_size) std::atomic<size_t> tail_seq_{0};

            const size_t capacity_;
            const size_t mask_;
            std::vector<Slot> buffer_;
        };
        #pragma warning(pop)
    } // namespace detail


    /**
     * @brief 高性能、无锁的线程池执行器。
     */
    class ThreadPoolExecutor final : public Executor
    {
    public:
        explicit ThreadPoolExecutor(size_t thread_count = 0, size_t queue_capacity = 256)
            : queue_(queue_capacity)
        {
            if (thread_count == 0)
            {
                thread_count = std::thread::hardware_concurrency();
                if (thread_count == 0) thread_count = 1;
            }

            threads_.reserve(thread_count);
            for (size_t i = 0; i < thread_count; ++i)
            {
                threads_.emplace_back([this](std::stop_token stoken)
                {
                    worker_loop(std::move(stoken));
                });
            }
        }

        ~ThreadPoolExecutor()
        {
            for (auto& thread : threads_)
            {
                thread.request_stop();
            }
            semaphore_.release(threads_.size());
        }

        ThreadPoolExecutor(const ThreadPoolExecutor&) = delete;
        ThreadPoolExecutor& operator=(const ThreadPoolExecutor&) = delete;

        void execute(std::coroutine_handle<> handle) override
        {
            if (!handle) return;
            queue_.push(std::move(handle));
            semaphore_.release();
        }

    private:
        void worker_loop(std::stop_token stop_token)
        {
            std::coroutine_handle<> handle;
            while (!stop_token.stop_requested())
            {
                if (queue_.try_pop(handle))
                {
                    handle.resume();
                    continue;
                }
                semaphore_.acquire();
                if (queue_.try_pop(handle))
                {
                    handle.resume();
                }
            }
        }

        detail::MpmcRingBufferQueue<std::coroutine_handle<>> queue_;
        std::counting_semaphore<> semaphore_{0};
        std::vector<std::jthread> threads_;
    };


    auto schedule_on(Executor& executor) noexcept
    {
        struct Awaiter
        {
            Executor& executor_;
            bool await_ready() const noexcept { return false; }

            void await_suspend(std::coroutine_handle<> handle) const noexcept
            {
                executor_.execute(handle);
            }

            void await_resume() const noexcept
            {
            }
        };
        return Awaiter{executor};
    }


    //==================================================================================================
    // 3. 取消机制
    //==================================================================================================

    // 使用 TinaKit 异常体系中的异常类
    using OperationCanceledException = ::tinakit::OperationCanceledException;
    using ExecutorException = ::tinakit::ExecutorException;

    class CancellationState
    {
    public:
        void cancel() noexcept { canceled_.store(true, std::memory_order_release); }
        bool is_cancellation_requested() const noexcept { return canceled_.load(std::memory_order_acquire); }

    private:
        std::atomic<bool> canceled_{false};
    };

    class CancellationToken
    {
    public:
        CancellationToken() = default;

        explicit CancellationToken(std::shared_ptr<CancellationState> state) : state_(std::move(state))
        {
        }

        bool is_cancellation_requested() const noexcept
        {
            return state_ && state_->is_cancellation_requested();
        }

        void throwIfCancellationRequested() const
        {
            if (is_cancellation_requested())
            {
                throw OperationCanceledException();
            }
        }

    private:
        std::shared_ptr<CancellationState> state_;
    };

    class CancellationTokenSource
    {
    public:
        CancellationTokenSource() : state_(std::make_shared<CancellationState>())
        {
        }

        void cancel() noexcept { if (state_) state_->cancel(); }
        CancellationToken token() const noexcept { return CancellationToken(state_); }

    private:
        std::shared_ptr<CancellationState> state_;
    };


    //==================================================================================================
    // 4. 超时与定时器实现
    //==================================================================================================
    namespace detail
    {
        /**
         * @brief 高效的时间轮定时器，用于管理大量定时事件。
         */
        class TimerWheel
        {
        public:
            using TimerId = uint64_t;
            using Callback = std::function<void()>;

            TimerWheel() = default;
            TimerWheel(const TimerWheel&) = delete;
            TimerWheel& operator=(const TimerWheel&) = delete;

            template <typename Rep, typename Period>
            TimerId add_timer(std::chrono::duration<Rep, Period> timeout, Callback callback)
            {
                auto expires_at = std::chrono::steady_clock::now() + timeout;
                std::lock_guard lock(mutex_);
                TimerId id = next_timer_id_++;
                auto it = timers_.emplace(timers_.end(), id, expires_at, std::move(callback));
                timers_map_[id] = it;
                return id;
            }

            void remove_timer(TimerId id)
            {
                std::lock_guard lock(mutex_);
                auto it = timers_map_.find(id);
                if (it != timers_map_.end())
                {
                    timers_.erase(it->second);
                    timers_map_.erase(it);
                }
            }

            void tick()
            {
                auto now = std::chrono::steady_clock::now();
                std::vector<Callback> expired_callbacks;
                {
                    std::lock_guard lock(mutex_);
                    for (auto it = timers_.begin(); it != timers_.end();)
                    {
                        if (it->expires_at <= now)
                        {
                            expired_callbacks.push_back(std::move(it->callback));
                            timers_map_.erase(it->id);
                            it = timers_.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                }
                for (auto& cb : expired_callbacks)
                {
                    cb();
                }
            }

        private:
            struct TimerNode
            {
                TimerId id;
                std::chrono::steady_clock::time_point expires_at;
                Callback callback;
            };

            std::mutex mutex_;
            std::list<TimerNode> timers_;
            std::unordered_map<TimerId, std::list<TimerNode>::iterator> timers_map_;
            std::atomic<TimerId> next_timer_id_{1};
        };

        /**
         * @brief 定时器管理器的单例。
         */
        class TimerManager
        {
        public:
            static TimerManager& get_instance()
            {
                static TimerManager instance;
                return instance;
            }

            TimerManager(const TimerManager&) = delete;
            TimerManager& operator=(const TimerManager&) = delete;

            template <typename Rep, typename Period>
            TimerWheel::TimerId add_timer(std::chrono::duration<Rep, Period> timeout, TimerWheel::Callback callback)
            {
                return timer_wheel_.add_timer(timeout, std::move(callback));
            }

            void remove_timer(TimerWheel::TimerId id)
            {
                timer_wheel_.remove_timer(id);
            }

        private:
            TimerManager() : worker_([this](std::stop_token st) { timer_loop(st); })
            {
            }

            ~TimerManager()
            {
                worker_.request_stop();
                cv_.notify_one();
            }

            void timer_loop(std::stop_token stoken)
            {
                while (!stoken.stop_requested())
                {
                    timer_wheel_.tick();
                    std::unique_lock lock(mutex_);
                    cv_.wait_for(lock, TIMER_RESOLUTION, [&stoken] { return stoken.stop_requested(); });
                }
            }

            static constexpr std::chrono::milliseconds TIMER_RESOLUTION{10};
            TimerWheel timer_wheel_;
            std::jthread worker_;
            std::mutex mutex_;
            std::condition_variable cv_;
        };
    } // namespace detail


    //==================================================================================================
    // 5. 核心Task实现
    //==================================================================================================

    namespace detail
    {
        template <typename T>
        struct Promise;

        struct FinalAwaiter
        {
            bool await_ready() const noexcept { return false; }

            template <typename PromiseType>
            std::coroutine_handle<> await_suspend(std::coroutine_handle<PromiseType> h) noexcept
            {
                auto& promise = h.promise();
                if (promise.executor_ && promise.continuation_)
                {
                    promise.executor_->execute(promise.continuation_);
                    return std::noop_coroutine();
                }
                return promise.continuation_ ? promise.continuation_ : std::noop_coroutine();
            }

            void await_resume() noexcept
            {
            }
        };

        struct PromiseBase
        {
            std::coroutine_handle<> continuation_ = nullptr;
            std::exception_ptr exception_ = nullptr;
            CancellationToken cancellation_token_;
            Executor* executor_ = nullptr;

#if defined(TINAKIT_ENABLE_MEMORY_POOL)
        static void* operator new(std::size_t size) {
            return detail::CoroutineMemoryPoolManager::get_instance().get_resource()->allocate(size);
        }
        static void operator delete(void* ptr, std::size_t size) noexcept {
            detail::CoroutineMemoryPoolManager::get_instance().get_resource()->deallocate(ptr, size);
        }
#endif

            std::suspend_always initial_suspend() const noexcept { return {}; }
            FinalAwaiter final_suspend() const noexcept { return {}; }
            void unhandled_exception() noexcept { exception_ = std::current_exception(); }
            void set_cancellation_token(CancellationToken token) { cancellation_token_ = std::move(token); }
            const CancellationToken& get_cancellation_token() const { return cancellation_token_; }
        };

        template <typename T>
        struct Promise final : PromiseBase
        {
            std::optional<T> result_;
            Task<T> get_return_object();

            template <typename U>
            void return_value(U&& value) noexcept(std::is_nothrow_constructible_v<T, U&&>)
            {
                result_.emplace(std::forward<U>(value));
            }

            T& result() &
            {
                if (exception_) std::rethrow_exception(exception_);
                return *result_;
            }

            T&& result() &&
            {
                if (exception_) std::rethrow_exception(exception_);
                return std::move(*result_);
            }
        };

        template <>
        struct Promise<void> final : PromiseBase
        {
            Task<void> get_return_object();

            void return_void() noexcept
            {
            }

            void result() { if (exception_) std::rethrow_exception(exception_); }
        };
    }

    template <typename T = void>
    class Task
    {
    public:
        using promise_type = detail::Promise<T>;
        friend struct detail::Promise<T>;
        friend struct detail::Promise<void>;

        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        Task(Task&& other) noexcept : handle_(std::exchange(other.handle_, nullptr))
        {
        }

        Task& operator=(Task&& other) noexcept
        {
            if (this != &other)
            {
                if (handle_) handle_.destroy();
                handle_ = std::exchange(other.handle_, nullptr);
            }
            return *this;
        }

        ~Task()
        {
            if (handle_) handle_.destroy();
        }

        auto operator co_await() & noexcept;
        auto operator co_await() && noexcept;

        Task<T> with_cancellation(CancellationToken token) &&
        {
            handle_.promise().set_cancellation_token(std::move(token));
            return std::move(*this);
        }

        Task<T> via(Executor& executor) &&
        {
            handle_.promise().executor_ = &executor;
            return std::move(*this);
        }

        template <typename Rep, typename Period>
        Task<T> with_timeout(std::chrono::duration<Rep, Period> timeout) &&
        {
            return tinakit::async::with_timeout(std::move(*this), timeout);
        }

    private:
        explicit Task(std::coroutine_handle<promise_type> handle) noexcept : handle_(handle)
        {
        }

        std::coroutine_handle<promise_type> handle_;
    };

    // Task Awaiter 实现 - 移到类外部以支持模板成员函数
    namespace detail {
        template<typename T>
        struct TaskAwaiter {
            std::coroutine_handle<typename Task<T>::promise_type> handle_;

            bool await_ready() const noexcept {
                return !handle_ || handle_.done();
            }

            template<typename AwaitingPromise>
            std::coroutine_handle<> await_suspend(std::coroutine_handle<AwaitingPromise> awaiting_coro) noexcept {
                handle_.promise().continuation_ = awaiting_coro;
                if constexpr (requires { awaiting_coro.promise().get_cancellation_token(); }) {
                    if (!handle_.promise().get_cancellation_token().is_cancellation_requested()) {
                       handle_.promise().set_cancellation_token(awaiting_coro.promise().get_cancellation_token());
                    }
                }
                return handle_;
            }

            std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaiting_coro) noexcept {
                handle_.promise().continuation_ = awaiting_coro;
                return handle_;
            }

            auto await_resume() {
                if constexpr (std::is_void_v<T>) {
                    handle_.promise().result();
                } else {
                    return std::move(handle_.promise()).result();
                }
            }
        };

        template <typename T>
        Task<T> Promise<T>::get_return_object()
        {
            return Task<T>{std::coroutine_handle<Promise<T>>::from_promise(*this)};
        }

        inline Task<void> Promise<void>::get_return_object()
        {
            return Task<void>{std::coroutine_handle<Promise<void>>::from_promise(*this)};
        }
    }

    template<typename T>
    auto Task<T>::operator co_await() & noexcept {
        return detail::TaskAwaiter<T>{handle_};
    }

    template<typename T>
    auto Task<T>::operator co_await() && noexcept {
        return detail::TaskAwaiter<T>{handle_};
    }


    /**
     * @brief 为一个任务附加超时功能。
     */
    template <typename T, typename Rep, typename Period>
    Task<T> with_timeout(Task<T>&& task, std::chrono::duration<Rep, Period> timeout)
    {
        auto cts_ptr = std::make_shared<CancellationTokenSource>();

        auto timer_id = detail::TimerManager::get_instance().add_timer(timeout, [cts_ptr]
        {
            cts_ptr->cancel();
        });

        try
        {
            if constexpr (std::is_void_v<T>)
            {
                co_await std::move(task).with_cancellation(cts_ptr->token());
                detail::TimerManager::get_instance().remove_timer(timer_id);
            }
            else
            {
                T result = co_await std::move(task).with_cancellation(cts_ptr->token());
                detail::TimerManager::get_instance().remove_timer(timer_id);
                co_return result;
            }
        }
        catch (...)
        {
            detail::TimerManager::get_instance().remove_timer(timer_id);
            throw;
        }
    }


    //==================================================================================================
    // 6. 并发组合器与工具函数
    //==================================================================================================
    namespace detail
    {
        struct void_result
        {
        };

        template <typename T>
        struct task_result_impl
        {
            using type = T;
        };

        template <>
        struct task_result_impl<void>
        {
            using type = void_result;
        };

        template <typename T>
        using task_result_t = typename task_result_impl<T>::type;
        template <typename T>
        struct task_value_type;

        template <typename T>
        struct task_value_type<Task<T>>
        {
            using type = T;
        };

        template <typename T>
        using task_value_t = typename task_value_type<T>::type;

        template <typename Tuple, std::size_t... Is>
        auto when_all_impl(Tuple&& tasks, std::index_sequence<Is...>) ->
            Task<std::tuple<task_result_t<task_value_t<std::decay_t<decltype(std::get<Is>(tasks))>>>...>>
        {
            co_return std::make_tuple((co_await std::get<Is>(std::forward<Tuple>(tasks)))...);
        }
    }

    template <typename... Tasks>
    auto when_all(Tasks&&... tasks)
    {
        return detail::when_all_impl(
            std::make_tuple(std::forward<Tasks>(tasks)...),
            std::index_sequence_for<Tasks...>{}
        );
    }

    // sync_wait 的 void 特化版本
    inline void sync_wait(Task<void>&& task)
    {
        std::binary_semaphore sem{0};

        auto final_task_runner = [](Task<void> t, auto& s) -> Task<void>
        {
            try
            {
                co_await std::move(t);
            }
            catch (...)
            {
                s.release();
                throw;
            }
            s.release();
        };

        // 从Task中提取协程句柄，但保持Task的生命周期
        auto task_with_signal = final_task_runner(std::move(task), sem);
        auto awaiter = task_with_signal.operator co_await();
        auto handle = awaiter.await_suspend(std::noop_coroutine());

        handle.resume();
        sem.acquire();

        awaiter.await_resume();
    }

    // sync_wait 的非 void 版本
    template <typename T>
    T sync_wait(Task<T>&& task)
    {
        static_assert(!std::is_void_v<T>, "Use sync_wait(Task<void>&&) for void tasks");

        std::binary_semaphore sem{0};

        auto final_task_runner = [](Task<T> t, auto& s) -> Task<std::optional<T>>
        {
            std::optional<T> result;
            try
            {
                result = co_await std::move(t);
            }
            catch (...)
            {
                s.release();
                throw;
            }
            s.release();
            co_return result;
        };

        // 从Task中提取协程句柄，但保持Task的生命周期
        auto task_with_signal = final_task_runner(std::move(task), sem);
        auto awaiter = task_with_signal.operator co_await();
        auto handle = awaiter.await_suspend(std::noop_coroutine());

        handle.resume();
        sem.acquire();

        return *awaiter.await_resume();
    }
} // namespace tinakit::async
