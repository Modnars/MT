/*
 * @Author: modnarshen
 * @Date: 2023.07.20 11:47:54
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include "pb_co_code_generator.h"
#include "rpc_options.pb.h"

#define ADD_IFDEF(macro_str, ss)                  \
    {                                             \
        if (!macro_str.empty()) {                 \
            ss << "#ifdef " << macro_str << "\n"; \
        }                                         \
    }

#define ADD_ENDIF(macro_str, ss)  \
    {                             \
        if (!macro_str.empty()) { \
            ss << "#endif\n";     \
        }                         \
    }

static const std::string g_indent = "    ";
static const uint32_t g_default_timeout = 5000;

void PbCoCodeGenerator::CollectHeadFile(const ::google::protobuf::FileDescriptor *file,
                                        std::set<std::string> &need_head_file) {
    for (int i = 0; i < file->service_count(); ++i) {
        const ::google::protobuf::ServiceDescriptor *service = file->service(i);
        std::string name_prefix = service->options().GetExtension(ALL_PREFIX_NAME);
        if (!name_prefix.empty()) {
            std::transform(name_prefix.begin(), name_prefix.end(), name_prefix.begin(),
                           [](auto &&c) { return std::tolower(c, std::locale()); });
            need_head_file.insert(name_prefix);
        }

        for (int j = 0; j < service->method_count(); ++j) {
            const ::google::protobuf::MethodDescriptor *method = service->method(j);
            std::string class_name_prefix = method->options().GetExtension(PREFIX_NAME);
            if (!class_name_prefix.empty()) {
                std::transform(class_name_prefix.begin(), class_name_prefix.end(), class_name_prefix.begin(),
                               [](auto &&c) { return std::tolower(c, std::locale()); });
                need_head_file.insert(class_name_prefix);
            }
        }
    }
}

std::string PbCoCodeGenerator::GetHeaderIncludeStr(const std::string &base_path,
                                                   const std::set<std::string> &need_head_file) const {
    std::string prefix_str = "#include \"";
    prefix_str += base_path;
    prefix_str += "/";

    std::string include_str;
    std::for_each(need_head_file.begin(), need_head_file.end(), [&](auto &&str) {
        include_str += prefix_str;
        include_str += str;
        include_str += "_context.h\"\n";
    });
    return include_str;
}

std::string PbCoCodeGenerator::GetSourceIncludeStr(const std::string &base_path,
                                                   const std::set<std::string> &need_head_file) const {
    std::string prefix_str = "#include \"";
    prefix_str += base_path;
    prefix_str += "/";

    std::string include_str;
    std::for_each(need_head_file.begin(), need_head_file.end(), [&](auto &&str) {
        include_str += prefix_str;
        include_str += str;
        include_str += "_service.h\"\n";
    });
    return include_str;
}

std::string PbCoCodeGenerator::GenRpcDecl(const ::google::protobuf::FileDescriptor *file) const {
    std::stringstream ss;
    for (int i = 0; i < file->service_count(); ++i) {
        std::string name_prefix = file->service(i)->options().GetExtension(ALL_PREFIX_NAME);
        GenServiceDecl(ss, file->service(i), name_prefix);
        ss << "\n";
        GenStubDecl(ss, file->service(i), name_prefix);
        ss << "\n";
    }

    return ss.str();
}

std::string PbCoCodeGenerator::GenRpcImpl(const ::google::protobuf::FileDescriptor *file) const {
    std::stringstream ss;
    for (int i = 0; i < file->service_count(); ++i) {
        std::string name_prefix = file->service(i)->options().GetExtension(ALL_PREFIX_NAME);
        GenServiceImpl(ss, file->service(i), name_prefix);
        ss << "\n";
        GenStubImpl(ss, file->service(i), name_prefix);
        ss << "\n";
    }

    return ss.str();
}

void PbCoCodeGenerator::GenServiceDecl(std::stringstream &ss, const ::google::protobuf::ServiceDescriptor *service,
                                       const std::string &name_prefix) const {
    // 同步的base类
    ss << "class " << service->name() << "Base : public " << service->name() << "\n"
       << "{\n"
       << "protected:\n";
    for (int i = 0; i < service->method_count(); ++i) {
        std::string class_name_prefix = service->method(i)->options().GetExtension(PREFIX_NAME);
        if (class_name_prefix.empty())
            GenServiceProtectedMethodDecl(ss, service->method(i), false, name_prefix);
        else
            GenServiceProtectedMethodDecl(ss, service->method(i), false, class_name_prefix);
    }
    ss << "private:\n";
    for (int i = 0; i < service->method_count(); ++i) {
        GenServicePrivateMethodDecl(ss, service->method(i));
    }
    ss << "};\n";

    ss << "\n";

    // 异步的base类
    ss << "class " << service->name() << "AsyncBase : public " << service->name() << "\n"
       << "{\n"
       << "protected:\n";
    for (int i = 0; i < service->method_count(); ++i) {
        std::string class_name_prefix = service->method(i)->options().GetExtension(PREFIX_NAME);
        if (class_name_prefix.empty())
            GenServiceProtectedMethodDecl(ss, service->method(i), true, name_prefix);
        else
            GenServiceProtectedMethodDecl(ss, service->method(i), true, class_name_prefix);
    }
    ss << "private:\n";
    for (int i = 0; i < service->method_count(); ++i) {
        GenServicePrivateMethodDecl(ss, service->method(i));
    }
    ss << "};\n";
}

void PbCoCodeGenerator::GenServiceImpl(std::stringstream &ss, const ::google::protobuf::ServiceDescriptor *service,
                                       const std::string &name_prefix) const {
    // 同步接口的实现
    for (int i = 0; i < service->method_count(); ++i) {
        std::string class_name_prefix = service->method(i)->options().GetExtension(PREFIX_NAME);
        if (class_name_prefix.empty())
            GenServiceMethodImpl(ss, service->name() + "Base", service->method(i), false, name_prefix);
        else
            GenServiceMethodImpl(ss, service->name() + "Base", service->method(i), false, class_name_prefix);
        ss << "\n";
    }

    // 异步接口的实现
    for (int i = 0; i < service->method_count(); ++i) {
        std::string class_name_prefix = service->method(i)->options().GetExtension(PREFIX_NAME);
        if (class_name_prefix.empty())
            GenServiceMethodImpl(ss, service->name() + "AsyncBase", service->method(i), true, name_prefix);
        else
            GenServiceMethodImpl(ss, service->name() + "AsyncBase", service->method(i), true, class_name_prefix);
        ss << "\n";
    }
}

void PbCoCodeGenerator::GenServiceProtectedMethodDecl(std::stringstream &ss,
                                                      const ::google::protobuf::MethodDescriptor *method, bool is_async,
                                                      const std::string &name_prefix) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    ADD_IFDEF(macro_str, ss);
    ss << g_indent << "virtual int32_t " << method->name() << "(const " << method->input_type()->name() << "& req, ";

    if (is_async) {
        ss << method->output_type()->name() << "& rsp, ua::" << name_prefix << "Context* context) = 0;\n";
    } else {
        ss << method->output_type()->name() << "& rsp, const ua::" << name_prefix << "Context& context) = 0;\n";
    }
    ADD_ENDIF(macro_str, ss);
}

void PbCoCodeGenerator::GenServicePrivateMethodDecl(std::stringstream &ss,
                                                    const ::google::protobuf::MethodDescriptor *method) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    ADD_IFDEF(macro_str, ss);
    ss << g_indent << "virtual void " << method->name() << "(::google::protobuf::RpcController* controller,\n"
       << g_indent << g_indent << "const " << method->input_type()->name() << "* req,\n"
       << g_indent << g_indent << method->output_type()->name() << "* rsp,\n"
       << g_indent << g_indent << "::google::protobuf::Closure* done) final;\n";
    ADD_ENDIF(macro_str, ss);
}

void PbCoCodeGenerator::GenServiceMethodImpl(std::stringstream &ss, const std::string &class_name,
                                             const ::google::protobuf::MethodDescriptor *method, bool is_async,
                                             const std::string &name_prefix) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    ADD_IFDEF(macro_str, ss);
    ss << "void " << class_name << "::" << method->name() << "(::google::protobuf::RpcController* controller,\n"
       << g_indent << "const " << method->input_type()->name() << "* req,\n"
       << g_indent << method->output_type()->name() << "* rsp,\n"
       << g_indent << "::google::protobuf::Closure* done)\n"
       << "{\n"
       << g_indent << "ua::" << name_prefix << "Context* context = static_cast<ua::" << name_prefix
       << "Context*>(controller);\n";

    if (is_async) {
        ss << g_indent << "context->ret_code = " << method->name() << "(*req, *rsp, context);\n";
    } else {
        ss << g_indent << "context->ret_code = " << method->name() << "(*req, *rsp, *context);\n";
    }
    ss << "}\n";
    ADD_ENDIF(macro_str, ss);
}

void PbCoCodeGenerator::GenStubDecl(std::stringstream &ss, const ::google::protobuf::ServiceDescriptor *service,
                                    const std::string &name_prefix) const {
    ss << "class " << service->name() << "Stub\n"
       << "{\n"
       << "public:\n";
    for (int i = 0; i < service->method_count(); ++i) {
        std::string class_name_prefix = service->method(i)->options().GetExtension(PREFIX_NAME);
        if (class_name_prefix.empty())
            GenStubMethodDecl(ss, service->method(i), name_prefix);
        else
            GenStubMethodDecl(ss, service->method(i), class_name_prefix);
    }
    ss << "\n";
    for (int i = 0; i < service->method_count(); ++i) {
        std::string class_name_prefix = service->method(i)->options().GetExtension(PREFIX_NAME);
        if (class_name_prefix.empty())
            GenStubAsyncMethodDecl(ss, service->method(i), name_prefix);
        else
            GenStubAsyncMethodDecl(ss, service->method(i), class_name_prefix);
    }
    ss << "};\n";
}

void PbCoCodeGenerator::GenStubImpl(std::stringstream &ss, const ::google::protobuf::ServiceDescriptor *service,
                                    const std::string &name_prefix) const {
    for (int i = 0; i < service->method_count(); ++i) {
        std::string class_name_prefix = service->method(i)->options().GetExtension(PREFIX_NAME);
        if (class_name_prefix.empty())
            GenStubMethodImpl(ss, service->name() + "Stub", service->method(i), name_prefix);
        else
            GenStubMethodImpl(ss, service->name() + "Stub", service->method(i), class_name_prefix);
        ss << "\n";
    }

    for (int i = 0; i < service->method_count(); ++i) {
        std::string class_name_prefix = service->method(i)->options().GetExtension(PREFIX_NAME);
        if (class_name_prefix.empty())
            GenStubAsyncMethodImpl(ss, service->name() + "Stub", service->method(i), name_prefix);
        else
            GenStubAsyncMethodImpl(ss, service->name() + "Stub", service->method(i), class_name_prefix);
        ss << "\n";
    }
}

void PbCoCodeGenerator::GenStubMethodDecl(std::stringstream &ss, const ::google::protobuf::MethodDescriptor *method,
                                          const std::string &name_prefix) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    ADD_IFDEF(macro_str, ss);
    if (name_prefix == "Http") {
        ss << g_indent << "static int32_t " << method->name() << "(const " << method->input_type()->name() << "& req, "
           << method->output_type()->name() << "* rsp = nullptr, bool need_encode = false, uint32_t timeout = 0);\n";
    } else {
        if (method->options().GetExtension(IS_BROADCAST) || method->options().GetExtension(WORLD_BROADCAST)) {
            ss << g_indent << "static int32_t " << method->name() << "(uint64_t gid, const "
               << method->input_type()->name() << "& req);\n";
        } else {
            ss << g_indent << "static int32_t " << method->name() << "(uint64_t gid, const "
               << method->input_type()->name() << "& req, " << method->output_type()->name()
               << "* rsp = nullptr, uint32_t dest_id = 0, const Context::Callback& callback = nullptr, uint32_t "
                  "timeout = 0);\n";
        }
    }
    ADD_ENDIF(macro_str, ss);
}

void PbCoCodeGenerator::GenStubAsyncMethodDecl(std::stringstream &ss,
                                               const ::google::protobuf::MethodDescriptor *method,
                                               const std::string &name_prefix) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    ADD_IFDEF(macro_str, ss);
    if (method->options().GetExtension(IS_BROADCAST) || method->options().GetExtension(WORLD_BROADCAST)) {
        ss << g_indent << "static int32_t Async" << method->name() << "(uint64_t gid, const "
           << method->input_type()->name() << "& req);\n";
    } else {
        ss << g_indent << "using " << method->name() << "CallBack = std::function<int32_t(int32_t, const "
           << method->output_type()->name() << "&, ua::Context*)>;\n";

        if (name_prefix == "Http") {
            ss << g_indent << "static int32_t Async" << method->name() << "(const " << method->input_type()->name()
               << "& req, " << method->name()
               << "CallBack callback = nullptr, ua::Context* context = nullptr, bool need_encode = false, uint32_t "
                  "timeout = 0);\n";
        } else {
            ss << g_indent << "static int32_t Async" << method->name() << "(uint64_t gid, const "
               << method->input_type()->name() << "& req, " << method->name()
               << "CallBack callback = nullptr, ua::Context* context = nullptr, uint32_t dest_id = 0, uint32_t timeout "
                  "= 0);\n";
        }
    }
    ADD_ENDIF(macro_str, ss);
}

void PbCoCodeGenerator::GenStubMethodImpl(std::stringstream &ss, const std::string &class_name,
                                          const ::google::protobuf::MethodDescriptor *method,
                                          const std::string &name_prefix) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    ADD_IFDEF(macro_str, ss);
    if (name_prefix == "Http") {
        uint32_t timeout = g_default_timeout;
        if (method->options().HasExtension(TIME_OUT))
            timeout = method->options().GetExtension(TIME_OUT);

        ss << "int32_t " << class_name << "::" << method->name() << "(const " << method->input_type()->name()
           << "& req, " << method->output_type()->name() << "* rsp, bool need_encode, uint32_t timeout)\n"
           << "{\n"
           << g_indent << "if (timeout == 0)\n"
           << g_indent << g_indent << "timeout = " << std::dec << timeout << ";\n"
           << g_indent << "return ua::HttpService::GetInst().Rpc("
           << "req"
           << ", rsp, nullptr, nullptr, need_encode, timeout);\n"
           << "}\n";
    } else {
        bool is_broadcast = method->options().GetExtension(IS_BROADCAST);
        uint32_t broadcast_world = method->options().GetExtension(WORLD_BROADCAST);
        if (is_broadcast || broadcast_world) {
            ss << "int32_t " << class_name << "::" << method->name() << "(uint64_t gid, const "
               << method->input_type()->name() << "& req)\n"
               << "{\n"
               << g_indent << "return ua::" << name_prefix << "Service::GetInst().Rpc(gid, 0x" << std::hex
               << method->options().GetExtension(RPC_CMD) << ", req"
               << ", nullptr, nullptr, nullptr, GetCrossWorldDest(0x" << broadcast_world << "), true";
        } else {
            uint32_t timeout = g_default_timeout;
            if (method->options().HasExtension(TIME_OUT))
                timeout = method->options().GetExtension(TIME_OUT);

            ss << "int32_t " << class_name << "::" << method->name() << "(uint64_t gid, const "
               << method->input_type()->name() << "& req, " << method->output_type()->name()
               << "* rsp, uint32_t dest_id, const Context::Callback& callback, uint32_t timeout)\n"
               << "{\n"
               << g_indent << "if (timeout == 0)\n"
               << g_indent << g_indent << "timeout = " << std::dec << timeout << ";\n"
               << g_indent << "return ua::" << name_prefix << "Service::GetInst().Rpc(gid, 0x" << std::hex
               << method->options().GetExtension(RPC_CMD) << ", req"
               << ", rsp, callback, nullptr, dest_id, false, timeout";
        }
        ss << ");\n"
           << "}\n";
    }
    ADD_ENDIF(macro_str, ss);
}

void PbCoCodeGenerator::GenStubAsyncMethodImpl(std::stringstream &ss, const std::string &class_name,
                                               const ::google::protobuf::MethodDescriptor *method,
                                               const std::string &name_prefix) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    ADD_IFDEF(macro_str, ss);
    if (name_prefix == "Http") {
        uint32_t timeout = g_default_timeout;
        if (method->options().HasExtension(TIME_OUT))
            timeout = method->options().GetExtension(TIME_OUT);

        ss << "int32_t " << class_name << "::Async" << method->name() << "(const " << method->input_type()->name()
           << "& req, " << method->name()
           << "CallBack callback, ua::Context* context, bool need_encode, uint32_t timeout)\n"
           << "{\n"
           << g_indent << "if (timeout == 0)\n"
           << g_indent << g_indent << "timeout = " << std::dec << timeout << ";\n"
           << g_indent << "if (callback)\n"
           << g_indent << "{\n"
           << g_indent << g_indent << "auto tmp_rsp = new " << method->output_type()->name() << ";\n"
           << g_indent << g_indent << "return ua::HttpService::GetInst().Rpc("
           << "req, tmp_rsp,\n"
           << g_indent << g_indent << g_indent << "[=](int32_t ret_code) {\n"
           << g_indent << g_indent << g_indent << g_indent
           << "context->ret_code = callback(ret_code, *tmp_rsp, context);\n"
           << g_indent << g_indent << g_indent << g_indent << "delete tmp_rsp;\n"
           << g_indent << g_indent << g_indent << g_indent << "}, context, need_encode, timeout);\n"
           << g_indent << "}\n"
           << g_indent << "else\n"
           << g_indent << "{\n"
           << g_indent << g_indent << "return ua::HttpService::GetInst().Rpc("
           << "req, nullptr, nullptr, nullptr, need_encode, timeout);\n"
           << g_indent << "}\n"
           << g_indent << "return 0;\n";
    } else {
        bool is_broadcast = method->options().GetExtension(IS_BROADCAST);
        uint32_t broadcast_world = method->options().GetExtension(WORLD_BROADCAST);
        if (is_broadcast || broadcast_world) {
            ss << "int32_t " << class_name << "::Async" << method->name() << "(uint64_t gid, const "
               << method->input_type()->name() << "& req)\n"
               << "{\n"
               << g_indent << "return ua::" << name_prefix << "Service::GetInst().Rpc(gid, 0x" << std::hex
               << method->options().GetExtension(RPC_CMD) << ", req"
               << ", nullptr, nullptr, nullptr, GetCrossWorldDest(0x" << broadcast_world << "), true";
            if (method->options().HasExtension(TIME_OUT)) {
                ss << ", " << std::dec << method->options().GetExtension(TIME_OUT);
            }
            ss << ");\n";
        } else {
            uint32_t timeout = g_default_timeout;
            if (method->options().HasExtension(TIME_OUT))
                timeout = method->options().GetExtension(TIME_OUT);

            ss << "int32_t " << class_name << "::Async" << method->name() << "(uint64_t gid, const "
               << method->input_type()->name() << "& req, " << method->name()
               << "CallBack callback, ua::Context* context, uint32_t dest_id, uint32_t timeout)\n"
               << "{\n"
               << g_indent << "if (timeout == 0)\n"
               << g_indent << g_indent << "timeout = " << std::dec << timeout << ";\n"
               << g_indent << "if (callback)\n"
               << g_indent << "{\n"
               << g_indent << g_indent << "auto tmp_rsp = new " << method->output_type()->name() << ";\n"
               << g_indent << g_indent << "return ua::" << name_prefix << "Service::GetInst().Rpc(gid, 0x" << std::hex
               << method->options().GetExtension(RPC_CMD) << ", req, tmp_rsp,\n"
               << g_indent << g_indent << g_indent << "[=](int32_t ret_code) {\n"
               << g_indent << g_indent << g_indent << g_indent
               << "context->ret_code = callback(ret_code, *tmp_rsp, context);\n"
               << g_indent << g_indent << g_indent << g_indent << "delete tmp_rsp;\n"
               << g_indent << g_indent << g_indent << g_indent << "}, context, dest_id, false, timeout);\n"
               << g_indent << "}\n"
               << g_indent << "else\n"
               << g_indent << "{\n"
               << g_indent << g_indent << "return ua::" << name_prefix << "Service::GetInst().Rpc(gid, 0x" << std::hex
               << method->options().GetExtension(RPC_CMD)
               << ", req, nullptr, nullptr, nullptr, dest_id, false, timeout);\n"
               << g_indent << "}\n";
        }
    }
    ss << "}\n";
    ADD_ENDIF(macro_str, ss);
}
