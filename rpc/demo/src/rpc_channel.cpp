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

#include "common/demo.pb.h"
#include "common/error_code.pb.h"
#include "conn_mgr.h"
#include "macros.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_service_mgr.h"

using namespace llbc;

int PkgHead::FromPacket(llbc::LLBC_Packet &packet) {
    int ret = packet.Read(this->dst);
    COND_RET_ELOG(ret != LLBC_OK, ret, "read pkg_head.dst failed|ret:%d", ret);
    ret = packet.Read(this->cmd);
    COND_RET_ELOG(ret != LLBC_OK, ret, "read pkg_head.cmd failed|ret:%d", ret);
    ret = packet.Read(this->uid);
    COND_RET_ELOG(ret != LLBC_OK, ret, "read pkg_head.uid failed|ret:%d", ret);
    ret = packet.Read(this->seq);
    COND_RET_ELOG(ret != LLBC_OK, ret, "read pkg_head.seq failed|ret:%d", ret);

    LLOG_TRACE("read net packet done|dst:%u|cmd:%u|uid:%lu|seq:%lu|session_id:%d", this->dst, this->cmd, this->uid,
               this->seq, packet.GetSessionId());
    return 0;
}

int PkgHead::ToPacket(llbc::LLBC_Packet &packet) const {
    int ret = packet.Write(static_cast<std::uint32_t>(this->dst));
    COND_RET_ELOG(ret != LLBC_OK, ret, "write pkg_head.dst failed|ret:%d", ret);
    ret = packet.Write(static_cast<std::uint32_t>(this->cmd));
    COND_RET_ELOG(ret != LLBC_OK, ret, "write pkg_head.cmd failed|ret:%d", ret);
    ret = packet.Write(static_cast<std::uint64_t>(this->uid));
    COND_RET_ELOG(ret != LLBC_OK, ret, "write pkg_head.uid failed|ret:%d", ret);
    ret = packet.Write(static_cast<std::uint64_t>(this->seq));
    COND_RET_ELOG(ret != LLBC_OK, ret, "write pkg_head.seq failed|ret:%d", ret);

    LLOG_TRACE("write net packet done|dst:%u|cmd:%u|uid:%lu|seq:%lu|sessond_id:%d", this->dst, this->cmd, this->uid,
               this->seq, packet.GetSessionId());
    return 0;
}

RpcChannel::~RpcChannel() {
    connMgr_->CloseSession(sessionId_);
}

int RpcChannel::BlockingWaitResponse(::google::protobuf::Message *response) {
    auto recv_packet = connMgr_->PopPacket();
    int count = 0;
    while (!recv_packet && count < 100) {
        LLBC_Sleep(1);
        ++count;
        recv_packet = connMgr_->PopPacket();
    }

    if (!recv_packet) {
        response->Clear();
        LLOG_ERROR("receive packet timeout!");
        return protocol::ErrorCode::RECV_PKG_TIME_OUT;
    }

    LLOG_TRACE("Payload info|length:%lu|info: %s", recv_packet->GetPayloadLength(), recv_packet->ToString().c_str());
    PkgHead pkg_head;
    auto ret = pkg_head.FromPacket(*recv_packet);
    COND_RET_ELOG(ret != 0, ret, "parse net packet failed|ret:%d", ret);
    ret = recv_packet->Read(*response);
    COND_RET_ELOG(ret != 0, protocol::ErrorCode::PARSE_PKG_FAILED, "read recv_packet failed|ret:%d", ret);

    LLBC_Recycle(recv_packet);
    LLOG_TRACE("recved: %s|extdata:%lu", response->ShortDebugString().c_str(), pkg_head.seq);
    return 0;
}

// 先预留个接口在这里，内部先用阻塞的方式搞一下
mt::Task<int> RpcChannel::AwaitResponse(::google::protobuf::Message *response) {
    LLOG_TRACE("[AwaitResponse] COROUTINE START");
    auto ret = BlockingWaitResponse(response);
    CO_COND_RET_ELOG(ret != 0, ret, "await response failed|ret:%d", ret);
    LLOG_TRACE("[AwaitResponse] COROUTINE RESUME");
    LLOG_TRACE("[AwaitResponse] COROUTINE RETURN");
    co_return 0;
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
    std::uint64_t coro_uid = RpcCoroMgr::GetInst().NewCoroUid();
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

int RpcChannel::Send(const PkgHead &pkg_head, const ::google::protobuf::Message &message) {
    LLBC_Packet *packet = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    packet->SetHeader(0, RpcOpCode::RpcReq, 0);
    packet->SetSessionId(sessionId_);

    int ret = pkg_head.ToPacket(*packet);
    COND_RET_ELOG(ret != 0, ret, "pkg_head.ToPacket failed|ret:%d", ret);
    ret = packet->Write(message);
    COND_RET_ELOG(ret != 0, ret, "packet.Write message failed|ret:%d", ret);
    LLOG_DEBUG("send data|message: %s|packet: %s", message.ShortDebugString().c_str(), packet->ToString().c_str());

    connMgr_->PushPacket(packet);
    return 0;
}
