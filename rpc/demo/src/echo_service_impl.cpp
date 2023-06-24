/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-21 15:47:04
 * @edit: regangcli
 * @brief:
 */
#include "echo_service_impl.h"
#include "llbc.h"

using namespace llbc;

void DemoServiceImpl::Echo(::google::protobuf::RpcController * /* controller */, const ::protocol::EchoReq *req,
                           ::protocol::EchoRsp *rsp, ::google::protobuf::Closure *done) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "received, msg:%s", req->msg().c_str());
    rsp->set_msg(std::string("I have received '") + req->msg() + std::string("'"));
    done->Run();
}