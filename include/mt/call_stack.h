/*
 * @Author: modnarshen
 * @Date: 2023.01.09 20:54:33
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_CALL_STACK_H
#define _MT_CALL_STACK_H 1

#include <coroutine>

namespace mt {
namespace detail {

struct CallStackAwaiter {
    constexpr bool await_ready() noexcept { return false; }
    constexpr void await_resume() const noexcept { }

    template <typename _Promise>
    bool await_suspend(std::coroutine_handle<_Promise> caller) const noexcept {
        caller.promise().dump_backtrace();
        return false;
    }
};

}  // namespace detail

[[nodiscard]] detail::CallStackAwaiter dump_call_stack() {
    return {};
}

}  // namespace mt

#endif  // _MT_CALL_STACK_H
