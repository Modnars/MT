#include <csignal>
#include <cstdlib>
#include <functional>
#include <iostream>

#include <fmt/core.h>
#include <llbc.h>
#include <mt/runner.h>

#include "conn_mgr.h"
#include "demo.pb.h"
#include "demo_service.pb.h"
#include "macros.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_service_mgr.h"

using namespace llbc;

bool stop = false;
static const std::string CLIENT_LLOG_CONF_PATH = "../../config/client_log.cfg";

void signalHandler(int signum) {
    fmt::print("INTERRUPT SIGNAL [{}] RECEIVED.\n", signum);
    stop = true;
}

mt::Task<> mainloop() {
    // 创建 rpc req & resp
    protocol::EchoReq req;
    protocol::EchoRsp rsp;

    while (!stop) {
        std::string input;
        COND_EXP(!(std::cin >> input), break);  // 手动阻塞
        COND_EXP(input == "exit", break);
        req.set_msg(input);
        auto ret = co_await protocol::DemoServiceStub::Echo(0UL, req, &rsp);
        COND_EXP_ELOG(ret != 0, continue, "call Stub::method failed|ret:%d", ret);
    }
    co_return;
}

int main(int argc, char *argv[]) {
    // 注册信号 SIGINT 和信号处理程序
    ::signal(SIGINT, signalHandler);

    // 初始化 llbc 库
    LLBC_Startup();
    LLBC_Defer(LLBC_Cleanup());

    // 初始化日志
    auto ret = LLBC_LoggerMgrSingleton->Initialize(CLIENT_LLOG_CONF_PATH);
    if (ret == LLBC_FAILED) {
        fmt::print("Initialize logger failed|path:{}|error: {}\n", CLIENT_LLOG_CONF_PATH, LLBC_FormatLastError());
        return EXIT_FAILURE;
    }

    // 初始化连接管理器
    ret = ConnMgr::GetInst().Init();
    COND_RET_ELOG(ret != LLBC_OK, -1, "init ConnMgr failed|error: %s", LLBC_FormatLastError());

    RpcCoroMgr::GetInst().UseCoro(false);  // 客户端默认不用协程，一直阻塞等待回包

    // 协程方案, 在新协程中 call rpc
    ret = RpcServiceMgr::GetInst().Init(&ConnMgr::GetInst());
    COND_RET_ELOG(ret != 0, ret, "RpcServiceMgr init failed|ret:%d", ret);
    RpcServiceMgr::GetInst().RegisterChannel("127.0.0.1", 6688);

    mt::run(mainloop());

    LLOG_TRACE("client stop");

    return 0;
}
