/*
 * @Author: modnarshen
 * @Date: 2023.07.11 11:36:13
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <llbc.h>
#include <coroutine>

#include "conn_mgr.h"
#include "rpc_coro_mgr.h"
#include "rpc_server.h"

mt::Task<> RpcServer::serve() {
    LLOG_INFO(">>> RPC SERVER START SERVING <<<");

    co_await MainCoroAwaiter{true};
    LLOG_TRACE("START LOOP");

    while (!stop_) {
        ConnMgr::GetInst().Tick();
        LLBC_Sleep(1);
    }

    LLOG_INFO(">>> RPC SERVER STOP SERVING <<<");
    co_return;
}
