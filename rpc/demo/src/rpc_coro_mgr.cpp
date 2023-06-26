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

std::map<uint64_t /* task_id_ */, mt::Task<> /* task_ */> id_to_task_map_ = {};
