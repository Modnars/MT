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

using namespace llbc;

RpcChannel::~RpcChannel() {
    connMgr_->CloseSession(sessionId_);
}

void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
                            ::google::protobuf::RpcController * /* controller */,
                            const ::google::protobuf::Message *request, ::google::protobuf::Message *response,
                            ::google::protobuf::Closure *) {
// 协程方案, 不允许在主协程中call Rpc
#ifdef UseCoroRpc
    auto coro = g_rpcCoroMgr->GetCurCoro();
    if (coro->IsMainCoro()) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "Main coro not allow call Yield() method!");
        return;
    }
#endif

    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "CallMethod!");
    LLBC_Packet *sendPacket = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    sendPacket->SetHeader(0, RpcOpCode::RpcReq, 0);
    sendPacket->SetSessionId(sessionId_);
    sendPacket->Write(method->service()->name());
    sendPacket->Write(method->name());
    sendPacket->Write(*request);

// // 协程方案，需填充原始协程id
#ifdef UseCoroRpc
    sendPacket->SetExtData1(g_rpcCoroMgr->GetCurCoroId());
#endif

    connMgr_->PushPacket(sendPacket);
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Waiting!");

// 协程方案
#ifdef UseCoroRpc
    coro->Yield();  // yield当前协程，直到收到回包
    // 协程被唤醒时，从协程获取接收到的回包
    LLBC_Packet *recvPacket = reinterpret_cast<LLBC_Packet *>(coro->GetPtrParam1());
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "PayLoad(%d):%s", recvPacket->GetPayloadLength(),
         recvPacket->GetPayload());
    recvPacket->Read(*response);
    // LLBC_Recycle(recvPacket);
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Recved: %s", response->DebugString().c_str());

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

    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "PayLoad(%d):%s", recvPacket->GetPayloadLength(),
         recvPacket->GetPayload());
    recvPacket->Read(*response);
    LLBC_Recycle(recvPacket);
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Recved: %s", response->DebugString().c_str());
#endif
}