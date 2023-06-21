#include "rpc_channel.h"
#include "conn_mgr.h"
#include "rpc_service_mgr.h"
#include "../pb/echo.pb.h"
#include "llbc.h"
#include <csignal>

using namespace llbc;

class MyEchoService : public echo::EchoService {
public:
  virtual void Echo(::google::protobuf::RpcController* /* controller */,
                       const ::echo::EchoRequest* request,
                       ::echo::EchoResponse* response,
                       ::google::protobuf::Closure* done) 
{
      
      LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "received, msg:%s", request->msg().c_str());
      response->set_msg(
              std::string("I have received '") + request->msg() + std::string("'"));
      done->Run();
  }
};//MyEchoService

bool stop = false;

void signalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received.\n";
  stop = true;
}

int main() 
{
    // 注册信号 SIGINT 和信号处理程序
    signal(SIGINT, signalHandler);  

    // 初始化llbc库
    LLBC_Startup();
    LLBC_Defer(LLBC_Cleanup());
    
    // 初始化日志
    auto ret = LLBC_LoggerMgrSingleton->Initialize("log/cfg/server_log.cfg");
    if (ret == LLBC_FAILED)
    {
        std::cout << "Initialize logger failed, error: " << LLBC_FormatLastError() << std::endl;
        return -1;
    }
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Hello Server!");

    ConnMgr *connMgr = new ConnMgr();
    connMgr->Init();

    // 启动rpc服务
    if (connMgr->StartRpcService("127.0.0.1", 6688) != LLBC_OK)
    {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "connMgr StartRpcService Fail");
        return -1;
    }

    RpcServiceMgr serviceMgr(connMgr);
    MyEchoService echoService;
    serviceMgr.AddService(&echoService);

    // 死循环处理rpc请求
    while (!stop)
    {
        serviceMgr.OnUpdate();
        LLBC_Sleep(1);
    }
    
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "server Stop");

    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
