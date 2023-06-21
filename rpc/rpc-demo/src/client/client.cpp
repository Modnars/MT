#include <iostream>
#include "../pb/echo.pb.h"
#include "rpc_channel.h"
#include "conn_mgr.h"
#include "llbc.h"
#include "rpc_coro_mgr.h"
#include "rpc_service_mgr.h"
#include <csignal>

using namespace llbc;

bool stop = false;

void signalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received.\n";
  stop = true;
}

int main() 
{
    // // 注册信号 SIGINT 和信号处理程序
    // signal(SIGINT, signalHandler);  

    // 初始化llbc库
    LLBC_Startup();
    LLBC_Defer(LLBC_Cleanup());
    
    // 初始化日志
    auto ret = LLBC_LoggerMgrSingleton->Initialize("log/cfg/server_log.cfg");
    if (ret == LLBC_FAILED)
    {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Initialize logger failed, error:%s",  LLBC_FormatLastError());
        return -1;
    }

    // 初始化连接管理器
    ConnMgr *connMgr = new ConnMgr();
    ret = connMgr->Init();
    if(ret != LLBC_OK)
    {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Initialize connMgr failed, error:%s",  LLBC_FormatLastError());
        return -1;
    }
    // if (connMgr->StartRpcService("127.0.0.1", 6688) == LLBC_FAILED)
    // {

    //     LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "connMgr Init Fail");
    //     return -1;
    // }

    // 创建rpc channel
    RpcChannel *channel = connMgr->CreateRpcChannel("127.0.0.1", 6688);
    LLBC_Defer(delete channel);

    if (!channel)
    {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "CreateRpcChannel Fail");
        return -1;
    }

    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Client Start!");

    // 创建rpc req & resp
    echo::EchoRequest req;
    echo::EchoResponse rsp;
    req.set_msg("hello, myrpc.");
    
    // 创建rpc controller & stub
    MyController cntl;
    echo::EchoService_Stub stub(channel);

    // // 协程方案, 在新协程中call rpc
    // RpcServiceMgr serviceMgr(connMgr);
    // while (!stop)
    // {
    //     // 创建协程并Resume
    //     auto func = [&stub, &cntl, &channel, &req, &rsp](void *){
    //         stub.Echo(&cntl, &req, &rsp, nullptr);
    //     };
    //     auto coro = g_rpcCoroMgr->CreateCoro(func, nullptr);
    //     coro->Resume();

    //     // 处理服务收到的数据包，若有Rpc Rsp，OnUpdate内部会唤醒对应休眠的协程
    //     serviceMgr.OnUpdate();
    // }

    // 直接调用方案
    while (!stop)
    {
        stub.Echo(&cntl, &req, &rsp, nullptr);
        // std::cout << "resp:" << response.msg() << std::endl;
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "recv rsp:%s", rsp.msg().c_str());
        LLBC_Sleep(1000);
    }

    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "client Stop");

    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
