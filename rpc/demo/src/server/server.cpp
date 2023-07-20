#include <csignal>
#include <cstdlib>

#include <fmt/core.h>
#include <llbc.h>
#include <mt/runner.h>

#include "conn_mgr.h"
#include "demo_service_impl.h"
#include "macros.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_server.h"
#include "rpc_service_mgr.h"

using namespace llbc;

void signalHandler(int signum) {
    fmt::print("INTERRUPT SIGNAL [{}] RECEIVED.\n", signum);
    RpcServer::GetInst().Stop();
}

int main(int argc, char *argv[]) {
    // 注册信号 SIGINT 和信号处理程序
    signal(SIGINT, signalHandler);

    // 初始化 llbc 库
    LLBC_Startup();
    // LLBC_HookProcessCrash();
    LLBC_Defer(LLBC_Cleanup());

    // 初始化日志
    auto ret = LLBC_LoggerMgrSingleton->Initialize("log/cfg/server_log.cfg");
    if (ret == LLBC_FAILED) {
        fmt::print("Initialize logger failed|error: %s\n", LLBC_FormatLastError());
        return -1;
    }
    LLOG_TRACE("Hello Server!");

    ret = ConnMgr::GetInst().Init();
    COND_RET_ELOG(ret != 0, ret, "ConnMgr init failed|ret:%d", ret);

    // 启动 rpc 服务
    COND_RET_ELOG(ConnMgr::GetInst().StartRpcService("127.0.0.1", 6688) != LLBC_OK, -1,
                  "ConnMgr StartRpcService failed");

    ret = RpcServiceMgr::GetInst().Init(&ConnMgr::GetInst());
    COND_RET_ELOG(ret != 0, ret, "RpcServiceMgr init failed|ret:%d", ret);
    bool succ = RpcServiceMgr::GetInst().AddService(new DemoServiceImpl);
    COND_RET_ELOG(!succ, EXIT_FAILURE, "add service failed");

#define REGISTER_RPC_CHANNEL(ip, port)                                                          \
    {                                                                                           \
        bool _succ = RpcServiceMgr::GetInst().RegisterChannel(ip, port);                        \
        COND_RET_ELOG(!_succ, EXIT_FAILURE, "register channel failed|ip:%s|port:%d", ip, port); \
        LLOG_INFO("register channel succ|ip:%s|port:%d", ip, port);                             \
    }

    REGISTER_RPC_CHANNEL("127.0.0.1", 6688);
    REGISTER_RPC_CHANNEL("127.0.0.1", 6699);

#undef REGISTER_RPC_CHANNEL

    ret = RpcServer::GetInst().Init();
    COND_RET_ELOG(ret != 0, ret, "RpcServer init failed|ret:%d", ret);

    RpcCoroMgr::GetInst().UseCoro(true);  // 服务端启用协程来处理请求
    mt::run(RpcServer::GetInst().serve());

    return 0;
}
