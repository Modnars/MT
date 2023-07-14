/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-20 20:13:23
 * @edit: regangcli
 * @brief:
 */

#pragma once

#include <unordered_map>
#include "llbc/core/log/LoggerMgr.h"

#include <llbc.h>
#include <mt/handle.h>
#include <mt/task.h>
#include <mt/util/singleton.h>

class RpcCoroMgr : public mt::Singleton<RpcCoroMgr> {
public:
    using coro_uid_type = std::uint64_t;

public:
    int Init() { return 0; }

    bool UseCoro() const { return use_coro_; }
    void UseCoro(bool use_coro) { use_coro_ = use_coro; }

    coro_uid_type NewCoroUid() {
        COND_RET(!use_coro_, 0UL);
        return ++coro_uid_generator_ == 0UL ? ++coro_uid_generator_ : coro_uid_generator_;
    }
    bool Suspend(coro_uid_type coro_uid, mt::Task<> &&task) {
        return suspended_tasks_.insert({coro_uid, std::move(task)}).second;
    }

    mt::Task<> Pop(coro_uid_type coro_uid) {
        if (auto iter = suspended_tasks_.find(coro_uid); iter != suspended_tasks_.end()) {
            auto tt = std::move(iter->second);
            suspended_tasks_.erase(iter);
            return tt;
        }
        LLOG_ERROR("coro_uid not found|coro_uid:%lu", coro_uid);
        return noop();
    }

private:
    mt::Task<> noop() const { co_return; }

private:
    // map<coro_uid, task>
    std::unordered_map<coro_uid_type, mt::Task<>> suspended_tasks_;
    // coro_uid generator, which could generate unique id without `0`.
    static coro_uid_type coro_uid_generator_;
    bool use_coro_ = false;
};
