/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-20 20:13:31
 * @edit: regangcli
 * @brief:
 */
#include "rpc_coro_mgr.h"

RpcCoroMgr::coro_uid_type RpcCoroMgr::coro_uid_generator_ = 0UL;
