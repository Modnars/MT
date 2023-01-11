/*
 * @Author: modnarshen
 * @Date: 2023.01.10 16:51:45
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_VOID_VALUE_H
#define _MT_VOID_VALUE_H 1

#include <type_traits>

namespace mt {

struct VoidValue { };

namespace detail {

template <typename _Tp>
struct GetTypeIfVoid : std::type_identity<T> { };

template <>
struct GetTypeIfVoid<void> : std::type_identity<VoidValue> { };

}  // namespace detail

template <typename _Tp>
using GetTypeIfVoid_t = typename detail::GetTypeIfVoid<_Tp>::type;

}  // namespace mt

#endif  // _MT_VOID_VALUE_H
