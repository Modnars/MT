/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-15 20:21:17
 * @edit: regangcli
 * @brief:
 */

#pragma once

#include <map>

#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <mt/task.h>
#include <mt/util/singleton.h>

class ConnMgr;

class RpcController : public ::google::protobuf::RpcController, public mt::Singleton<RpcController> {
public:
    RpcController() : rsp_(nullptr) { }
    ~RpcController() { }
    virtual void Reset() { }
    virtual bool Failed() const { return false; }
    virtual std::string ErrorText() const { return ""; }
    virtual void StartCancel() { }
    virtual void SetFailed(const std::string & /* reason */) { }
    virtual bool IsCanceled() const { return false; }
    virtual void NotifyOnCancel(::google::protobuf::Closure * /* callback */) { }

public:
    void SetUseCoro(bool use_coro) { use_coro_ = use_coro; }
    bool UseCoro() const { return use_coro_; }

    ::google::protobuf::Message *GetRsp() { return rsp_; }
    void SetRsp(::google::protobuf::Message *rsp) { rsp_ = rsp; }

private:
    ::google::protobuf::Message *rsp_;
    bool use_coro_ = false;  // RPC 是否启用协程，不启用协程时相应协程 ID 字段填充为 0
};

class RpcChannel : public ::google::protobuf::RpcChannel {
public:
    RpcChannel(ConnMgr *connMgr, int sessionId) : connMgr_(connMgr), sessionId_(sessionId) { }
    virtual ~RpcChannel();

    void CallMethod(const ::google::protobuf::MethodDescriptor *method, ::google::protobuf::RpcController *controller,
                    const ::google::protobuf::Message *request, ::google::protobuf::Message *response,
                    ::google::protobuf::Closure *done) override;

private:
    int32_t BlockingWaitResponse(::google::protobuf::Message *response);
    mt::Task<> AwaitResponse(::google::protobuf::Message *response);

private:
    ConnMgr *connMgr_ = nullptr;
    int sessionId_ = 0;
};
