/*
 * @Author: modnarshen
 * @Date: 2023.01.06 11:57:13
 * @Note: Copyrights (c) 2022 modnarshen. All rights reserved.
 */
#ifndef _MT_CONCEPT_AWAITABLE_H
#define _MT_CONCEPT_AWAITABLE_H 1

/*
 * NOTE:
 *   Please read the article `C++ Coroutines: Understanding operator co_await` before reading the code.
 *   The article address: https://lewissbaker.github.io/2017/11/17/understanding-operator-co-await
 *
 * ABSTRACT:
 *   Awaitable: A type that supports the `co_await` operator is called an `Awaitable` type.
 *   Awaiter: An `Awaiter` type is a type that implements the three special methods that are called as part of a
 * `co_await` expression: `await_ready`, `await_suspend` and `await_resume`.
 */

#include <coroutine>
#include <utility>

namespace mt {
namespace detail {

template <typename _Tp>
struct Awaitable : std::type_identity<_Tp> { };

template <typename _Tp>
requires requires(_Tp&& _obj) { std::forward<_Tp>(_obj).operator co_await(); }
struct Awaitable<_Tp> : std::type_identity<decltype(std::declval<_Tp>().operator co_await())> { };

template <typename _Tp>
requires requires(_Tp&& _obj) {
             operator co_await(std::forward<_Tp>(_obj));
             requires !(requires { std::forward<_Tp>(_obj).operator co_await(); });
         }
struct Awaitable<_Tp> : std::type_identity<decltype(operator co_await(std::declval<_Tp>()))> { };

template <typename _Tp>
using Awaitable_t = typename Awaitable<_Tp>::type;

}  // namespace detail

namespace concepts {

template <typename _Tp>
concept Awaiter = requires {
                      typename detail::Awaitable_t<_Tp>;
                      requires requires(detail::Awaitable_t<_Tp> _awaiter, std::coroutine_handle<> _handle) {
                                   { _awaiter.await_ready() } -> std::convertible_to<bool>;
                                   _awaiter.await_suspend(_handle);
                                   _awaiter.await_resume();
                               };
                  };

}  // namespace concepts

template <concepts::Awaiter _Tp>
using AwaitResult = decltype(std::declval<detail::Awaitable_t<_Tp>>().await_resume());

// check std::suspend_xxx
static_assert(concepts::Awaiter<std::suspend_always>);
static_assert(concepts::Awaiter<std::suspend_never>);

}  // namespace mt

#endif  // _MT_CONCEPT_AWAITABLE_H
