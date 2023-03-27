/*
 * @Author: modnarshen
 * @Date: 2023.01.08 16:45:10
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_RUNNER_H
#define _MT_RUNNER_H 1

#include <mt/concept/schedulable.h>
#include <mt/event_loop.h>
#include <mt/scheduled_task.h>

namespace mt {

template <concepts::SchedulableTask _Task>
decltype(auto) run(_Task &&task) {
    auto st = scheduled_task(std::forward<_Task>(task));
    get_event_loop().run_until_complete();
    if constexpr (std::is_lvalue_reference_v<_Task &&>) {
        return st.result();
    } else {
        return std::move(st).result();
    }
}

}  // namespace mt

#endif  // _MT_RUNNER_H
