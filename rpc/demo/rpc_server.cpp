/*
 * @Author: modnarshen
 * @Date: 2023.07.11 11:36:13
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <coroutine>

#include <llbc.h>
#include <mt/runner.h>

#include "conn_mgr.h"
#include "rpc_coro_mgr.h"
#include "rpc_server.h"

void RpcServer::Run() {
    mt::sync_wait(Serve());
}

mt::Task<> RpcServer::Serve() {
    LLOG_INFO(">>> RPC SERVER START SERVING <<<");

    LLOG_TRACE("START LOOP");

    while (!stop_) {
        ConnMgr::GetInst().Tick();
        LLBC_Sleep(1);
    }

    LLOG_INFO(">>> RPC SERVER STOP SERVING <<<");
    co_return;
}
