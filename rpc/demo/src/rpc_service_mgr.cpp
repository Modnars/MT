/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-19 20:13:51
 * @edit: regangcli
 * @brief:
 */
#include <cassert>
#include <coroutine>

#include <fmt/color.h>
#include <llbc.h>

#include <mt/runner.h>
#include <mt/task.h>

#include "common/error_code.pb.h"
#include "conn_mgr.h"
#include "google/protobuf/message.h"
#include "macros.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_service_mgr.h"

using namespace llbc;

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

bool RpcServiceMgr::AddService(::google::protobuf::Service *service) {
    // TODO modnarshen 改用一个从 service 获取 option 的方式来注册 cmd
    static std::uint32_t cmd = 0U;

    const auto *service_desc = service->GetDescriptor();
    for (int i = 0; i < service_desc->method_count(); ++i) {
        bool succ = services_.insert({++cmd, {service, service_desc->method(i)}}).second;
        COND_RET_ELOG(!succ, false, "add service failed|cmd:%u|service:%s", cmd,
                      service_desc->method(i)->full_name().c_str());
    }
    return true;
}

bool RpcServiceMgr::RegisterChannel(const char *ip, int32_t port) {
    auto *channel = connMgr_->CreateRpcChannel(ip, port);
    COND_RET_ELOG(!channel, false, "create rpc channel failed");
    channels_.emplace_back(channel);
    return true;
}

mt::Task<int> RpcServiceMgr::Rpc(std::uint32_t cmd, std::uint64_t uid, const ::google::protobuf::Message &req,
                                 ::google::protobuf::Message *rsp) {
    LLOG_TRACE("call rpc|uid:%lu|req: %s", uid, req.ShortDebugString().c_str());
    assert(!channels_.empty());

    std::uint64_t seq_id = RpcCoroMgr::GetInst().NewCoroUid();
    PkgHead pkg_head{.src = 0UL, .dst = 0UL, .uid = uid, .seq = seq_id, .cmd = cmd};
    auto *channel = channels_[uid % channels_.size()];
    channel->Send(pkg_head, req);

    COND_EXP(rsp, co_return co_await channel->AwaitResponse(rsp));
    co_return 0;
}

void RpcServiceMgr::HandleRpcReq(LLBC_Packet &packet) {
#if ENABLE_CXX20_COROUTINE
    int ret = mt::run(DealRequest(packet));
    COND_RET_ELOG(ret != 0, , "deal request failed|ret:%d", ret);
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
    COND_RET(pkg_head.FromPacket(packet) != 0);
    sessionId_ = packet.GetSessionId();

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
    mt::run(RpcCoroMgr::GetInst().Pop(static_cast<RpcCoroMgr::coro_uid_type>(pkg_head.seq)));
#endif
}

mt::Task<int> RpcServiceMgr::DealRequest(llbc::LLBC_Packet packet) {
    PkgHead pkg_head;
    int ret = pkg_head.FromPacket(packet) != 0;
    CO_COND_RET_ELOG(ret != 0, ret, "pkg_head.FromPacket failed|ret:%d", ret);
    sessionId_ = packet.GetSessionId();

    auto iter = services_.find(pkg_head.cmd);
    CO_COND_RET_ELOG(iter == services_.end(), -1, "service and method not found|cmd:%08X", pkg_head.cmd);
    auto *service = iter->second.first;
    const auto *method = iter->second.second;
    // 解析 req & 创建 rsp
    auto *req = service->GetRequestPrototype(method).New();
    auto *rsp = service->GetResponsePrototype(method).New();
    LLOG_TRACE("packet: %s", packet.ToString().c_str());
    ret = packet.Read(*req);
    CO_COND_RET_ELOG(ret != LLBC_OK, ret, "read req failed|ret:%d|reason: %s", ret, LLBC_FormatLastError());

    // TODO modnarshen 这里的 done 传递的是引用，可能要确定下回调执行时，引用的实例是否还存在
    // auto done = ::google::protobuf::NewCallback<RpcServiceMgr, const PkgHead &, const ::google::protobuf::Message &>(
    //     &RpcServiceMgr::GetInst(), &RpcServiceMgr::OnRpcDone, pkg_head, *rsp);
    service->CallMethod(method, &RpcController::GetInst(), req, rsp, nullptr);
    OnRpcDone(pkg_head, *rsp);

    co_return 0;
}

void RpcServiceMgr::OnRpcDone(const PkgHead &pkg_head, const ::google::protobuf::Message &rsp) {
    auto *packet = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    COND_RET_ELOG(!packet, , "alloc packet failed|pkg_head: %s|rsp: %s", pkg_head.ToString().c_str(),
                  rsp.ShortDebugString().c_str());

    packet->SetOpcode(RpcOpCode::RpcRsp);
    packet->SetSessionId(sessionId_);

    int ret = pkg_head.ToPacket(*packet);
    COND_RET_ELOG(ret != 0, , "pkg_head.ToPacket failed|ret:%d", ret);

    ret = packet->Write(rsp);
    COND_RET_ELOG(ret != 0, , "packet.Write failed|ret:%d", ret);

    connMgr_->PushPacket(packet);
}
