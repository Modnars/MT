/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-21 15:47:04
 * @edit: regangcli
 * @brief:
 */
#include <llbc.h>
#include <mt/runner.h>

#include "common/demo.pb.h"
#include "conn_mgr.h"
#include "demo_service_impl.h"
#include "macros.h"
#include "rpc_channel.h"
#include "rpc_service_mgr.h"

using namespace llbc;

void DemoServiceImpl::Echo(::google::protobuf::RpcController *controller, const ::protocol::EchoReq *req,
                           ::protocol::EchoRsp *rsp, ::google::protobuf::Closure *done) {
    auto ret = mt::run(Echo(controller, *req, *rsp, done));
    COND_RET_ELOG(ret != 0, , "coroutine exec failed|ret:%d", ret);
}

mt::Task<int> DemoServiceStub::Echo(std::uint64_t uid, const ::protocol::EchoReq &req, ::protocol::EchoRsp *rsp) {
    co_return co_await RpcServiceMgr::GetInst().Rpc(0x00000001U, uid, req, rsp);
}

mt::Task<int> DemoServiceImpl::Echo(::google::protobuf::RpcController *controller, const ::protocol::EchoReq &req,
                                    ::protocol::EchoRsp &rsp, ::google::protobuf::Closure *done) {
    LLOG_TRACE("request msg: %s", req.msg().c_str());
    if (req.msg().size() > 0UL && req.msg()[0] != 'A') {
        protocol::EchoReq inner_req;
        protocol::EchoRsp inner_rsp;
        inner_req.set_msg(std::string("A ") + req.msg());
        std::uint64_t uid = 1UL;
        auto ret = co_await DemoServiceStub::Echo(uid, inner_req, &inner_rsp);
        CO_COND_RET_WLOG(ret != 0, ret, "call DemoServiceStub::Echo failed|ret:%d", ret);
        LLOG_TRACE("received inner rsp: %s", inner_rsp.msg().c_str());
        rsp.set_msg(std::string("FIX: ") + inner_rsp.msg());
        co_return 0;
    }
    rsp.set_msg(std::string("OK: ") + req.msg());
    co_return 0;
}
