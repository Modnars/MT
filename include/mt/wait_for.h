/*
 * @Author: modnarshen
 * @Date: 2023.01.11 16:04:00
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_WAIT_FOR_H
#define _MT_WAIT_FOR_H 1

#include <chrono>

#include <mt/concept/awaitable.h>
#include <mt/concept/future.h>
#include <mt/event_loop.h>
#include <mt/exception.h>
#include <mt/handle.h>
#include <mt/result.h>
#include <mt/scheduled_task.h>
#include <mt/task.h>
#include <mt/util/noncopyable.h>

namespace mt {
namespace detail {

template <typename _Res, typename _Duration>
struct WaitForAwaiter : NonCopyable {
private:
    struct TimeOutHandle : Handle {
        TimeOutHandle(WaitForAwaiter &awaiter, _Duration time_out) : awaiter_(awaiter) {
            get_event_loop().call_later(time_out, *this);
        }

        void run() final {  // on time out!
            awaiter_.wait_for_task_.cancel();
            awaiter_.result_.set_exception(std::make_exception_ptr(ExceptionTimeOut{}));
            get_event_loop().call_soon(*awaiter_.continuation_);
        }

        WaitForAwaiter &awaiter_;
    };

public:
    constexpr bool await_ready() noexcept { return result_.has_value(); }
    constexpr decltype(auto) await_resume() { return std::move(result_).result(); }

    template <typename _Promise>
    void await_suspend(std::coroutine_handle<_Promise> continuation) noexcept {
        continuation_ = &continuation.promise();
        // set continuation_ to SUSPEND, don't schedule anymore, until it resume continuation_
        continuation_->set_state(Handle::STATE::SUSPENDED);
    }

    template <concepts::Awaiter _Future>
    WaitForAwaiter(_Future &&future, _Duration time_out)
        : time_out_handle_(*this, time_out),
          wait_for_task_(scheduled_task(wait_for_task(no_wait_at_initial_suspend, std::forward<_Future>(future)))) { }

private:
    template <concepts::Awaiter _Future>
    Task<> wait_for_task(NoWaitAtInitialSuspend, _Future &&future) {
        try {
            if constexpr (std::is_void_v<_Res>) {
                co_await std::forward<_Future>(future);
            } else {
                result_.set_value(co_await std::forward<_Future>(future));
            }
        } catch (...) {
            result_.unhandled_exception();
        }
        EventLoop &loop{get_event_loop()};
        loop.cancel_handle(time_out_handle_);
        COND_EXP(continuation_, loop.call_soon(*continuation_));
    }

private:
    Result<_Res> result_;
    CoroHandle *continuation_{};
    TimeOutHandle time_out_handle_;
    ScheduledTask<Task<>> wait_for_task_;
};

template <concepts::Awaiter _Future, typename _Duration>
WaitForAwaiter(_Future &&, _Duration) -> WaitForAwaiter<AwaitResult<_Future>, _Duration>;

template <concepts::Awaiter _Future, typename _Duration>
struct WaitForAwaiterRegistry {
public:
    WaitForAwaiterRegistry(_Future &&future, _Duration duration)
        : future_(std::forward<_Future>(future)), duration_(duration) { }

    auto operator co_await() && { return WaitForAwaiter{std::forward<_Future>(future_), duration_}; }

private:
    _Future future_;  // lift awaiter's lifetime
    _Duration duration_;
};

template <concepts::Awaiter _Future, typename _Duration>
WaitForAwaiterRegistry(_Future &&, _Duration) -> WaitForAwaiterRegistry<_Future, _Duration>;

template <concepts::Awaiter _Future, typename _Rep, typename _Period>
auto wait_for(NoWaitAtInitialSuspend, _Future &&future, std::chrono::duration<_Rep, _Period> time_out)
    -> Task<AwaitResult<_Future>> {  // lift awaiter type(WaitForAwaiterRegistry) to coroutine
    co_return co_await WaitForAwaiterRegistry{std::forward<_Future>(future), time_out};
}

}  // namespace detail

template <concepts::Awaiter _Future, typename _Rep, typename _Period>
[[nodiscard("discard wait_for doesn't make sense")]] Task<AwaitResult<_Future>> wait_for(
    _Future &&future, std::chrono::duration<_Rep, _Period> time_out) {
    return detail::wait_for(no_wait_at_initial_suspend, std::forward<_Future>(future), time_out);
}

}  // namespace mt

#endif  // _MT_WAIT_FOR_H
