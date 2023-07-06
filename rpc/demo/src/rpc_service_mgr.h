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

struct PkgHead {
    std::string service = "";
    std::string method = "";
    std::uint64_t coro_uid = 0UL;
};

namespace util {

/// @brief 解析网络包数据，将其数据缓存至 PkgHead 结构以便于后续访问使用
/// @param [IN/OUT] packet 网络包数据 内部调用 packet 的 Read 接口会导致其内部成员发生变化，所以这里是传入其引用
/// @param [OUT] pkg_head 解析得到的 PkgHead 结构参数
/// @return 解析成功时返回 ErrorCode::SUCCESS，否则返回其他定义的错误码。
int32_t ParseNetPacket(llbc::LLBC_Packet &packet, PkgHead &pkg_head);

}  // namespace util

class ConnMgr;
class RpcServiceMgr {
public:
    RpcServiceMgr(ConnMgr *connMgr);
    virtual ~RpcServiceMgr();

    // 添加服务
    void AddService(::google::protobuf::Service *service);

private:
    // 处理 RPC 请求和返回包
    void HandleRpcReq(llbc::LLBC_Packet &packet);
    void HandleRpcRsp(llbc::LLBC_Packet &packet);
    // 处理 RPC 处理结束
    void OnRpcDone(::google::protobuf::Message *recv_msg, ::google::protobuf::Message *resp_msg,
                   const ::google::protobuf::MethodDescriptor *method = nullptr, uint64_t task_id = 0);

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