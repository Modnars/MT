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

#include "common/demo.pb.h"
#include "common/demo_service.pb.h"
#include "conn_mgr.h"
#include "macros.h"
#include "rpc_channel.h"

// ==================== PERF: AUTO GENERATE FROM PROTOC BEGIN ====================

class DemoServiceImpl : public protocol::DemoService {
public:
    void Echo(::google::protobuf::RpcController *controller, const ::protocol::EchoReq *req, ::protocol::EchoRsp *rsp,
              ::google::protobuf::Closure *done) override;

    mt::Task<int> Echo(::google::protobuf::RpcController *controller, const ::protocol::EchoReq &req,
                       ::protocol::EchoRsp &rsp, ::google::protobuf::Closure *done) override;

};  // DemoServiceImpl

class DemoServiceStub {
public:
    static mt::Task<int> Echo(std::uint64_t uid, const ::protocol::EchoReq &req, ::protocol::EchoRsp *rsp = nullptr);
};  // DemoServiceStub

// ==================== PERF: AUTO GENERATE FROM PROTOC END ====================
