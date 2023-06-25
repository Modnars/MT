/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-19 20:13:51
 * @edit: regangcli
 * @brief:
 */
#define ENABLE_CXX20_COROUTINE 1

#include <fmt/color.h>
#include "llbc.h"

#include <mt/runner.h>
#include <mt/task.h>

#include "conn_mgr.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_service_mgr.h"

using namespace llbc;

RpcServiceMgr::RpcServiceMgr(ConnMgr *connMgr) : connMgr_(connMgr) {
    connMgr_->Subscribe(RpcOpCode::RpcReq, LLBC_Delegate<void(LLBC_Packet &)>(this, &RpcServiceMgr::HandleRpcReq));
    connMgr_->Subscribe(RpcOpCode::RpcRsp, LLBC_Delegate<void(LLBC_Packet &)>(this, &RpcServiceMgr::HandleRpcRsp));
}

RpcServiceMgr::~RpcServiceMgr() {
    connMgr_->Unsubscribe(RpcOpCode::RpcReq);
    connMgr_->Unsubscribe(RpcOpCode::RpcRsp);
}

void RpcServiceMgr::AddService(::google::protobuf::Service *service) {
    ServiceInfo service_info;
    service_info.service = service;
    service_info.sd = service->GetDescriptor();
    for (int i = 0; i < service_info.sd->method_count(); ++i) {
        service_info.mds[service_info.sd->method(i)->name()] = service_info.sd->method(i);
    }

    _services[service_info.sd->name()] = service_info;
}

void RpcServiceMgr::HandleRpcReq(LLBC_Packet &packet) {
    // 读取 serviceName & methodName
    std::string serviceName;
    std::string methodName;
    packet.Read(serviceName);
    packet.Read(methodName);
    auto *service = _services[serviceName].service;
    auto *method = _services[serviceName].mds[methodName];
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "recv service_name: %s", serviceName.c_str());
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "recv method_name: %s", methodName.c_str());
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "req type: %s", method->input_type()->name().c_str());
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "rsp type: %s", method->output_type()->name().c_str());

    // 解析 req & 创建 rsp
    auto *req = service->GetRequestPrototype(method).New();
    packet.Read(*req);
    auto *rsp = service->GetResponsePrototype(method).New();

#if ENABLE_CXX20_COROUTINE
    auto func = [&packet, service, method, req, rsp, this](void *) -> mt::Task<> {
        fmt::print(fg(fmt::color::floral_white) | bg(fmt::color::slate_gray) | fmt::emphasis::underline,
                   "[HANDLE_RPC_REQ] THIS IS A COROUTINE CALL FROM C++20\n");
        this->sessionId_ = packet.GetSessionId();
        auto done = ::google::protobuf::NewCallback(this, &RpcServiceMgr::OnRpcDone, req, rsp);
        service->CallMethod(method, &RpcController::GetInst(), req, rsp, done);
        co_return;
    };
    mt::run(func(nullptr));
#else
    // 直接调用方案
    sessionId_ = packet.GetSessionId();

    // 创建 rpc 完成回调函数
    auto done = ::google::protobuf::NewCallback(this, &RpcServiceMgr::OnRpcDone, req, rsp);
    service->CallMethod(method, &RpcController::GetInst(), req, rsp, done);
#endif
}

void RpcServiceMgr::HandleRpcRsp(LLBC_Packet &packet) {
#if ENABLE_CXX20_COROUTINE
    fmt::print(fg(fmt::color::floral_white) | bg(fmt::color::slate_gray) | fmt::emphasis::underline,
               "[HANDLE_RPC_RSP] THIS IS A COROUTINE CALL FROM C++20\n");
    // auto dstCoroId = packet.GetExtData1();
    // Coro *coro = g_rpcCoroMgr->GetCoro(dstCoroId);
    // if (!coro) {
    //     LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "coro not found, coroId:%d", dstCoroId);
    // }
    // // TODO: 唤醒休眠的协程，传递收到的packet
    // else {
    //     coro->SetPtrParam1(&packet);
    //     coro->Resume();
    // }
#endif
}

void RpcServiceMgr::OnRpcDone(::google::protobuf::Message *req, ::google::protobuf::Message *rsp) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "OnRpcDone, req: %s, rsp: %s", req->DebugString().c_str(),
         rsp->DebugString().c_str());

#ifdef UseCoroRpc
    // 协程方案
    auto coro = g_rpcCoroMgr->GetCurCoro();
    auto sessionId = coro->GetParam1();
    auto srcCoroId = coro->GetParam2();
    auto packet = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    packet->SetSessionId(sessionId);
    packet->SetOpcode(RpcOpCode::RpcRsp);
    packet->SetExtData1(srcCoroId);

#else
    // 直接调用方案
    auto packet = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    packet->SetSessionId(sessionId_);
    packet->SetOpcode(RpcOpCode::RpcRsp);
    packet->SetExtData1(0);
#endif

    packet->Write(*rsp);
    // 回包
    connMgr_->PushPacket(packet);
}
