/*
 * @Author: modnarshen
 * @Date: 2023.01.06 15:11:18
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_EXCEPTION_H
#define _MT_EXCEPTION_H 1

#include <exception>

namespace mt {

struct ExceptionTimeOut : std::exception {
    [[nodiscard]] const char *what() const noexcept override { return "[EXCEPTION] TIME_OUT"; }
};

struct ExceptionNoResult : std::exception {
    [[nodiscard]] const char *what() const noexcept override { return "[EXCEPTION] NO_RESULT"; }
};

struct ExceptionInvalidFuture : std::exception {
    [[nodiscard]] const char *what() const noexcept override { return "[EXCEPTION] INVALID_FUTURE"; }
};

}  // namespace mt

#endif  // _MT_EXCEPTION_H
