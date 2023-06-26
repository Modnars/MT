/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-19 20:13:51
 * @edit: regangcli
 * @brief:
 */

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

int32_t RpcServiceMgr::PreHandlePacket(LLBC_Packet &packet, std::string &serviceName, std::string &methodName,
                                       uint64_t &task_id) {
    if (packet.Read(serviceName) != LLBC_OK || packet.Read(methodName) != LLBC_OK || packet.Read(task_id) != LLBC_OK) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "read packet failed");
        return -1;
    }
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "recv service_name: %s", serviceName.c_str());
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "recv method_name: %s", methodName.c_str());
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "get task id: %lu", task_id);
    sessionId_ = packet.GetSessionId();
    return 0;
}

void RpcServiceMgr::HandleRpcReq(LLBC_Packet &packet) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "HandleRpcReq");
    std::string serviceName, methodName;
    uint64_t task_id;
    COND_RET(PreHandlePacket(packet, serviceName, methodName, task_id) != 0);

    auto *service = _services[serviceName].service;
    auto *method = _services[serviceName].mds[methodName];
    // 解析 req & 创建 rsp
    auto *req = service->GetRequestPrototype(method).New();
    auto *rsp = service->GetResponsePrototype(method).New();
    auto ret = packet.Read(*req);
    if (ret != LLBC_OK) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "Read req failed, ret: %d", ret);
        return;
    }

#if ENABLE_CXX20_COROUTINE
    auto func = [service, method, req, rsp, task_id, this]() -> mt::Task<> {
        fmt::print(fg(fmt::color::floral_white) | bg(fmt::color::slate_gray) | fmt::emphasis::underline,
                   "[HANDLE_RPC_REQ] THIS IS A COROUTINE CALL FROM C++20\n");
        service->CallMethod(method, &RpcController::GetInst(), req, rsp, nullptr);
        OnRpcDone(req, rsp, method, task_id);
        co_return;
    };
    mt::run(func());
#else
    // 直接调用方案
    // 创建 rpc 完成回调函数
    service->CallMethod(method, &RpcController::GetInst(), req, rsp, nullptr);
    OnRpcDone(req, rsp, method, task_id);
#endif
}

void RpcServiceMgr::HandleRpcRsp(LLBC_Packet &packet) {
#if ENABLE_CXX20_COROUTINE
    // fmt::print(fg(fmt::color::floral_white) | bg(fmt::color::slate_gray) | fmt::emphasis::underline,
    //            "[HANDLE_RPC_RSP] THIS IS A COROUTINE CALL FROM C++20\n");
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "HandleRpcRsp");
    std::string serviceName, methodName;
    uint64_t task_id;
    COND_RET(PreHandlePacket(packet, serviceName, methodName, task_id) != 0);

    auto *service = _services[serviceName].service;
    auto *method = _services[serviceName].mds[methodName];
    // 解析 rsp
    // auto* rsp = RpcController::GetInst().GetRsp();
    auto* rsp = service->GetResponsePrototype(method).New();
    auto ret = packet.Read(*rsp);
    if (ret != LLBC_OK) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "Read rsp failed, ret: %d", ret);
        return;
    }
    RpcController::GetInst().SetRsp(std::unique_ptr<::google::protobuf::Message>(rsp));
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Recv rsp: %s", rsp->DebugString().c_str());

    // 唤醒 task
    auto it = id_to_task_map_.find(task_id);
    if (it != id_to_task_map_.end()) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace,
             "[HANDLE_RPC_RSP] THIS IS A COROUTINE CALL FROM C++20, task resume: %lu | %d", task_id, &it->second);
        // task->schedule();
        mt::run(it->second);
    }
#endif
}

void RpcServiceMgr::OnRpcDone(::google::protobuf::Message *req, ::google::protobuf::Message *rsp,
                              const ::google::protobuf::MethodDescriptor *method, uint64_t task_id) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "OnRpcDone|req: %s|rsp: %s|task id: %lu", req->DebugString().c_str(),
         rsp->DebugString().c_str(), task_id);
    auto *packet = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    if (!packet) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "alloc packet failed|req: %s|rsp: %s", req->DebugString().c_str(),
             rsp->DebugString().c_str());
        return;
    }
    packet->SetOpcode(RpcOpCode::RpcRsp);
    packet->SetSessionId(sessionId_);
    packet->Write(method->service()->name());
    packet->Write(method->name());
#if ENABLE_CXX20_COROUTINE
    // 协程方案
    // auto coro = g_rpcCoroMgr->GetCurCoro();
    // auto sessionId = coro->GetParam1();
    // auto srcCoroId = coro->GetParam2();
    packet->Write(task_id);
#else
    // 直接调用方案
    packet->Write(uint64(555));
#endif

    packet->Write(*rsp);
    // 回包
    connMgr_->PushPacket(packet);
}
