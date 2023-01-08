/*
 * @Author: modnarshen
 * @Date: 2023.01.06 11:12:20
 * @Note: Copyrights (c) 2022 modnarshen. All rights reserved.
 */
#ifndef _MT_CONCEPT_PROMISE_H
#define _MT_CONCEPT_PROMISE_H 1

#include <mt/concept/awaitable.h>
#include <mt/concept/future.h>

namespace mt {
namespace concepts {

template <typename _Promise>
concept Promise = requires(_Promise _promise) {
                      { _promise.get_return_object() } -> Future;
                      { _promise.initial_suspend() } -> Awaiter;
                      { _promise.final_suspend() } noexcept -> Awaiter;
                      _promise.unhandled_exception();
                      requires(
                          requires(int v) { _promise.return_value(v); } || requires { _promise.return_void(); });
                  };

}  // namespace concepts
}  // namespace mt

#endif  // _MT_CONCEPT_PROMISE_H
