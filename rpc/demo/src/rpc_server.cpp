/*
 * @Author: modnarshen
 * @Date: 2023.07.11 11:36:13
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include "rpc_server.h"
#include "conn_mgr.h"
#include "llbc/core/os/OS_Thread.h"

mt::Task<> RpcServer::serve() {
    LLOG_INFO(">>> RPC SERVER START SERVING <<<");

    while (!stop_) {
        ConnMgr::GetInst().Tick();
        LLBC_Sleep(1);
    }

    LLOG_INFO(">>> RPC SERVER STOP SERVING <<<");
    co_return;
}
