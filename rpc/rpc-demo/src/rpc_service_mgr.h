/*
 * @file: 
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-19 20:14:54
 * @edit: regangcli
 * @brief: 
 */
#pragma once

#include "google/protobuf/message.h"
#include "google/protobuf/service.h"
#include "google/protobuf/stubs/common.h"
#include <map>

class ConnMgr;
class RpcServiceMgr 
{
public:
    RpcServiceMgr(ConnMgr *connMgr) 
    {
        connMgr_ = connMgr;
    }

    void OnUpdate();
    // 添加服务
    void AddService(::google::protobuf::Service* service);

private:
    void OnRpcDone(
            ::google::protobuf::Message* recv_msg,
            ::google::protobuf::Message* resp_msg);
    // void dispatch_msg(
    //         const std::string& service_name,
    //         const std::string& method_name,
    //         const std::string& serialzied_data);
    // void pack_message(
    //         const ::google::protobuf::Message* msg,
    //         std::string* serialized_data);

private:
    struct ServiceInfo{
        ::google::protobuf::Service* service;
        const ::google::protobuf::ServiceDescriptor* sd;
        std::map<std::string, const ::google::protobuf::MethodDescriptor*> mds;
    };//ServiceInfo

    std::map<std::string, ServiceInfo> _services;
    ConnMgr *connMgr_ = nullptr;
    int sessionId_ = 0;
    int serviceId_ = 0;
    bool stop_ = false;
};//RpcServiceMgr