/*
 * @Author: modnarshen
 * @Date: 2023.06.28 15:04:29
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _RPC_DEMO_SRC_MACROS_H
#define _RPC_DEMO_SRC_MACROS_H 1

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

#endif  // _RPC_DEMO_SRC_MACROS_H
