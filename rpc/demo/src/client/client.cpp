#include <csignal>
#include <iostream>

#include <fmt/core.h>
#include "common/demo.pb.h"
#include "llbc.h"

#include "common/demo_service.pb.h"
#include "conn_mgr.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_service_mgr.h"

using namespace llbc;

bool stop = false;

void signalHandler(int signum) {
    fmt::print("INTERRUPT SIGNAL [{}] RECEIVED.\n", signum);
    stop = true;
}

int main() {
    // 注册信号 SIGINT 和信号处理程序
    signal(SIGINT, signalHandler);

    // 初始化 llbc 库
    LLBC_Startup();
    LLBC_Defer(LLBC_Cleanup());

    // 初始化日志
    auto ret = LLBC_LoggerMgrSingleton->Initialize("log/cfg/server_log.cfg");
    if (ret == LLBC_FAILED) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Initialize logger failed, error:%s", LLBC_FormatLastError());
        return -1;
    }

    // 初始化连接管理器
    ConnMgr *connMgr = new ConnMgr();
    ret = connMgr->Init();
    if (ret != LLBC_OK) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Initialize connMgr failed, error:%s", LLBC_FormatLastError());
        return -1;
    }

    // 创建 rpc channel
    RpcChannel *channel = connMgr->CreateRpcChannel("127.0.0.1", 6688);
    LLBC_Defer(delete channel);

    if (!channel) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "CreateRpcChannel Fail");
        return -1;
    }

    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Client Start!");

    // 创建 rpc req & resp
    protocol::EchoReq req;
    protocol::EchoRsp rsp;
    req.set_msg("hello, myrpc.");

    // 创建 rpc controller & stub
    protocol::DemoService_Stub stub(channel);

#ifdef UseCoroRpc
    // 协程方案, 在新协程中call rpc
    RpcServiceMgr serviceMgr(connMgr);
    while (!stop) {
        // 创建协程并Resume
        auto func = [&stub, &cntl, &channel, &req, &rsp](void *) { stub.Echo(&cntl, &req, &rsp, nullptr); };
        auto coro = g_rpcCoroMgr->CreateCoro(func, nullptr, "");
        coro->Resume();

        // 处理服务收到的数据包，若有Rpc Rsp，OnUpdate内部会唤醒对应休眠的协程
        connMgr->Tick();
    }

#else
    // 直接调用方案
    while (!stop) {
        std::string input;
        std::cin >> input;  // 手动阻塞
        if (input != "\n")
            req.set_msg(input.c_str());
        stub.Echo(&RpcController::GetInst(), &req, &rsp, nullptr);
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "RECEIVED RSP: %s", rsp.msg().c_str());
        // LLBC_Sleep(1000);
    }
#endif

    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "client Stop");

    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
