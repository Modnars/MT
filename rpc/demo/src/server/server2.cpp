#include <csignal>

#include <fmt/core.h>
#include <llbc.h>

#include "conn_mgr.h"
#include "demo_service_impl.h"
#include "macros.h"
#include "rpc_channel.h"
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
        fmt::print("Initialize logger failed|error: %s\n", LLBC_FormatLastError());
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
    RpcServiceMgr::GetInst().AddService(new DemoServiceImpl);

    RpcController::GetInst().SetUseCoro(true);  // 服务端启用协程来处理请求
    // 死循环处理 rpc 请求
    while (!stop) {
        ConnMgr::GetInst().Tick();
        LLBC_Sleep(1);
    }

    LLOG_TRACE("server stop");

    return 0;
}
