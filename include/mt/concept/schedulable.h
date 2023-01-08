/*
 * @Author: modnarshen
 * @Date: 2023.01.08 17:33:56
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_CONCEPT_SCHEDULABLE_H
#define _MT_CONCEPT_SCHEDULABLE_H 1

#include <mt/concept/future.h>

namespace mt {
namespace concepts {

template <typename _Task>
concept Schedulable = requires(_Task _task) {
                          _task.valid();
                          _task.done();
                          _task.schedule();
                          _task.cancel();
                      };

template <typename _Task>
concept SchedulableTask = concepts::Future<_Task> && concepts::Schedulable<_Task>;

}  // namespace concepts
}  // namespace mt

#endif  // _MT_CONCEPT_SCHEDULABLE_H
