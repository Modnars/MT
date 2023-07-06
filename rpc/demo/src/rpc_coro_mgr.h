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

#include <llbc.h>
#include <mt/handle.h>
#include <mt/task.h>
#include <mt/util/singleton.h>

class RpcCoroMgr : public mt::Singleton<RpcCoroMgr> {
public:
    using coro_uid_type = std::uint64_t;

public:
    int Init() { return 0; }

    coro_uid_type NewCoroUid() { return ++coro_uid_generator_ == 0UL ? ++coro_uid_generator_ : coro_uid_generator_; }
    bool Suspend(coro_uid_type coro_uid, mt::Task<> &&task) {
        return suspended_tasks_.insert({coro_uid, std::move(task)}).second;
    }
    bool Suspend(mt::Task<> &&task) { return Suspend(NewCoroUid(), std::move(task)); }

    mt::Task<> Pop(coro_uid_type coro_uid) {
        auto noop = [coro_uid]() -> mt::Task<> {
            LLOG_ERROR("task not found|coro_uid:%lu", coro_uid);
            co_return;
        };
        if (auto iter = suspended_tasks_.find(coro_uid); iter != suspended_tasks_.end()) {
            auto tt = std::move(iter->second);
            suspended_tasks_.erase(iter);
            return tt;
        }
        return noop();
    }

private:
    // map<coro_uid, task>
    std::unordered_map<coro_uid_type, mt::Task<>> suspended_tasks_;
    // coro_uid generator, which could generate unique id without `0`.
    static coro_uid_type coro_uid_generator_;
};
