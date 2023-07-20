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
#include "demo_service_impl.h"
#include "google/protobuf/service.h"
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
        auto ret = co_await DemoServiceStub::Echo(0UL, req, &rsp);
        COND_EXP_ELOG(ret != 0, continue, "call Stub::method failed|ret:%d", ret);
    }
    co_return;
}

// TODO modnarshen 改造成一个更通用的 RPC 调用接口
mt::Task<> RpcEcho(std::uint64_t uid, const protocol::EchoReq *req, protocol::EchoRsp *rsp) {
    auto ret = co_await DemoServiceStub::Echo(uid, *req, rsp);
    CO_COND_RET_ELOG(ret != 0, , "call DemoServiceStub::Echo failed|ret:%d", ret);
    LLOG_INFO("recved rsp: %s", rsp->ShortDebugString().c_str());
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

    // // 创建 rpc channel
    // RpcChannel *channel = ConnMgr::GetInst().CreateRpcChannel("127.0.0.1", 6688);
    // LLBC_Defer(delete channel);

    // COND_RET_ELOG(!channel, -1, "CreateRpcChannel failed");
    // LLOG_TRACE("CLIENT START!");

    RpcCoroMgr::GetInst().UseCoro(false);  // 客户端默认不用协程，一直阻塞等待回包

#if ENABLE_CXX20_COROUTINE
    // 协程方案, 在新协程中 call rpc
    ret = RpcServiceMgr::GetInst().Init(&ConnMgr::GetInst());
    COND_RET_ELOG(ret != 0, ret, "RpcServiceMgr init failed|ret:%d", ret);
    RpcServiceMgr::GetInst().RegisterChannel("127.0.0.1", 6688);

    mt::run(mainloop());
#else
    // 创建 rpc controller & stub
    protocol::DemoService_Stub stub{channel};
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
