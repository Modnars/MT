#include <llbc.h>

#include "demo_service.pb.h"
#include "macros.h"

mt::Task<int> protocol::DemoServiceImpl::Echo(::google::protobuf::RpcController *controller,
                                              const ::protocol::EchoReq &req, ::protocol::EchoRsp &rsp,
                                              ::google::protobuf::Closure *done) {
    LLOG_TRACE("req: %s", req.ShortDebugString().c_str());
    if (req.msg().size() > 0UL && req.msg()[0UL] != 'A') {
        protocol::EchoReq inner_req;
        protocol::EchoRsp inner_rsp;
        inner_req.set_msg(std::string("ACK: ") + req.msg());
        std::uint64_t uid = 1UL;  // 路由到另一个服务进程（用 uid % channels_num 来路由）
        LLOG_INFO("send req|req:%s", inner_req.ShortDebugString().c_str());
        auto ret = co_await DemoServiceStub::Echo(uid, inner_req, &inner_rsp);
        CO_COND_RET_WLOG(ret != 0, ret, "call DemoServiceStub::Echo failed|ret:%d", ret);
        LLOG_TRACE("get inner rsp: %s", inner_rsp.ShortDebugString().c_str());
        rsp.set_msg(inner_rsp.msg() + " @2");
        co_return 0;
    }
    rsp.set_msg(req.msg() + " @1");
    LLOG_TRACE("rsp: %s", rsp.ShortDebugString().c_str());
    co_return 0;
}
