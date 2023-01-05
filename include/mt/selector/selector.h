/*
 * @Author: modnarshen
 * @Date: 2023.01.05 15:57:48
 * @Note: Copyrights (c) 2022 modnarshen. All rights reserved.
 */
#ifndef _MT_SELECTOR_SELECTOR_H
#define _MT_SELECTOR_SELECTOR_H 1

#if defined(__APPLE__)

#include <mt/selector/selector_kqueue.h>

namespace mt {

using Selector = SelectorKQueue;

}  // namespace mt

#elif defined(__linux)

#include <mt/selector/selector_epoll.h>

namespace mt {

using Selector = SelectorEpoll;

}  // namespace mt

#endif

#endif  // _MT_SELECTOR_SELECTOR_H
