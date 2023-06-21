#include <csignal>
#include "conn_mgr.h"
#include "echo_service_impl.h"
#include "llbc.h"
#include "rpc_channel.h"
#include "rpc_service_mgr.h"

using namespace llbc;

bool stop = false;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    stop = true;
}

int main() {
    // 注册信号 SIGINT 和信号处理程序
    signal(SIGINT, signalHandler);

    // 初始化llbc库
    LLBC_Startup();
    LLBC_Defer(LLBC_Cleanup());

    // 初始化日志
    auto ret = LLBC_LoggerMgrSingleton->Initialize("log/cfg/server_log.cfg");
    if (ret == LLBC_FAILED) {
        std::cout << "Initialize logger failed, error: " << LLBC_FormatLastError() << std::endl;
        return -1;
    }
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Hello Server!");

    ConnMgr *connMgr = new ConnMgr();
    connMgr->Init();

    // 启动rpc服务
    if (connMgr->StartRpcService("127.0.0.1", 6688) != LLBC_OK) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "connMgr StartRpcService Fail");
        return -1;
    }

    RpcServiceMgr serviceMgr(connMgr);
    MyEchoService echoService;
    serviceMgr.AddService(&echoService);

    // 死循环处理rpc请求
    while (!stop) {
        connMgr->Tick();
        LLBC_Sleep(1);
    }

    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "server Stop");

    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
