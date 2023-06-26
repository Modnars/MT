/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-15 20:21:17
 * @edit: regangcli
 * @brief:
 */
#include "rpc_channel.h"
#include "conn_mgr.h"
#include "llbc.h"
#include "rpc_coro_mgr.h"
#include <mt/runner.h>

using namespace llbc;

RpcChannel::~RpcChannel() {
    connMgr_->CloseSession(sessionId_);
}

void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
                            ::google::protobuf::RpcController *controller,
                            const ::google::protobuf::Message *request, ::google::protobuf::Message *response,
                            ::google::protobuf::Closure *) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "CallMethod!");
    LLBC_Packet *sendPacket = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    sendPacket->SetHeader(0, RpcOpCode::RpcReq, 0);
    sendPacket->SetSessionId(sessionId_);
    sendPacket->Write(method->service()->name());
    sendPacket->Write(method->name());

// 协程方案，需填充原始协程 id
#ifdef ENABLE_CXX20_COROUTINE
    auto task_id = RpcController::GetInst().GetID();
    sendPacket->Write(task_id);
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "set extdata: %lu", task_id);
#else
    sendPacket->Write(uint64(666));
#endif

    sendPacket->Write(*request);
    connMgr_->PushPacket(sendPacket);
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Waiting!");

    // respone为空时直接返回，不等回包
    if(!response){
        return;
    }

// 协程方案
#ifndef ENABLE_CXX20_COROUTINE
    auto func = [&response](void *) -> mt::Task<> {
            // 处理rsp
            // co_yield 0;
            response->CopyFrom(*RpcController::GetInst().GetRsp());
            LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Recved : %s", response->DebugString().c_str());
        };
    mt::run(func(nullptr));
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
    std::string A,B;
    uint64 srcCoroId;
    recvPacket->Read(A);
    recvPacket->Read(B);
    if (recvPacket->Read(srcCoroId) != LLBC_OK
        || recvPacket->Read(*response) != LLBC_OK) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "Read recvPacket fail");
        return;
    }

    LLBC_Recycle(recvPacket);
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Recved: %s, extdata: %lu", response->DebugString().c_str(), srcCoroId);
#endif
}