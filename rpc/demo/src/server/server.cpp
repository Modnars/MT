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
    LLBC_HookProcessCrash();
    LLBC_Defer(LLBC_Cleanup());

    // 初始化日志
    auto ret = LLBC_LoggerMgrSingleton->Initialize("log/cfg/server_log.cfg");
    if (ret == LLBC_FAILED) {
        fmt::print("Initialize logger failed|error: %s\n", LLBC_FormatLastError());
        return -1;
    }
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Hello Server!");

    ret = ConnMgr::GetInst().Init();
    if (ret != 0) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "ConnMgr Init failed|ret:%d", ret);
        return ret;
    }

    // 启动 rpc 服务
    if (ConnMgr::GetInst().StartRpcService("127.0.0.1", 6688) != LLBC_OK) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "connMgr StartRpcService Failed");
        return -1;
    }

    RpcServiceMgr serviceMgr(&ConnMgr::GetInst());
    serviceMgr.AddService(new DemoServiceImpl);

    bool succ = DemoServiceHelper::GetInst().Init();
    if (!succ) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "DemoServiceHelper init failed");
        return 1;
    }

#define REGISTER_SVR_SERVICE(serviceHelper, ip, port, serverId)                                      \
    {                                                                                                \
        bool _succ = serviceHelper::GetInst().Register(ip, port, serverId);                          \
        if (!_succ) {                                                                                \
            LLOG(nullptr, nullptr, LLBC_LogLevel::Error,                                             \
                 #serviceHelper " register failed|ip:%s|port:%d|server_id:%lu", ip, port, serverId); \
        }                                                                                            \
    }

    REGISTER_SVR_SERVICE(DemoServiceHelper, "127.0.0.1", 6688, 0UL);
    REGISTER_SVR_SERVICE(DemoServiceHelper, "127.0.0.1", 6699, 1UL);

#undef REGISTER_SVR_SERVICE

    // 死循环处理 rpc 请求
    while (!stop) {
        ConnMgr::GetInst().Tick();
        LLBC_Sleep(1);
    }

    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "server Stop");

    return 0;
}
