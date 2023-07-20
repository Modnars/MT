/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-20 20:13:23
 * @edit: regangcli
 * @brief:
 */

#pragma once

#include <coroutine>
#include <unordered_map>

#include <google/protobuf/message.h>
#include <llbc.h>
#include <mt/handle.h>
#include <mt/task.h>
#include <mt/util/singleton.h>

class RpcCoroMgr : public mt::Singleton<RpcCoroMgr> {
public:
    using coro_uid_type = std::uint64_t;
    struct context {
        int session_id = 0;
        std::coroutine_handle<> handle = nullptr;
        ::google::protobuf::Message *rsp = nullptr;
    };

public:
    int Init() { return 0; }

    bool UseCoro() const { return use_coro_; }
    void UseCoro(bool use_coro) { use_coro_ = use_coro; }

    coro_uid_type NewCoroUid() {
        COND_RET(!use_coro_, 0UL);
        return ++coro_uid_generator_ == 0UL ? ++coro_uid_generator_ : coro_uid_generator_;
    }
    bool Suspend(coro_uid_type coro_uid, context ctx) { return suspended_contexts_.insert({coro_uid, ctx}).second; }

    context Pop(coro_uid_type coro_uid) {
        if (auto iter = suspended_contexts_.find(coro_uid); iter != suspended_contexts_.end()) {
            auto ctx = iter->second;
            suspended_contexts_.erase(iter);
            return ctx;
        }
        LLOG_ERROR("coro_uid not found|coro_uid:%lu", coro_uid);
        return context{.handle = {}, .rsp = nullptr};
    }

    std::coroutine_handle<> MainHandle() const { return main_handle_; }
    void SetMainHandle(std::coroutine_handle<> handle) { main_handle_ = handle; }

private:
    std::unordered_map<coro_uid_type, context> suspended_contexts_;
    // coro_uid generator, which could generate unique id without `0`.
    static coro_uid_type coro_uid_generator_;
    bool use_coro_ = false;
    std::coroutine_handle<> main_handle_ = nullptr;
};

struct MainCoroAwaiter {
public:
    MainCoroAwaiter(bool is_main) : is_main_(is_main) { }
    // MainCoroAwaiter(RpcCoroMgr::coro_uid_type coro_uid, ::google::protobuf::Message *rsp)
    //     : coro_uid_(coro_uid), rsp_(rsp) { }
    MainCoroAwaiter(RpcCoroMgr::coro_uid_type coro_uid, RpcCoroMgr::context context)
        : coro_uid_(coro_uid), context_(context) { }

    bool await_ready() const noexcept { return false; }

    decltype(auto) await_suspend(std::coroutine_handle<> handle) {
        if (is_main_) {
            LLOG_INFO("set main handle|%p", handle.address());
            RpcCoroMgr::GetInst().SetMainHandle(handle);
            return handle;
        }
        LLOG_INFO("suspend coro|coro_uid:%lu|%p|rsp:%p", coro_uid_, handle.address(), rsp_);
        context_.handle = handle;
        // RpcCoroMgr::GetInst().Suspend(coro_uid_, {.handle = handle, .rsp = rsp_});
        RpcCoroMgr::GetInst().Suspend(coro_uid_, context_);
        return RpcCoroMgr::GetInst().MainHandle();
    }

    void await_resume() { }

private:
    bool is_main_ = false;
    RpcCoroMgr::coro_uid_type coro_uid_ = 0UL;
    RpcCoroMgr::context context_;
    ::google::protobuf::Message *rsp_ = nullptr;
};
