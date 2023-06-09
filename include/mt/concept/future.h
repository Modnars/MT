/*
 * @Author: modnarshen
 * @Date: 2023.01.06 10:51:03
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_CONCEPT_FUTURE_H
#define _MT_CONCEPT_FUTURE_H 1

#include <concepts>

#include <mt/concept/awaitable.h>

namespace mt {
namespace concepts {

template <typename _Future>
concept Future = Awaiter<_Future> && requires(_Future _future) {
    requires !std::default_initializable<_Future>;
    requires std::move_constructible<_Future>;
    typename std::remove_cvref_t<_Future>::promise_type;
    _future.result();
};

}  // namespace concepts
}  // namespace mt

#endif  // _MT_CONCEPT_FUTURE_H
