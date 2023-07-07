/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-21 15:46:03
 * @edit: regangcli
 * @brief:
 */
#pragma once

#include <unordered_map>

#include <mt/util/macros.h>
#include <mt/util/singleton.h>

#include "common/demo_service.pb.h"
#include "conn_mgr.h"
#include "macros.h"
#include "rpc_channel.h"

class DemoServiceImpl : public protocol::DemoService {
public:
    void Echo(::google::protobuf::RpcController *controller, const ::protocol::EchoReq *req, ::protocol::EchoRsp *rsp,
              ::google::protobuf::Closure *done) override;

};  // DemoServiceImpl

// NOTES: 便于 server 端调用，支持自动生成更好
class DemoServiceHelper : public mt::Singleton<DemoServiceHelper> {
public:
    using server_id = std::uint64_t;

public:
    ~DemoServiceHelper() {
        for (auto kv : stubs_) {
            if (kv.second)
                delete kv.second;
        }
    }

public:
    bool Init() { return true; }

    bool Register(const char *ip, int port, server_id sid) {
        if (auto iter = stubs_.find(sid); iter != stubs_.end()) {
            LLOG_ERROR("repeated server_id|sid:%lu", sid);
            return false;
        }
        RpcChannel *channel = ConnMgr::GetInst().CreateRpcChannel(ip, port);
        COND_RET_ELOG(!channel, false, "create rpc channel failed|ip:%s|port:%d", ip, port);
        // LLBC_Defer(delete channel);

        // 内部 rpc 调用
        stubs_[sid] = new protocol::DemoService_Stub{channel};
        COND_RET_ELOG(stubs_[sid] == nullptr, false, "create service stub failed|sid:%lu", sid);
        return true;
    }

    protocol::DemoService_Stub *Stub(server_id sid = 0UL) const {
        if (auto iter = stubs_.find(sid); iter != stubs_.end()) {
            return iter->second;
        }
        return nullptr;
    }

private:
    // map<uid, stub>
    std::unordered_map<std::uint64_t, protocol::DemoService_Stub *> stubs_;
};
