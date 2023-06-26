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

#include "google/protobuf/message.h"
#include "google/protobuf/service.h"
#include "google/protobuf/stubs/common.h"

namespace llbc {
class LLBC_Packet;
}

class ConnMgr;
class RpcServiceMgr {
public:
    RpcServiceMgr(ConnMgr *connMgr);
    virtual ~RpcServiceMgr();

    // 添加服务
    void AddService(::google::protobuf::Service *service);

private:
    // 预处理 packet
    int32_t PreHandlePacket(LLBC_Packet &packet, std::string& serviceName, std::string& methodName, uint64_t& task_id);
    // 处理 RPC 请求和返回包
    void HandleRpcReq(llbc::LLBC_Packet &packet);
    void HandleRpcRsp(llbc::LLBC_Packet &packet);
    // 处理 RPC 处理结束
    void OnRpcDone(::google::protobuf::Message *recv_msg, ::google::protobuf::Message *resp_msg, const ::google::protobuf::MethodDescriptor *method = nullptr, uint64_t task_id = 0);

private:
    struct ServiceInfo {
        ::google::protobuf::Service *service;
        const ::google::protobuf::ServiceDescriptor *sd;
        std::map<std::string, const ::google::protobuf::MethodDescriptor *> mds;
    };  // ServiceInfo

    std::map<std::string, ServiceInfo> _services;
    ConnMgr *connMgr_ = nullptr;
    int sessionId_ = 0;
    int serviceId_ = 0;
    bool stop_ = false;
};  // RpcServiceMgr