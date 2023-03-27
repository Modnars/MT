/*
 * @Author: modnarshen
 * @Date: 2023.01.08 16:25:39
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_SCHEDULED_TASH_H
#define _MT_SCHEDULED_TASH_H 1

#include <mt/concept/schedulable.h>
#include <mt/util/noncopyable.h>

namespace mt {

template <concepts::SchedulableTask _Task>
struct ScheduledTask : private NonCopyable {
    template <concepts::SchedulableTask _SchedulableTask>
    explicit ScheduledTask(_SchedulableTask &&_task) : task_(std::forward<_SchedulableTask>(_task)) {
        if (task_.valid() && !task_.done()) {
            task_.schedule();
        }
    }

    void cancel() { task_.cancel(); }

    decltype(auto) operator co_await() const & noexcept { return task_.operator co_await(); }
    auto operator co_await() const && noexcept { return task_.operator co_await(); }

    decltype(auto) result() & { return task_.result(); }
    decltype(auto) result() && { return std::move(task_).result(); }

    bool valid() const { return task_.valid(); }
    bool done() const { return task_.done(); }

private:
    _Task task_;
};

template <concepts::SchedulableTask _Task>
ScheduledTask(_Task &&) -> ScheduledTask<_Task>;

template <concepts::SchedulableTask _Task>
[[nodiscard("discard a task will not schedule to run")]] ScheduledTask<_Task> scheduled_task(_Task &&_task) {
    return ScheduledTask{std::forward<_Task>(_task)};
}

}  // namespace mt

#endif  // _MT_SCHEDULED_TASH_H
