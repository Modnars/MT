/*
 * @Author: modnarshen
 * @Date: 2023.01.11 14:48:35
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_SLEEP_H
#define _MT_SLEEP_H 1

#include <chrono>

#include <mt/task.h>
#include <mt/util/noncopyable.h>

namespace mt {
namespace detail {

template <typename _Duration>
struct SleepAwaiter : private NonCopyable {
public:
    explicit SleepAwaiter(_Duration delay) : delay_(delay) { }
    constexpr bool await_ready() noexcept { return false; }
    constexpr void await_resume() const noexcept { }

    template <typename _Promise>
    void await_suspend(std::coroutine_handle<_Promise> caller) const noexcept {
        get_event_loop().call_later(delay_, caller.promise());
    }

private:
    _Duration delay_;
};

template <typename _Rep, typename _Period>
Task<> sleep(NoWaitAtInitialSuspend, std::chrono::duration<_Rep, _Period> delay) {
    co_await detail::SleepAwaiter{delay};
}

}  // namespace detail

template <typename _Rep, typename _Period>
[[nodiscard("discard sleep doesn't make sense")]] Task<> sleep(std::chrono::duration<_Rep, _Period> delay) {
    return detail::sleep(no_wait_at_initial_suspend, delay);
}

}  // namespace mt

using namespace std::chrono_literals;  // Introduce literals.

#endif  // _MT_SLEEP_H
