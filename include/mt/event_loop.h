/*
 * @Author: modnarshen
 * @Date: 2023.01.05 15:20:06
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_EVENT_LOOP_H
#define _MT_EVENT_LOOP_H 1

#include <algorithm>
#include <chrono>
#include <coroutine>
#include <queue>
#include <unordered_set>
#include <utility>
#include <vector>

#include <mt/handle.h>
#include <mt/selector/selector.h>
#include <mt/util/noncopyable.h>

namespace mt {

class EventLoop : private NonCopyable {
public:
    using duration_type = std::chrono::milliseconds;
    using clock_type = std::chrono::steady_clock;
    using time_point_type = std::chrono::time_point<clock_type>;

    struct WaitEventAwaiter {
    public:
        WaitEventAwaiter(Selector &selector, Event event) : selector_(selector), event_(event) { }
        ~WaitEventAwaiter() { selector_.remove_event(event_); }

    public:
        constexpr bool await_ready() const noexcept { return false; }

        template <typename _Promise>
        constexpr void await_suspend(std::coroutine_handle<_Promise> handle) noexcept {
            handle.promise().set_state(Handle::STATE::SUSPENDED);
            event_.handle_info = {.id = handle.promise().handle_id(), .handle = &handle.promise()};
            selector_.register_event(event_);
        }

        void await_resume() noexcept { }

    private:
        Selector &selector_;
        Event event_;
    };

public:
    EventLoop() : start_time_(clock_type::now()) { }

    duration_type time() { return std::chrono::duration_cast<duration_type>(clock_type::now() - start_time_); }

    void cancel_handle(Handle &handle) {
        handle.set_state(Handle::STATE::UNSCHEDULED);
        cancelled_.insert(handle.handle_id());
    }

    void call_soon(Handle &handle) {
        handle.set_state(Handle::STATE::SCHEDULED);
        ready_.push({handle.handle_id(), &handle});
    }

    template <typename _Rep, typename _Period>
    void call_later(std::chrono::duration<_Rep, _Period> delay, Handle &call_back) {
        call_at(time() + std::chrono::duration_cast<duration_type>(delay), call_back);
    }

    [[nodiscard]] auto wait_event(const Event &event) { return WaitEventAwaiter{selector_, event}; }

    void run_until_complete();

private:
    bool is_stop() { return scheduled_.empty() && ready_.empty() && selector_.is_stop(); }

    void clean_delayed_call();

    template <typename _Rep, typename _Period>
    void call_at(std::chrono::duration<_Rep, _Period> when, Handle &call_back) {
        call_back.set_state(Handle::STATE::SCHEDULED);
        scheduled_.emplace_back(std::chrono::duration_cast<duration_type>(when),
                                HandleInfo{call_back.handle_id(), &call_back});
        std::ranges::push_heap(scheduled_, std::ranges::greater{}, &TimerHandle::first);
    }

    void run_once();

private:
    using TimerHandle = std::pair<duration_type, HandleInfo>;
    time_point_type start_time_;
    Selector selector_;
    std::queue<HandleInfo> ready_;
    std::vector<TimerHandle> scheduled_;  // min time heap
    std::unordered_set<HandleId> cancelled_;
};  // namespace mt

EventLoop &get_event_loop();

}  // namespace mt

#endif
