/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-15 20:21:17
 * @edit: regangcli
 * @brief:
 */

#pragma once

#include <cstdint>
#include <cstdio>
#include <map>
#include <string>

#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <llbc.h>
#include <mt/task.h>
#include <mt/util/singleton.h>

class ConnMgr;

static constexpr std::size_t MAX_BUFFER_SIZE = 1024UL;

struct PacketHead {
    std::string service = "";
    std::string method = "";
    std::uint64_t coro_uid = 0UL;
};

// LLBC_Packet:
//
//   0                         32                        64
//   +-------------------------+-------------------------+
//   |           dst           |           cmd           |
//   +-------------------------+-------------------------+
//   |                        uid                        |
//   +---------------------------------------------------+
//   |                        seq                        |
//   +---------------------------------------------------+
//   |                    body(message)                  |
//   +---------------------------------------------------+
//
struct PkgHead {
public:
    std::uint32_t src = 0U;
    std::uint32_t dst = 0U;
    std::uint64_t uid = 0UL;
    std::uint64_t seq = 0UL;
    std::uint32_t cmd = 0U;
    std::int8_t flag = 0;
    std::int8_t type = 0;

    int FromPacket(llbc::LLBC_Packet &packet);
    int ToPacket(llbc::LLBC_Packet &packet) const;
    const std::string &ToString() const {
        static std::string buffer(MAX_BUFFER_SIZE, '\0');
        ::snprintf(buffer.data(), MAX_BUFFER_SIZE, "src:%u|dst:%u|uid:%lu|seq:%lu|cmd:0x%08X", src, dst, uid, seq, cmd);
        return buffer;
    }
};

class RpcController : public ::google::protobuf::RpcController, public mt::Singleton<RpcController> {
public:
    RpcController() = default;
    ~RpcController() { }
    virtual void Reset() { }
    virtual bool Failed() const { return false; }
    virtual std::string ErrorText() const { return ""; }
    virtual void StartCancel() { }
    virtual void SetFailed(const std::string & /* reason */) { }
    virtual bool IsCanceled() const { return false; }
    virtual void NotifyOnCancel(::google::protobuf::Closure * /* callback */) { }
};

class RpcChannel : public ::google::protobuf::RpcChannel {
public:
    RpcChannel(ConnMgr *connMgr, int sessionId) : connMgr_(connMgr), sessionId_(sessionId) { }
    virtual ~RpcChannel();

    void CallMethod(const ::google::protobuf::MethodDescriptor *method, ::google::protobuf::RpcController *controller,
                    const ::google::protobuf::Message *request, ::google::protobuf::Message *response,
                    ::google::protobuf::Closure *done) override;

    // 发送数据（暂时先不将其搞成协程）
    int Send(const PkgHead &pkg_head, const ::google::protobuf::Message &message);

public:  // TODO modnarshen 临时搞两个收包接口用来保证流程畅通
    int BlockingWaitResponse(::google::protobuf::Message *response);
    mt::Task<int> AwaitResponse(::google::protobuf::Message *response);

private:
    ConnMgr *connMgr_ = nullptr;
    int sessionId_ = 0;
};
