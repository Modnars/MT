/*
 * @Author: modnarshen
 * @Date: 2023.01.05 17:58:48
 * @Note: Copyrights (c) 2022 modnarshen. All rights reserved.
 */
#include <chrono>
#include <optional>

#include <mt/event_loop.h>

namespace mt {

void EventLoop::run_until_complete() {
    while (!is_stop()) {
        run_once();
    }
}

void EventLoop::clean_delayed_call() {
    // Remove delayed calls that were cancelled from head of queue.
    while (!scheduled_.empty()) {
        auto&& [when, handle_info] = scheduled_[0];
        if (auto iter = cancelled_.find(handle_info.id); iter != cancelled_.end()) {
            std::ranges::pop_heap(scheduled_, std::ranges::greater{}, &TimerHandle::first);
            scheduled_.pop_back();
            cancelled_.erase(iter);
        } else {
            break;
        }
    }
}

void EventLoop::run_once() {
    std::optional<duration_type> time_out;
    if (!ready_.empty()) {
        time_out.emplace(0);
    } else if (!scheduled_.empty()) {
        auto&& [when, _] = scheduled_[0];
        time_out = std::max(when - time(), duration_type(0));
    }

    auto event_lists = selector_.select(time_out.has_value() ? time_out->count() : -1);
    for (auto&& event : event_lists)
        ready_.push(event.handle_info);

    auto end_time = time();
    while (!scheduled_.empty()) {
        auto&& [when, handle_info] = scheduled_[0];
        COND_EXP(when >= end_time, break);
        ready_.push(handle_info);
        std::ranges::pop_heap(scheduled_, std::ranges::greater{}, &TimerHandle::first);
        scheduled_.pop_back();
    }

    for (std::size_t ntodo = ready_.size(), i = 0; i < ntodo; ++i) {
        auto [handle_id, handle] = ready_.front();
        ready_.pop();
        if (auto iter = cancelled_.find(handle_id); iter != cancelled_.end()) {
            cancelled_.erase(iter);
        } else {
            handle->set_state(Handle::STATE::UNSCHEDULED);
            handle->run();
        }
    }

    clean_delayed_call();
}

HandleId Handle::handle_id_generator_ = 0;

const std::source_location& CoroHandle::get_frame_info() const {
    static const std::source_location frame_info = std::source_location::current();
    return frame_info;
}

void CoroHandle::schedule() {
    if (state_ == Handle::STATE::UNSCHEDULED) {
        get_event_loop().call_soon(*this);
    }
}

void CoroHandle::cancel() {
    get_event_loop().cancel_handle(*this);
}

EventLoop& get_event_loop() {
    static EventLoop event_loop;
    return event_loop;
}

}  // namespace mt
