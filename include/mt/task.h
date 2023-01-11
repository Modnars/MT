/*
 * @Author: modnarshen
 * @Date: 2023.01.06 15:05:39
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_TASK_H
#define _MT_TASK_H 1

#include <cassert>
#include <coroutine>
#include <memory>
#include <variant>

#include <mt/concept/promise.h>
#include <mt/event_loop.h>
#include <mt/handle.h>
#include <mt/result.h>

namespace mt {

struct NoWaitAtInitialSuspend { };
inline constexpr NoWaitAtInitialSuspend no_wait_at_initial_suspend;

template <typename _Tp = void>
struct Task : private NonCopyable {
public:
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    struct AwaiterBase {
    public:
        constexpr bool await_ready() {
            if (self_coro_) [[likely]] {
                return self_coro_.done();
            }
            return true;
        }

        template <typename _Promise>
        void await_suspend(std::coroutine_handle<_Promise> resumer) const noexcept {
            assert(!self_coro_.promise().continuation_);
            resumer.promise().set_state(Handle::STATE::SUSPENDED);
            self_coro_.promise().continuation_ = &resumer.promise();
            self_coro_.promise().schedule();
        }

        coro_handle self_coro_{};
    };

    struct InitialSuspendAwaiter {
        constexpr bool await_ready() const noexcept { return !wait_at_initial_suspend_; }
        constexpr void await_suspend(std::coroutine_handle<>) const noexcept { }
        constexpr void await_resume() const noexcept { }
        const bool wait_at_initial_suspend_{true};
    };

    struct FinalSuspendAwaiter {
        constexpr bool await_ready() const noexcept { return false; }
        template <typename _Promise>
        constexpr void await_suspend(std::coroutine_handle<_Promise> h) const noexcept {
            if (auto continuation = h.promise().continuation_) {
                get_event_loop().call_soon(*continuation);
            }
        }
        constexpr void await_resume() const noexcept { }
    };

    struct promise_type : CoroHandle, Result<_Tp> {
    public:
        promise_type() = default;

        template <typename... _Args>  // from free function
        promise_type(NoWaitAtInitialSuspend, _Args &&...) : wait_at_initial_suspend_{false} { }
        template <typename _Obj, typename... _Args>  // from member function
        promise_type(_Obj &&, NoWaitAtInitialSuspend, _Args &&...) : wait_at_initial_suspend_{false} { }

        auto initial_suspend() noexcept { return InitialSuspendAwaiter{wait_at_initial_suspend_}; }

        auto final_suspend() noexcept { return FinalSuspendAwaiter{}; }

        Task get_return_object() noexcept { return Task{coro_handle::from_promise(*this)}; }

        template <concepts::Awaiter _Awaiter>
        decltype(auto) await_transform(_Awaiter &&awaiter, std::source_location loc = std::source_location::current()) {
            frame_info_ = loc;  // save soure_location info
            return std::forward<_Awaiter>(awaiter);
        }

        void run() final { coro_handle::from_promise(*this).resume(); }
        const std::source_location &get_frame_info() const final override { return frame_info_; }
        void dump_backtrace(std::size_t depth = 0) const final override {
            fmt::print("[{}] {}\n", depth, frame_name());
            if (continuation_) {
                continuation_->dump_backtrace(depth + 1);
            } else {
                fmt::print("\n");
            }
        }

    public:
        const bool wait_at_initial_suspend_{true};
        CoroHandle *continuation_{};
        std::source_location frame_info_{};
    };

public:
    explicit Task(coro_handle h) noexcept : handle_(h) { }
    Task(Task &&t) noexcept : handle_(std::exchange(t.handle_, {})) { }
    ~Task() { destroy(); }

    decltype(auto) result() & { return handle_.promise().result(); }

    decltype(auto) result() && { return std::move(handle_.promise()).result(); }

    auto operator co_await() const &noexcept {
        struct AwaiterImpl : AwaiterBase {
            decltype(auto) await_resume() const {
                if (!AwaiterBase::self_coro_) [[unlikely]] {
                    throw ExceptionInvalidFuture{};
                }
                return AwaiterBase::self_coro_.promise().result();
            }
        };
        return AwaiterImpl{handle_};
    }

    auto operator co_await() const &&noexcept {
        struct AwaiterImpl : AwaiterBase {
            decltype(auto) await_resume() const {
                if (!AwaiterBase::self_coro_) [[unlikely]] {
                    throw ExceptionInvalidFuture{};
                }
                return std::move(AwaiterBase::self_coro_.promise()).result();
            }
        };
        return AwaiterImpl{handle_};
    }

    bool valid() const { return handle_ != nullptr; }
    bool done() const { return handle_.done(); }
    void schedule() { handle_.promise().schedule(); }
    void cancel() { destroy(); }

private:
    void destroy() {
        if (auto handle = std::exchange(handle_, nullptr)) {
            handle.promise().cancel();
            handle.destroy();
        }
    }

private:
    coro_handle handle_;
};

static_assert(concepts::Promise<Task<>::promise_type>);
static_assert(concepts::Future<Task<>>);

}  // namespace mt

#endif  // _MT_TASK_H
