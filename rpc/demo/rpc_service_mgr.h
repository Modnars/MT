/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-19 20:14:54
 * @edit: regangcli
 * @brief:
 */
#pragma once

#include <map>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <mt/task.h>
#include <mt/util/singleton.h>

#include "rpc_channel.h"

namespace llbc {
class LLBC_Packet;
}

class CtxController : public ::google::protobuf::RpcController {
public:
    CtxController(void *continuation) : continuation_(continuation) { }
    ~CtxController() { }

    virtual void Reset() { }
    virtual bool Failed() const { return false; }
    virtual std::string ErrorText() const { return ""; }
    virtual void StartCancel() { }
    virtual void SetFailed(const std::string & /* reason */) { }
    virtual bool IsCanceled() const { return false; }
    virtual void NotifyOnCancel(::google::protobuf::Closure * /* callback */) { }

    void *GetContinuation() const { return continuation_; }
    void SetContinuation(void *continuation) { continuation_ = continuation; }

private:
    void *continuation_ = nullptr;  // 恢复执行的协程挂起点
};

class ConnMgr;
class RpcServiceMgr : public mt::Singleton<RpcServiceMgr> {
public:
    using pb_service_method = std::pair<::google::protobuf::Service *, const ::google::protobuf::MethodDescriptor *>;

public:
    int Init(ConnMgr *conn_mgr);
    virtual ~RpcServiceMgr();

public:
    // 添加服务
    bool AddService(::google::protobuf::Service *service);
    bool RegisterChannel(const char *ip, int32_t port);

    mt::Task<int> Rpc(std::uint32_t cmd, std::uint64_t uid, const ::google::protobuf::Message &req,
                      ::google::protobuf::Message *rsp = nullptr);

private:
    // 处理 RPC 请求和返回包
    void HandleRpcReq(llbc::LLBC_Packet &packet);
    void HandleRpcRsp(llbc::LLBC_Packet &packet);

    mt::Task<int> DealRequest(llbc::LLBC_Packet packet);
    mt::Task<int> DealResonse(llbc::LLBC_Packet packet);

    // 处理 RPC 结束回调
    void OnRpcDone(const PkgHead &pkg_head, const ::google::protobuf::Message &rsp);

private:
    ConnMgr *conn_mgr_ = nullptr;
    int session_id_ = 0;  // 收发包时缓存的 session_id

    std::vector<RpcChannel *> channels_;
    std::unordered_map<std::uint32_t, pb_service_method> services_;
};  // RpcServiceMgr