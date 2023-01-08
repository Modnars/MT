/*
 * @Author: modnarshen
 * @Date: 2023.01.05 16:50:43
 * @Note: Copyrights (c) 2022 modnarshen. All rights reserved.
 */
#ifndef _MT_UTIL_MACROS_H
#define _MT_UTIL_MACROS_H 1

#define COND_RET(condition, ...) \
    {                            \
        if (condition) {         \
            return __VA_ARGS__;  \
        }                        \
    }

#define COND_LIKELY_RET(condition, ...) \
    {                                   \
        if (condition) [[likely]] {     \
            return __VA_ARGS__;         \
        }                               \
    }

#define COND_EXP(condition, expr, ...) \
    {                                  \
        if (condition) {               \
            expr;                      \
        }                              \
    }

#define COND_LIKELY_EXP(condition, expr, ...) \
    {                                         \
        if (condition) [[likely]] {           \
            expr;                             \
        }                                     \
    }

#endif  // _MT_UTIL_MACROS_H
