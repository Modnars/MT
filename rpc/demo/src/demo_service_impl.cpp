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
#include "conn_mgr.h"

using namespace llbc;

void DemoServiceImpl::Echo(::google::protobuf::RpcController * /* controller */, const ::protocol::EchoReq *req,
                           ::protocol::EchoRsp *rsp, ::google::protobuf::Closure *done) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "RECEIVED MSG: %s", req->msg().c_str());
    if (req->msg().size() > 0UL && req->msg()[0] != 'A') {
        protocol::EchoReq innerReq;
        protocol::EchoRsp innerRsp;
        innerReq.set_msg(std::string("A ") + req->msg());
        // 创建 rpc channel
        RpcChannel *channel = ConnMgr::GetInst().CreateRpcChannel("127.0.0.1", 6699);
        if (!channel) {
            LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "CreateRpcChannel Fail");
            return;
        }

        // 内部 rpc 调用
        protocol::DemoService_Stub stub(channel);
        stub.Echo(&RpcController::GetInst(), &innerReq, &innerRsp, nullptr);
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "RECEIVED inner RSP: %s", innerRsp.msg().c_str());

        rsp->set_msg(std::string("FIX: ") + innerRsp.msg());
        done->Run();
        delete channel;
        return;
    }

    rsp->set_msg(std::string("OK: ") + req->msg());
    done->Run();
}