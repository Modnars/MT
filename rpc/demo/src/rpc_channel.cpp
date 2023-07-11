/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-15 20:21:17
 * @edit: regangcli
 * @brief:
 */
#include <coroutine>

#include <llbc.h>
#include <mt/runner.h>

#include "common/error_code.pb.h"
#include "conn_mgr.h"
#include "macros.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_service_mgr.h"

using namespace llbc;

RpcChannel::~RpcChannel() {
    connMgr_->CloseSession(sessionId_);
}

int32_t RpcChannel::BlockingWaitResponse(::google::protobuf::Message *response) {
    auto recvPacket = connMgr_->PopPacket();
    int count = 0;
    while (!recvPacket && count < 100) {
        LLBC_Sleep(1);
        ++count;
        recvPacket = connMgr_->PopPacket();
    }

    if (!recvPacket) {
        response->Clear();
        LLOG_ERROR("receive packet timeout!");
        return protocol::ErrorCode::RECV_PKG_TIME_OUT;
    }

    LLOG_TRACE("Payload info|length:%lu|contents: %s", recvPacket->GetPayloadLength(),
               reinterpret_cast<const char *>(recvPacket->GetPayload()));
    PkgHead pkg_head;
    auto ret = util::ParseNetPacket(*recvPacket, pkg_head);
    COND_RET_ELOG(ret != 0, ret, "parse net packet failed|ret:%d", ret);
    ret = recvPacket->Read(*response);
    COND_RET_ELOG(ret != 0, protocol::ErrorCode::PARSE_PKG_FAILED, "read recvpacket failed|ret:%d", ret);

    LLBC_Recycle(recvPacket);
    LLOG_TRACE("recved: %s|extdata:%lu", response->ShortDebugString().c_str(), pkg_head.coro_uid);
    return 0;
}

// 先预留个接口在这里，内部先用阻塞的方式搞一下
mt::Task<> RpcChannel::AwaitResponse(::google::protobuf::Message *response) {
    LLOG_TRACE("[AwaitResponse] COROUTINE START");
    BlockingWaitResponse(response);
    LLOG_TRACE("[AwaitResponse] COROUTINE RESUME");
    LLOG_TRACE("[AwaitResponse] COROUTINE RETURN");
    co_return;
}

void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
                            ::google::protobuf::RpcController *controller, const ::google::protobuf::Message *request,
                            ::google::protobuf::Message *response, ::google::protobuf::Closure *done) {
    LLOG_TRACE("CallMethod");
    LLBC_Packet *sendPacket = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    sendPacket->SetHeader(0, RpcOpCode::RpcReq, 0);
    sendPacket->SetSessionId(sessionId_);
    sendPacket->Write(method->service()->name());
    sendPacket->Write(method->name());

// 协程方案，需填充原始协程 id
#if ENABLE_CXX20_COROUTINE
    std::uint64_t coro_uid = RpcController::GetInst().UseCoro() ? RpcCoroMgr::GetInst().NewCoroUid() : 0UL;
    sendPacket->Write(coro_uid);
    LLOG_TRACE("fill coro_uid done|coro_uid:%lu", coro_uid);
#else
    sendPacket->Write(uint64(666));
#endif

    sendPacket->Write(*request);
    connMgr_->PushPacket(sendPacket);
    // respone 为空时直接返回，不等回包
    COND_RET(!response);

// 协程方案
#if ENABLE_CXX20_COROUTINE
    mt::run(AwaitResponse(response));
    LLOG_TRACE("await response done|address:%p|response: %s", response, response->ShortDebugString().c_str());
#else
    BlockingWaitResponse(response);
    LLOG_TRACE("await response done|address:%p|response: %s", response, response->ShortDebugString().c_str());
#endif
}