/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-21 15:46:03
 * @edit: regangcli
 * @brief:
 */
#pragma once

#include <mt/util/macros.h>
#include <mt/util/singleton.h>
#include "common/demo_service.pb.h"

class DemoServiceImpl : public protocol::DemoService {
public:
    void Echo(::google::protobuf::RpcController *controller, const ::protocol::EchoReq *req, ::protocol::EchoRsp *rsp,
              ::google::protobuf::Closure *done) override;

};  // DemoServiceImpl

// NOTES: 便于 server 端调用，支持自动生成更好
class DemoServiceHelper : public mt::Singleton<DemoServiceHelper> {
public:
    bool Init(protocol::DemoService_Stub *stub) {
        COND_RET(!stub, false);
        stub_ = stub;
        return true;
    }
    protocol::DemoService_Stub &Stub() const { return *stub_; }

private:
    protocol::DemoService_Stub *stub_ = nullptr;
};
