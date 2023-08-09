#include <cassert>
#include <coroutine>

#include <fmt/color.h>
#include <llbc.h>

#include <mt/call_stack.h>
#include <mt/runner.h>
#include <mt/task.h>

#include "conn_mgr.h"
#include "error_code.pb.h"
#include "google/protobuf/message.h"
#include "macros.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_service_mgr.h"

using namespace llbc;

int RpcServiceMgr::Init(ConnMgr *conn_mgr) {
    COND_RET_ELOG(conn_mgr_ != nullptr, -1, "ConnMgr has already been registered|address:%p", conn_mgr_);
    conn_mgr_ = conn_mgr;
    if (conn_mgr_) [[likely]] {
        conn_mgr_->Subscribe(RpcOpCode::RpcReq, LLBC_Delegate<void(LLBC_Packet &)>(this, &RpcServiceMgr::HandleRpcReq));
        conn_mgr_->Subscribe(RpcOpCode::RpcRsp, LLBC_Delegate<void(LLBC_Packet &)>(this, &RpcServiceMgr::HandleRpcRsp));
    }
    return 0;
}

RpcServiceMgr::~RpcServiceMgr() {
    if (conn_mgr_) [[likely]] {
        conn_mgr_->Unsubscribe(RpcOpCode::RpcReq);
        conn_mgr_->Unsubscribe(RpcOpCode::RpcRsp);
    }
}

bool RpcServiceMgr::RegisterChannel(const char *ip, int32_t port) {
    auto *channel = conn_mgr_->CreateRpcChannel(ip, port);
    COND_RET_ELOG(!channel, false, "create rpc channel failed");
    channels_.emplace_back(channel);
    return true;
}

mt::Task<int> RpcServiceMgr::Rpc(std::uint32_t cmd, std::uint64_t uid, const ::google::protobuf::Message &req,
                                 ::google::protobuf::Message *rsp, std::uint32_t timeout) {
    LLOG_TRACE("call rpc|uid:%lu|req: %s", uid, req.ShortDebugString().c_str());
    assert(!channels_.empty());

    std::uint64_t seq_id = RpcCoroMgr::GetInst().NewCoroUid();
    PkgHead pkg_head{.src = 0UL, .dst = 0UL, .uid = uid, .seq = seq_id, .cmd = cmd};
    auto *channel = channels_[uid % channels_.size()];
    channel->Send(pkg_head, req);

    if (rsp) {
        COND_EXP(seq_id == 0UL, channel->BlockingWaitResponse(rsp); co_return 0);  // 针对 client 先阻塞
        RpcCoroMgr::context context{.session_id = session_id_, .rsp = rsp};
        co_await mt::dump_call_stack();
        co_await MainCoroAwaiter{seq_id, context};
        LLOG_INFO("RESUME TO CURRENT COROUTINE");
        co_await mt::dump_call_stack();
    }
    co_return 0;
}

void RpcServiceMgr::HandleRpcReq(LLBC_Packet &packet) {
    mt::sync_wait(DealRequest(packet));
}

void RpcServiceMgr::HandleRpcRsp(LLBC_Packet &packet) {
    mt::sync_wait(DealResponse(packet));
}

mt::Task<int> RpcServiceMgr::DealRequest(llbc::LLBC_Packet packet) {
    PkgHead pkg_head;
    int ret = pkg_head.FromPacket(packet);
    CO_COND_RET_ELOG(ret != 0, ret, "pkg_head.FromPacket failed|ret:%d", ret);
    session_id_ = packet.GetSessionId();

    auto iter = service_methods_.find(pkg_head.cmd);
    CO_COND_RET_ELOG(iter == service_methods_.end(), -1, "service and method not found|cmd:%08X", pkg_head.cmd);
    auto *service = iter->second.service;
    const auto *method = iter->second.method;
    // 解析 req & 创建 rsp
    auto *req = service->GetRequestPrototype(method).New();
    auto *rsp = service->GetResponsePrototype(method).New();
    LLOG_TRACE("packet: %s", packet.ToString().c_str());
    ret = packet.Read(*req);
    CO_COND_RET_ELOG(ret != LLBC_OK, ret, "read req failed|ret:%d|reason: %s", ret, LLBC_FormatLastError());

    // TODO modnarshen 这里的 done 传递的是引用，可能要确定下回调执行时，引用的实例是否还存在
    // auto done = ::google::protobuf::NewCallback<RpcServiceMgr, const PkgHead &, const ::google::protobuf::Message &>(
    //     &RpcServiceMgr::GetInst(), &RpcServiceMgr::OnRpcDone, pkg_head, *rsp);

    assert(iter->second.co_func);
    ret = co_await iter->second.co_func(method, &RpcController::GetInst(), *req, *rsp, nullptr);
    // service->CallMethod(method, &RpcController::GetInst(), req, rsp, nullptr);
    CO_COND_RET_ELOG(ret != 0, ret, "call method failed|ret:%d", ret);
    OnRpcDone(pkg_head, *rsp);

    co_return 0;
}

mt::Task<int> RpcServiceMgr::DealResponse(llbc::LLBC_Packet packet) {
    PkgHead pkg_head;
    int ret = pkg_head.FromPacket(packet);
    CO_COND_RET_ELOG(ret != 0, ret, "pkg_head.FromPacket failed|ret:%d", ret);
    LLOG_DEBUG("pkg_head info|%s", pkg_head.ToString().c_str());

    auto coro_uid = static_cast<RpcCoroMgr::coro_uid_type>(pkg_head.seq);
    auto ctx = RpcCoroMgr::GetInst().Pop(coro_uid);
    CO_COND_RET_ELOG(ctx.handle == nullptr || ctx.rsp == nullptr, protocol::ErrorCode::FAILURE,
                     "coro context not found|cmd:0x%08X|seq_id:%lu", pkg_head.cmd, coro_uid);
    // 解析 rsp
    ret = packet.Read(*ctx.rsp);
    CO_COND_RET_ELOG(ret != LLBC_OK, ret, "read rsp failed|ret:%d", ret);
    session_id_ = ctx.session_id;
    LLOG_INFO("received rsp|address:%p|info: %s|sesson_id:%d", ctx.rsp, ctx.rsp->DebugString().c_str(), session_id_);

    ctx.handle.resume();
    LLOG_INFO("coro is done|%u", ctx.handle.done());
    co_return 0;
}

void RpcServiceMgr::OnRpcDone(const PkgHead &pkg_head, const ::google::protobuf::Message &rsp) {
    auto *packet = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    COND_RET_ELOG(!packet, , "alloc packet failed|pkg_head: %s|rsp: %s", pkg_head.ToString().c_str(),
                  rsp.ShortDebugString().c_str());

    packet->SetOpcode(RpcOpCode::RpcRsp);
    packet->SetSessionId(session_id_);

    int ret = pkg_head.ToPacket(*packet);
    COND_RET_ELOG(ret != 0, , "pkg_head.ToPacket failed|ret:%d", ret);

    ret = packet->Write(rsp);
    COND_RET_ELOG(ret != 0, , "packet.Write failed|ret:%d", ret);

    conn_mgr_->PushPacket(packet);
}
