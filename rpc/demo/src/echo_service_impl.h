/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-21 15:46:03
 * @edit: regangcli
 * @brief:
 */
#pragma once
#include "echo.pb.h"

class MyEchoService : public echo::EchoService {
public:
    void Echo(::google::protobuf::RpcController *controller, const ::echo::EchoRequest *request,
              ::echo::EchoResponse *response, ::google::protobuf::Closure *done) override;

};  // MyEchoService