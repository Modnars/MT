/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-15 20:21:17
 * @edit: regangcli
 * @brief:
 */

#pragma once

#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <mt/task.h>
#include <mt/util/singleton.h>
#include <map>

class ConnMgr;

class RpcController : public ::google::protobuf::RpcController, public mt::Singleton<RpcController> {
public:
    virtual void Reset() { }
    virtual bool Failed() const { return false; }
    virtual std::string ErrorText() const { return ""; }
    virtual void StartCancel() { }
    virtual void SetFailed(const std::string & /* reason */) { }
    virtual bool IsCanceled() const { return false; }
    virtual void NotifyOnCancel(::google::protobuf::Closure * /* callback */) { }
    uint64_t GetID() { return ++task_generate_id_; }
    mt::Task<> *GetTaskbyID(uint64_t id) {
        auto it = id_to_task_map_.find(id);
        if (it != id_to_task_map_.end()) {
            return &it->second;
        }
        return nullptr;
    }
    ::google::protobuf::Message *GetRsp() { return rsp_; }

private:
    uint64_t task_generate_id_;
    std::map<uint64_t /* task_id_ */, mt::Task<> /* task_ */> id_to_task_map_;
    ::google::protobuf::Message *rsp_;
};

class RpcChannel : public ::google::protobuf::RpcChannel {
public:
    RpcChannel(ConnMgr *connMgr, int sessionId) : connMgr_(connMgr), sessionId_(sessionId) { }
    virtual ~RpcChannel();

    virtual void CallMethod(const ::google::protobuf::MethodDescriptor *method,
                            ::google::protobuf::RpcController *controller, const ::google::protobuf::Message *request,
                            ::google::protobuf::Message *response, ::google::protobuf::Closure *);

private:
    ConnMgr *connMgr_ = nullptr;
    int sessionId_ = 0;
};
