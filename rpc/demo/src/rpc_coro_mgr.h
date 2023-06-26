/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-20 20:13:23
 * @edit: regangcli
 * @brief:
 */

#pragma once

#include "llbc.h"
#include <mt/task.h>

extern std::map<uint64_t /* task_id_ */, mt::Task<> /* task_ */> id_to_task_map_;

class BaseRpcCoroMgr;
class Coro {
    LLBC_DISABLE_ASSIGNMENT(Coro);

#pragma region 构造 / 析构
public:
    Coro(const std::function<void(void *)> &entry, void *args, const llbc::LLBC_Variant &user_data);

    ~Coro();
#pragma endregion

#pragma region 操作方法
public:
    // 挂起 coro
    int Yield(const llbc::LLBC_TimeSpan &timeout = llbc::LLBC_TimeSpan::zero);
    // 恢复 coro
    int Resume(const llbc::LLBC_Variant &pass_data = llbc::LLBC_Variant::nil);
    // 判断是否为主协程
    bool IsMainCoro() const;

    // 取消 coro
    int Cancel();
#pragma endregion

#pragma region 属性访问
public:
    // 获取 coro ID
    llbc::uint64 GetId() const { return coroId_; }

    // 获取 coro entry
    const std::function<void(void *)> &GetEntry() const { return entry_; }
    // 获取 coro args
    void *GetArgs() const { return args_; }

    // 获取 coro param1
    int GetParam1() const { return param1; }
    void SetParam1(int param) { param1 = param; }
    // 获取 coro param2
    int GetParam2() const { return param2; }
    void SetParam2(int param) { param2 = param; }
    // 获取 coro ptrParam1
    void *GetPtrParam1() const { return ptrParam1; }
    void SetPtrParam1(void *param) { ptrParam1 = param; }

    // 获取 coro user_data
    std::string &GetUserData() { return userData_; }

private:
    BaseRpcCoroMgr *coroMgr{nullptr};             // coro component
    llbc::uint64 coroId_{0};                      // coro id
    std::function<void(void *)> entry_{nullptr};  // coro 逻辑执行 delegate
    void *args_{nullptr};                         // coro 逻辑执行 delegate 参数
    std::string &userData_;                       // coro user data
    int param1 = 0;
    int param2 = 0;
    void *ptrParam1 = nullptr;
};

class BaseRpcCoroMgr {
public:
    BaseRpcCoroMgr();
    virtual ~BaseRpcCoroMgr();

    virtual bool HasCoro() = 0;

    // Create coroutine.
    virtual Coro *CreateCoro(const std::function<void(void *)> &entry, void *args, const std::string &userData) = 0;

    virtual Coro *GetCoro(const int64_t &coroId) = 0;

    // Yield coroutine.
    virtual void YieldCoro(const int64_t &timeout = -1) = 0;

    // Resume coroutine.
    virtual void ResumeCoro(int64_t coroId, const std::string &pass_data = "") = 0;

    // Get coroutine
    virtual Coro *GetCurCoro() = 0;

    // Get coroutine
    virtual int64_t GetCurCoroId() = 0;
};

// 协程管理器、Todo:待实现
// class RpcCoroMgr
// {
// public:
//     RpcCoroMgr();
//     virtual ~RpcCoroMgr();

//     virtual bool HasCoro() {};

//     // Create coroutine.
//     virtual int64_t CreateCoro(const std::function<void(void *)> &entry,
//                               void *args,
//                               const std::string &userData) { return 0; };

//     // Yield coroutine.
//     virtual void YieldCoro(const int64_t &timeout = -1) {};

//     // Resume coroutine.
//     virtual void ResumeCoro(int64_t coroId, const std::string &pass_data = "") {};

//     // Get coroutine
//     virtual int64_t GetCurCoroId() { return 0; };
// };

// 全局协程管理单例,Todo:待实现
extern BaseRpcCoroMgr *g_rpcCoroMgr;
