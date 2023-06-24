/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-21 15:46:03
 * @edit: regangcli
 * @brief:
 */
#pragma once

#include "common/demo_service.pb.h"

class DemoServiceImpl : public protocol::DemoService {
public:
    void Echo(::google::protobuf::RpcController *controller, const ::protocol::EchoReq *req, ::protocol::EchoRsp *rsp,
              ::google::protobuf::Closure *done) override;

};  // MyEchoService