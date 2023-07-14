/*
 * @Author: modnarshen
 * @Date: 2023.06.28 15:04:29
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#pragma once  // 所有 rpc/demo 目录下的代码视为业务代码，采用 pragma 来避免重复 include

#include <llbc.h>

#define COND_RET_TLOG(condition, retCode, ...) \
    {                                          \
        if (condition) {                       \
            LLOG_TRACE(__VA_ARGS__);           \
            return retCode;                    \
        }                                      \
    }

#define COND_RET_ILOG(condition, retCode, ...) \
    {                                          \
        if (condition) {                       \
            LLOG_INFO(__VA_ARGS__);            \
            return retCode;                    \
        }                                      \
    }

#define COND_RET_WLOG(condition, retCode, ...) \
    {                                          \
        if (condition) {                       \
            LLOG_WARN(__VA_ARGS__);            \
            return retCode;                    \
        }                                      \
    }

#define COND_RET_ELOG(condition, retCode, ...) \
    {                                          \
        if (condition) {                       \
            LLOG_ERROR(__VA_ARGS__);           \
            return retCode;                    \
        }                                      \
    }

#define COND_EXP_ELOG(condition, expr, ...) \
    {                                       \
        if (condition) [[unlikely]] {       \
            LLOG_ERROR(__VA_ARGS__);        \
            expr;                           \
        }                                   \
    }

#define CO_COND_RET_TLOG(condition, retCode, ...) \
    {                                             \
        if (condition) {                          \
            LLOG_TRACE(__VA_ARGS__);              \
            co_return retCode;                    \
        }                                         \
    }

#define CO_COND_RET_ILOG(condition, retCode, ...) \
    {                                             \
        if (condition) {                          \
            LLOG_INFO(__VA_ARGS__);               \
            co_return retCode;                    \
        }                                         \
    }

#define CO_COND_RET_WLOG(condition, retCode, ...) \
    {                                             \
        if (condition) {                          \
            LLOG_WARN(__VA_ARGS__);               \
            co_return retCode;                    \
        }                                         \
    }

#define CO_COND_RET_ELOG(condition, retCode, ...) \
    {                                             \
        if (condition) {                          \
            LLOG_ERROR(__VA_ARGS__);              \
            co_return retCode;                    \
        }                                         \
    }
