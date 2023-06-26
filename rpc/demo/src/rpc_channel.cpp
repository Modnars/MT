/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-15 20:21:17
 * @edit: regangcli
 * @brief:
 */
#include <mt/runner.h>
#include <coroutine>
#include "llbc.h"

#include "conn_mgr.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"

using namespace llbc;

RpcChannel::~RpcChannel() {
    connMgr_->CloseSession(sessionId_);
}

void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
                            ::google::protobuf::RpcController *controller, const ::google::protobuf::Message *request,
                            ::google::protobuf::Message *response, ::google::protobuf::Closure *) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "CallMethod!");
    LLBC_Packet *sendPacket = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    sendPacket->SetHeader(0, RpcOpCode::RpcReq, 0);
    sendPacket->SetSessionId(sessionId_);
    sendPacket->Write(method->service()->name());
    sendPacket->Write(method->name());

// 协程方案，需填充原始协程 id
#if ENABLE_CXX20_COROUTINE
    auto task_id = RpcController::GetInst().GetID();
    sendPacket->Write(task_id);
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "set extdata: %lu", task_id);
#else
    sendPacket->Write(uint64(666));
#endif

    sendPacket->Write(*request);
    connMgr_->PushPacket(sendPacket);
    // respone为空时直接返回，不等回包
    if (!response) {
        return;
    }

// 协程方案
#if ENABLE_CXX20_COROUTINE
    auto func = [&response](void *) -> mt::Task<> {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Wait Rsp");
        co_await std::suspend_always{};
        // 处理rsp
        response->CopyFrom(*RpcController::GetInst().GetRsp());
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Recved : %s", response->DebugString().c_str());
        co_return;
    };
    //mt::Task<> task = func(nullptr);
    id_to_task_map_.insert(std::make_pair(task_id, std::move(func(nullptr))));
    mt::run(id_to_task_map_.find(task_id)->second);
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
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "RecvPacket timeout!");
        return;
    }

    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "PayLoad(%lu):%s", recvPacket->GetPayloadLength(),
         reinterpret_cast<const char *>(recvPacket->GetPayload()));
    std::string A, B;
    uint64 srcCoroId;
    recvPacket->Read(A);
    recvPacket->Read(B);
    if (recvPacket->Read(srcCoroId) != LLBC_OK || recvPacket->Read(*response) != LLBC_OK) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "Read recvPacket fail");
        return;
    }

    LLBC_Recycle(recvPacket);
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Recved: %s, extdata: %lu", response->DebugString().c_str(),
         srcCoroId);
#endif
}