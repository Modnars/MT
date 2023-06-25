/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-21 15:47:04
 * @edit: regangcli
 * @brief:
 */
#include "demo_service_impl.h"
#include "common/demo.pb.h"
#include "llbc.h"
#include "rpc_channel.h"

using namespace llbc;

void DemoServiceImpl::Echo(::google::protobuf::RpcController * /* controller */, const ::protocol::EchoReq *req,
                           ::protocol::EchoRsp *rsp, ::google::protobuf::Closure *done) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "RECEIVED MSG: %s", req->msg().c_str());
    if (req->msg().size() > 0UL && req->msg()[0] != 'A') {
        protocol::EchoReq new_req;
        protocol::EchoRsp new_rsp;
        new_req.set_msg(std::string("A ") + req->msg());
        DemoServiceHelper::GetInst().Stub().Echo(&RpcController::GetInst(), &new_req, &new_rsp, done);
        rsp->set_msg(std::string("FIX: ") + new_rsp.msg());
        done->Run();
        return;
    }
    rsp->set_msg(std::string("OK: ") + req->msg());
    done->Run();
}