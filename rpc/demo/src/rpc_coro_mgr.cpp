/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-20 20:13:31
 * @edit: regangcli
 * @brief:
 */

#include "rpc_coro_mgr.h"

// 用协程管理器单例，TODO: 用单例实现较好
BaseRpcCoroMgr *g_rpcCoroMgr = nullptr;