/*
 * @Author: modnarshen
 * @Date: 2023.01.04 10:28:36
 * @Note: Copyrights (c) 2022 modnarshen. All rights reserved.
 */
#ifndef _MT_UTIL_NONCOPYABLE_H
#define _MT_UTIL_NONCOPYABLE_H 1

namespace mt {

struct NonCopyable {
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

}  // namespace mt

#endif  // _MT_UTIL_NONCOPYABLE_H
