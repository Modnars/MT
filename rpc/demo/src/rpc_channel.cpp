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
#include "llbc/core/log/LoggerMgr.h"
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
    auto task_id = RpcCoroMgr::GetInst().NewCoroUid();
    sendPacket->Write(static_cast<std::uint64_t>(task_id));
    LLOG_TRACE("fill task_id done|task_id:%lu", task_id);
#else
    sendPacket->Write(uint64(666));
#endif

    sendPacket->Write(*request);
    connMgr_->PushPacket(sendPacket);
    // respone 为空时直接返回，不等回包
    COND_RET(!response);

// 协程方案
#if ENABLE_CXX20_COROUTINE
    // auto func = [&response](void *) -> mt::Task<> {
    //     LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Wait Rsp");
    //     RpcController::GetInst().SetRsp(response);
    //     co_await std::suspend_always{};
    //     // 处理rsp
    //     LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Task Resume");
    //     response = RpcController::GetInst().GetRsp();
    //     LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "response: %d(%s)", response, response->DebugString().c_str());
    //     co_return;
    // };
    // mt::Task<> task = func(nullptr);
    // id_to_task_map_.insert(std::make_pair(task_id, std::move(func(nullptr))));
    // mt::run(id_to_task_map_.find(task_id)->second);
    BlockingWaitResponse(response);
    LLOG_TRACE("await response done|address:%p|response: %s", response, response->ShortDebugString().c_str());
#else
    // 直接等待暴力回包方案, 100ms超时
    auto recvPacket = connMgr_->PopPacket();
    int count = 0;
    while (!recvPacket && count < 100) {
        LLBC_Sleep(1);
        count++;
        recvPacket = connMgr_->PopPacket();
    }

    if (!recvPacket) {
        response->Clear();
        LLOG_ERROR("RecvPacket timeout!");
        return;
    }

    LLOG_TRACE("PayLoad(%lu):%s", recvPacket->GetPayloadLength(),
               reinterpret_cast<const char *>(recvPacket->GetPayload()));
    std::string A, B;
    uint64 srcCoroId;
    recvPacket->Read(A);
    recvPacket->Read(B);
    if (recvPacket->Read(srcCoroId) != LLBC_OK || recvPacket->Read(*response) != LLBC_OK) {
        LLOG_ERROR("Read recvPacket fail");
        return;
    }

    LLBC_Recycle(recvPacket);
    LLOG_TRACE("Recved: %s, extdata: %lu", response->DebugString().c_str(), srcCoroId);
#endif
}