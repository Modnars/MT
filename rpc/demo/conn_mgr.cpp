#include "conn_mgr.h"
#include "llbc/common/Define.h"
#include "llbc/core/log/LoggerMgr.h"
#include "macros.h"
#include "rpc_channel.h"

ConnComp::ConnComp() : LLBC_Component(LLBC_ComponentEvents::DefaultEvents | LLBC_ComponentEvents::OnUpdate) { }

bool ConnComp::OnInit(bool &initFinished) {
    LLOG_TRACE("Service create!");
    return true;
}

void ConnComp::OnDestroy(bool &destroyFinished) {
    LLOG_TRACE("Service destroy!");
}

void ConnComp::OnSessionCreate(const LLBC_SessionInfo &sessionInfo) {
    LLOG_TRACE("Session Create: %s", sessionInfo.ToString().c_str());
}

void ConnComp::OnSessionDestroy(const LLBC_SessionDestroyInfo &destroyInfo) {
    LLOG_TRACE("Session Destroy, info: %s", destroyInfo.ToString().c_str());
}

void ConnComp::OnAsyncConnResult(const LLBC_AsyncConnResult &result) {
    LLOG_TRACE("Async-Conn result: %s", result.ToString().c_str());
}

void ConnComp::OnUnHandledPacket(const LLBC_Packet &packet) {
    LLOG_TRACE("Unhandled packet, sessionId: %d, opcode: %d, payloadLen: %ld", packet.GetSessionId(),
               packet.GetOpcode(), packet.GetPayloadLength());
}

void ConnComp::OnProtoReport(const LLBC_ProtoReport &report) {
    LLOG_TRACE("Proto report: %s", report.ToString().c_str());
}

void ConnComp::OnUpdate() {
    auto *sendPacket = sendQueue_.Pop();
    while (sendPacket) {
        LLOG_TRACE("sendPacket: %s", sendPacket->ToString().c_str());
        auto ret = GetService()->Send(sendPacket);
        if (ret != LLBC_OK) {
            LLOG_ERROR("Send packet failed, err: %s", LLBC_FormatLastError());
        }

        // LLBC_Recycle(sendPacket);
        sendPacket = sendQueue_.Pop();
    }
}

void ConnComp::OnRecvPacket(LLBC_Packet &packet) {
    LLOG_TRACE("OnRecvPacket: %s", packet.ToString().c_str());
    LLBC_Packet *recvPacket = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    recvPacket->SetHeader(packet, packet.GetOpcode(), 0);
    recvPacket->SetPayload(packet.DetachPayload());
    recvQueue_.Push(recvPacket);
}

int ConnComp::PushPacket(LLBC_Packet *sendPacket) {
    if (sendQueue_.Push(sendPacket))
        return LLBC_OK;

    return LLBC_FAILED;
}

ConnMgr::ConnMgr() {
    // Init();
}

ConnMgr::~ConnMgr() { }

int ConnMgr::Init() {
    // Create service
    svc_ = LLBC_Service::Create("SvcTest");
    comp_ = new ConnComp;
    svc_->AddComponent(comp_);
    svc_->Subscribe(RpcOpCode::RpcReq, comp_, &ConnComp::OnRecvPacket);
    svc_->Subscribe(RpcOpCode::RpcRsp, comp_, &ConnComp::OnRecvPacket);
    svc_->SuppressCoderNotFoundWarning();
    auto ret = svc_->Start(1);
    LLOG_TRACE("Service start, ret: %d", ret);
    return ret;
}

int ConnMgr::StartRpcService(const char *ip, int port) {
    LLOG_TRACE("ConnMgr StartRpcService");
    LLOG_TRACE("Server will listen on %s:%d", ip, port);
    int serverSessionId_ = svc_->Listen(ip, port);
    COND_RET_ELOG(serverSessionId_ == 0, LLBC_FAILED, "Create session failed, reason: %s", LLBC_FormatLastError())

    isServer_ = true;
    return LLBC_OK;
}

// 创建rpc客户端通信通道
RpcChannel *ConnMgr::CreateRpcChannel(const char *ip, int port) {
    LLOG_TRACE("CreateRpcChannel");
    auto sessionId = svc_->Connect(ip, port);
    COND_RET_ELOG(sessionId == 0, nullptr, "Create session failed, reason: %s", LLBC_FormatLastError());

    return new RpcChannel(this, sessionId);
}

int ConnMgr::CloseSession(int sessionId) {
    LLOG_TRACE("CloseSession, %d", sessionId);
    return svc_->RemoveSession(sessionId);
}

bool ConnMgr::Tick() {
    auto ret = false;
    // 读取接收到的数据包并给对应的订阅者处理
    auto packet = PopPacket();
    while (packet) {
        ret = true;
        LLOG_TRACE("Tick");
        auto it = packetDelegs_.find(packet->GetOpcode());
        if (it == packetDelegs_.end())
            LLOG_ERROR("Recv Untapped opcode:%d", packet->GetOpcode());
        else
            (it->second)(*packet);

        // 取下一个包
        LLBC_Recycle(packet);
        packet = PopPacket();
    }

    return ret;
}

int ConnMgr::Subscribe(int cmdId, const LLBC_Delegate<void(LLBC_Packet &)> &deleg) {
    auto pair = packetDelegs_.emplace(cmdId, deleg);
    COND_RET(!pair.second, LLBC_FAILED);
    return LLBC_OK;
}

void ConnMgr::Unsubscribe(int cmdId) {
    packetDelegs_.erase(cmdId);
}