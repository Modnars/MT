/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-21 15:46:03
 * @edit: regangcli
 * @brief:
 */
#pragma once

#include <unordered_map>

#include <mt/util/macros.h>
#include <mt/util/singleton.h>

#include "conn_mgr.h"
#include "demo.pb.h"
#include "demo_service.pb.h"
#include "macros.h"
#include "rpc_channel.h"

// ==================== PERF: AUTO GENERATE FROM PROTOC BEGIN ====================

// class DemoServiceStub {
// public:
//     static mt::Task<int> Echo(std::uint64_t uid, const ::protocol::EchoReq &req, ::protocol::EchoRsp *rsp = nullptr);
// };  // DemoServiceStub

// ==================== PERF: AUTO GENERATE FROM PROTOC END ====================
