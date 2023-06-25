#include <csignal>

#include <fmt/core.h>
#include "llbc.h"

#include "conn_mgr.h"
#include "demo_service_impl.h"
#include "llbc/core/log/LogLevel.h"
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
        fmt::print("Initialize logger failed, error: %s\n", LLBC_FormatLastError());
        return -1;
    }
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Hello Server!");

    ConnMgr *connMgr = new ConnMgr();
    connMgr->Init();

    // 启动 rpc 服务
    if (connMgr->StartRpcService("127.0.0.1", 6688) != LLBC_OK) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "connMgr StartRpcService Failed");
        return -1;
    }

    RpcServiceMgr serviceMgr(connMgr);
    serviceMgr.AddService(new DemoServiceImpl);

    RpcChannel *channel = connMgr->CreateRpcChannel("127.0.0.1", 6688);
    LLBC_Defer(delete channel);
    if (!channel) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "CreateRpcChannel Failed");
        return -1;
    }
    protocol::DemoService_Stub stub{channel};
    bool succ = DemoServiceHelper::GetInst().Init(&stub);
    if (!succ) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "Init ServiceHelper failed");
        return -1;
    }

    // 死循环处理 rpc 请求
    while (!stop) {
        connMgr->Tick();
        LLBC_Sleep(1);
    }

    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "server Stop");

    return 0;
}
