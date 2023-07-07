#include <csignal>
#include <iostream>

#include <fmt/core.h>
#include <llbc.h>
#include <mt/runner.h>

#include "common/demo_service.pb.h"
#include "conn_mgr.h"
#include "macros.h"
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
    COND_RET_ELOG(ret != LLBC_OK, -1, "init logger failed|error: %s", LLBC_FormatLastError());

    // 初始化连接管理器
    ret = ConnMgr::GetInst().Init();
    COND_RET_ELOG(ret != LLBC_OK, -1, "init ConnMgr failed|error: %s", LLBC_FormatLastError());

    // 创建 rpc channel
    RpcChannel *channel = ConnMgr::GetInst().CreateRpcChannel("127.0.0.1", 6688);
    LLBC_Defer(delete channel);

    COND_RET_ELOG(!channel, -1, "CreateRpcChannel failed");
    LLOG_TRACE("CLIENT START!");

    // 创建 rpc req & resp
    protocol::EchoReq req;
    protocol::EchoRsp rsp;
    req.set_msg("hello, myrpc.");

    // 创建 rpc controller & stub
    protocol::DemoService_Stub stub{channel};

#if ENABLE_CXX20_COROUTINE
    // 协程方案, 在新协程中 call rpc
    ret = RpcServiceMgr::GetInst().Init(&ConnMgr::GetInst());
    COND_RET_ELOG(ret != 0, ret, "RpcServiceMgr init failed|ret:%d", ret);
    while (!stop) {
        std::string input;
        std::cin >> input;  // 手动阻塞
        if (input != "\n")
            req.set_msg(input.c_str());
        // 创建协程并 Resume
        auto func = [&stub, &req, &rsp]() -> mt::Task<> {
            stub.Echo(&RpcController::GetInst(), &req, &rsp, nullptr);
            LLOG_INFO("received rsp: %s", rsp.msg().c_str());
            co_return;
        };
        mt::run(func());

        // 处理服务收到的数据包，若有Rpc Rsp，OnUpdate内部会唤醒对应休眠的协程
        // connMgr->Tick();
    }

#else
    // 直接调用方案
    while (!stop) {
        std::string input;
        std::cin >> input;  // 手动阻塞
        if (input != "\n")
            req.set_msg(input.c_str());
        stub.Echo(&RpcController::GetInst(), &req, &rsp, nullptr);
        LLOG_TRACE("RECEIVED RSP: %s", rsp.msg().c_str());
        // LLBC_Sleep(1000);
    }
#endif

    LLOG_TRACE("client stop");

    return 0;
}
