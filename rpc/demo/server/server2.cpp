#include <csignal>
#include <cstdlib>

#include <fmt/core.h>
#include <llbc.h>

#include "conn_mgr.h"
#include "demo_service.pb.h"
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
    auto ret = LLBC_LoggerMgrSingleton->Initialize(SERVER_LLOG_CONF_PATH);
    if (ret == LLBC_FAILED) {
        fmt::print("Initialize logger failed|path:{}|error: {}\n", SERVER_LLOG_CONF_PATH, LLBC_FormatLastError());
        return -1;
    }
    LLOG_TRACE("Hello Server!");

    ret = ConnMgr::GetInst().Init();
    COND_RET_ELOG(ret != 0, ret, "ConnMgr init failed|ret:%d", ret);

    // 启动 rpc 服务
    COND_RET_ELOG(ConnMgr::GetInst().StartRpcService("127.0.0.1", 6699) != LLBC_OK, -1,
                  "ConnMgr StartRpcService failed");

    ret = RpcServiceMgr::GetInst().Init(&ConnMgr::GetInst());
    COND_RET_ELOG(ret != 0, ret, "RpcServiceMgr init failed|ret:%d", ret);
    bool succ = RpcServiceMgr::GetInst().AddService(new protocol::DemoServiceImpl);
    COND_RET_ELOG(!succ, EXIT_FAILURE, "add service failed");

    ret = RpcServer::GetInst().Init();
    COND_RET_ELOG(ret != 0, ret, "RpcServer init failed|ret:%d", ret);

    RpcCoroMgr::GetInst().UseCoro(true);  // 服务端启用协程来处理请求
    RpcServer::GetInst().Run();

    return 0;
}
