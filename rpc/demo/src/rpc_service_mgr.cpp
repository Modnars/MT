/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-19 20:13:51
 * @edit: regangcli
 * @brief:
 */
#include "rpc_service_mgr.h"
#include "conn_mgr.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"

void RpcServiceMgr::OnUpdate() {
    // LLOG(nullptr, nullptr, LLBC_LogLevel::Warn, "OnUpdate");
    // 读取接收到的数据包
    auto packet = connMgr_->PopPacket();
    while (packet) {
        LLOG(nullptr, nullptr, LLBC_LogLevel::Warn, "OnUpdate");
        if (packet->GetOpcode() == RpcOpCode::RpcReq) {
            // 读取serviceName&methodName
            std::string serviceName;
            std::string methodName;
            packet->Read(serviceName);
            packet->Read(methodName);
            auto service = _services[serviceName].service;
            auto md = _services[serviceName].mds[methodName];
            LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "recv service_name:%s", serviceName.c_str());
            LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "recv method_name:%s", methodName.c_str());
            LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "req type:%s", md->input_type()->name().c_str());
            LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "rsp type:%s", md->output_type()->name().c_str());

            // 解析req&创建rsp
            auto req = service->GetRequestPrototype(md).New();
            packet->Read(*req);
            auto rsp = service->GetResponsePrototype(md).New();

            // // TODO: 协程方案, 在新协程中处理rpc请求
            // auto func = [packet, service, md, req, rsp, this](void *){
            //     MyController controller;
            //     // 创建rpc完成回调函数
            //     auto done = ::google::protobuf::NewCallback(
            //             this,
            //             &RpcServiceMgr::OnRpcDone,
            //             req,
            //             rsp);
            //     service->CallMethod(md, &controller, req, rsp, done);
            // };
            // Coro *coro = g_rpcCoroMgr->CreateCoro(func, nullptr, std::to_string(sessionId_));
            // coro->SetParam1(packet->GetSessionId());
            // coro->SetParam2(packet->GetExtData1());
            // coro->Resume(); // 启动协程，如协程内部有内嵌发起rpc请求，会在协程内部发起请求后，直接回到此处

            // 直接调用方案
            MyController controller;
            sessionId_ = packet->GetSessionId();

            // 创建rpc完成回调函数
            auto done = ::google::protobuf::NewCallback(this, &RpcServiceMgr::OnRpcDone, req, rsp);
            service->CallMethod(md, &controller, req, rsp, done);
        } else if (packet->GetOpcode() == RpcOpCode::RpcRsp) {
            // // TODO: 协程方案, 唤醒源协程处理rpc请求
            // auto dstCoroId = packet->GetExtData1();
            // Coro *coro = g_rpcCoroMgr->GetCoro(dstCoroId);
            // if (!coro)
            // {
            //     LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "coro not found, coroId:%d", dstCoroId);
            // }
            // // TODO: 唤醒休眠的协程，传递收到的packet
            // else
            // {
            //     coro->SetPtrParam1(packet);
            //     coro->Resume();
            // }
        } else {
            LLOG(nullptr, nullptr, LLBC_LogLevel::Warn, "unknown opcode:%d", packet->GetOpcode());
        }

        // 取下一个包
        LLBC_Recycle(packet);
        packet = connMgr_->PopPacket();
    }
}

void RpcServiceMgr::AddService(::google::protobuf::Service *service) {
    ServiceInfo service_info;
    service_info.service = service;
    service_info.sd = service->GetDescriptor();
    for (int i = 0; i < service_info.sd->method_count(); ++i) {
        service_info.mds[service_info.sd->method(i)->name()] = service_info.sd->method(i);
    }

    _services[service_info.sd->name()] = service_info;
}

// void RpcServiceMgr::dispatch_msg(
//         const std::string& service_name,
//         const std::string& method_name,
//         const std::string& serialzied_data)
// {
//     auto service = _services[service_name].service;
//     auto md = _services[service_name].mds[method_name];

//     LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "recv service_name:%s", service_name.c_str());
//     LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "recv method_name:%s", method_name.c_str());
//     LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "req type:%s",  md->input_type()->name().c_str());
//     LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "rsp type:%s", md->output_type()->name().c_str());

//     auto req = service->GetRequestPrototype(md).New();
//     req->ParseFromString(serialzied_data);
//     auto rsp = service->GetResponsePrototype(md).New();

//     MyController controller;
//     auto done = ::google::protobuf::NewCallback(
//             this,
//             &RpcServiceMgr::OnRpcDone,
//             req,
//             rsp);
//     service->CallMethod(md, &controller, req, rsp, done);
// }

void RpcServiceMgr::OnRpcDone(::google::protobuf::Message *req, ::google::protobuf::Message *rsp) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "OnRpcDone, req:%s, rsp:%s", req->DebugString().c_str(),
         rsp->DebugString().c_str());

    // // 协程方案
    // auto coro = g_rpcCoroMgr->GetCurCoro();
    // auto sessionId = coro->GetParam1();
    // auto srcCoroId = coro->GetParam2();
    // auto packet = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    // packet->SetSessionId(sessionId);
    // packet->SetOpcode(RpcOpCode::RpcRsp);
    // packet->SetExtData1(srcCoroId);
    // packet->Write(*rsp);
    // connMgr_->PushPacket(packet);

    // 直接调用方案
    auto packet = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
    packet->SetSessionId(sessionId_);
    packet->SetOpcode(RpcOpCode::RpcRsp);
    packet->SetExtData1(0);
    packet->Write(*rsp);
    // 回包
    connMgr_->PushPacket(packet);
}

// void RpcServiceMgr::pack_message(
//         const ::google::protobuf::Message* msg,
//         std::string* serialized_data)
// {
//     int serialized_size = msg->ByteSize();
//     serialized_data->assign(
//                 (const char*)&serialized_size,
//                 sizeof(serialized_size));
//     msg->AppendToString(serialized_data);
// }