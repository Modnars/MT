/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-19 20:13:51
 * @edit: regangcli
 * @brief:
 */
#include <coroutine>

#include <fmt/color.h>
#include <llbc.h>

#include <mt/runner.h>
#include <mt/task.h>

#include "common/error_code.pb.h"
#include "conn_mgr.h"
#include "macros.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_service_mgr.h"

using namespace llbc;

namespace util {

int32_t ParseNetPacket(llbc::LLBC_Packet &packet, PkgHead &pkg_head) {
    int ret = packet.Read(pkg_head.service);
    COND_RET_ELOG(ret != LLBC_OK, ret, "read service_name failed|ret:%d", ret);
    ret = packet.Read(pkg_head.method);
    COND_RET_ELOG(ret != LLBC_OK, ret, "read method_name failed|ret:%d", ret);
    ret = packet.Read(pkg_head.coro_uid);
    COND_RET_ELOG(ret != LLBC_OK, ret, "read coro_uid failed|ret:%d", ret);
    LLOG_INFO("parse net packet done|service:%s|mthod:%s|coro_uid:%lu|sesson_id:%d", pkg_head.service.c_str(),
              pkg_head.method.c_str(), pkg_head.coro_uid, packet.GetSessionId());
    return protocol::ErrorCode::SUCCESS;
}

}  // namespace util

int RpcServiceMgr::Init(ConnMgr *conn_mgr) {
    COND_RET_ELOG(connMgr_ != nullptr, -1, "ConnMgr has already been registered|address:%p", connMgr_);
    connMgr_ = conn_mgr;
    if (connMgr_) [[likely]] {
        connMgr_->Subscribe(RpcOpCode::RpcReq, LLBC_Delegate<void(LLBC_Packet &)>(this, &RpcServiceMgr::HandleRpcReq));
        connMgr_->Subscribe(RpcOpCode::RpcRsp, LLBC_Delegate<void(LLBC_Packet &)>(this, &RpcServiceMgr::HandleRpcRsp));
    }
    return 0;
}

RpcServiceMgr::~RpcServiceMgr() {
    if (connMgr_) [[likely]] {
        connMgr_->Unsubscribe(RpcOpCode::RpcReq);
        connMgr_->Unsubscribe(RpcOpCode::RpcRsp);
    }
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
    LLOG_TRACE("HandleRpcReq");
    PkgHead pkg_head;
    COND_RET(util::ParseNetPacket(packet, pkg_head) != 0);
    sessionId_ = packet.GetSessionId();

    auto *service = _services[pkg_head.service].service;
    auto *method = _services[pkg_head.service].mds[pkg_head.method];
    // 解析 req & 创建 rsp
    auto *req = service->GetRequestPrototype(method).New();
    auto *rsp = service->GetResponsePrototype(method).New();
    auto ret = packet.Read(*req);
    COND_RET_ELOG(ret != LLBC_OK, , "read req failed|ret:%d", ret);

#if ENABLE_CXX20_COROUTINE
    auto func = [service, method, req, rsp, pkg_head, this]() -> mt::Task<> {
        LLOG_INFO("[HANDLE_RPC_REQ] COROUTINE CALL FROM C++20");
        service->CallMethod(method, &RpcController::GetInst(), req, rsp, nullptr);
        // co_await std::suspend_always{};  // 这里直接挂起
        LLOG_INFO("[HANDLE_RPC_REQ] COROUTINE CALL FROM C++20|RESUME");
        OnRpcDone(req, rsp, method, pkg_head.coro_uid);
        co_return;
    };
    auto tt = func();
    mt::run(tt);
    RpcCoroMgr::GetInst().Suspend(std::move(tt));
#else
    // 直接调用方案
    // 创建 rpc 完成回调函数
    service->CallMethod(method, &RpcController::GetInst(), req, rsp, nullptr);
    OnRpcDone(req, rsp, method, task_id);
#endif
}

void RpcServiceMgr::HandleRpcRsp(LLBC_Packet &packet) {
    LLOG_TRACE("HandleRpcRsp");
    PkgHead pkg_head;
    COND_RET(util::ParseNetPacket(packet, pkg_head) != 0);
    sessionId_ = packet.GetSessionId();

    auto *service = _services[pkg_head.service].service;
    auto *method = _services[pkg_head.service].mds[pkg_head.method];
    // 解析 rsp
    auto *rsp = RpcController::GetInst().GetRsp();
    // auto *rsp = service->GetResponsePrototype(method).New();
    // auto *rsp = ::google::protobuf::down_cast<decltype(service->GetResponsePrototype(method).New())>(
    //     RpcController::GetInst().GetRsp());
    auto ret = packet.Read(*rsp);
    COND_RET_ELOG(ret != LLBC_OK, , "read rsp failed|ret:%d", ret);

    LLOG_INFO("received rsp|address:%p|info: %s", rsp, rsp->DebugString().c_str());

#if ENABLE_CXX20_COROUTINE
    // 拿出 coro_uid 直接唤醒执行
    mt::run(RpcCoroMgr::GetInst().Pop(static_cast<RpcCoroMgr::coro_uid_type>(pkg_head.coro_uid)));
#endif
}

void RpcServiceMgr::OnRpcDone(::google::protobuf::Message *req, ::google::protobuf::Message *rsp,
                              const ::google::protobuf::MethodDescriptor *method, uint64_t task_id) {
    LLOG_TRACE("OnRpcDone|req: %s|rsp: %s|task_id:%lu", req->ShortDebugString().c_str(),
               rsp->ShortDebugString().c_str(), task_id);

    auto *packet = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    COND_RET_ELOG(!packet, , "alloc packet failed|req: %s|rsp: %s", req->ShortDebugString().c_str(),
                  rsp->ShortDebugString().c_str());

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
    packet->Write(uint64_t(555));
#endif

    packet->Write(*rsp);
    // 回包
    connMgr_->PushPacket(packet);
}
