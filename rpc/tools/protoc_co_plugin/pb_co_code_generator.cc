/*
 * @Author: modnarshen
 * @Date: 2023.07.20 11:47:54
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <google/protobuf/descriptor.h>
#include <cstddef>
#include <iostream>

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

static const std::string g_indent = "  ";
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
    include_str += "#include \"mt/task.h\"\n";
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
    include_str += "#include \"mt/runner.h\"\n";
    include_str += "#include \"rpc_service_mgr.h\"\n";
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
    // C++20 协程接口类
    ss << "class " << service->name() << "Impl : public " << service->name() << "{\n"
       << "protected:\n";
    for (int i = 0; i < service->method_count(); ++i) {
        std::string class_name_prefix = service->method(i)->options().GetExtension(PREFIX_NAME);
        if (class_name_prefix.empty())
            GenCoServiceProtectedMethodDecl(ss, service->method(i), false, name_prefix);
        else
            GenCoServiceProtectedMethodDecl(ss, service->method(i), false, class_name_prefix);
    }
    ss << "\nprivate:\n";
    for (int i = 0; i < service->method_count(); ++i) {
        GenCoServicePrivateMethodDecl(ss, service->method(i));
    }
    ss << "\npublic:\n";
    GenCoServicePublicMethodDecl(ss);
    ss << "};\n";
}

void PbCoCodeGenerator::GenServiceImpl(std::stringstream &ss, const ::google::protobuf::ServiceDescriptor *service,
                                       const std::string &name_prefix) const {
    // C++20 协程同步接口的实现
    for (int i = 0; i < service->method_count(); ++i) {
        std::string class_name_prefix = service->method(i)->options().GetExtension(PREFIX_NAME);
        if (class_name_prefix.empty())
            GenCoServiceMethodImpl(ss, service->name() + "Impl", service->method(i), false, name_prefix);
        else
            GenCoServiceMethodImpl(ss, service->name() + "Impl", service->method(i), false, class_name_prefix);
        ss << "\n";
    }
    GenCoServicePublicMethodImpl(ss, service->name() + "Impl", service);
}

void PbCoCodeGenerator::GenCoServiceProtectedMethodDecl(std::stringstream &ss,
                                                        const ::google::protobuf::MethodDescriptor *method,
                                                        bool is_async, const std::string &name_prefix) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    const std::size_t indent_num = 23UL + method->name().size();
    ADD_IFDEF(macro_str, ss);
    ss << g_indent << "virtual mt::Task<int> " << method->name() << "(::google::protobuf::RpcController* controller,\n"
       << g_indent << std::string(indent_num, ' ') << "const " << method->input_type()->name() << "& request, "
       << method->output_type()->name() << "& response,\n"
       << g_indent << std::string(indent_num, ' ') << "::google::protobuf::Closure *done);\n";

    ADD_ENDIF(macro_str, ss);
}

void PbCoCodeGenerator::GenServicePrivateMethodDecl(std::stringstream &ss,
                                                    const ::google::protobuf::MethodDescriptor *method) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    ADD_IFDEF(macro_str, ss);
    const std::size_t indent_num = 14UL + method->name().size();
    ss << g_indent << "virtual void " << method->name() << "(::google::protobuf::RpcController* controller,\n"
       << g_indent << std::string(indent_num, ' ') << "const " << method->input_type()->name() << "* request, "
       << method->output_type()->name() << "* response,\n"
       << g_indent << std::string(indent_num, ' ') << "::google::protobuf::Closure* done);\n";
    ADD_ENDIF(macro_str, ss);
}

void PbCoCodeGenerator::GenCoServicePrivateMethodDecl(std::stringstream &ss,
                                                      const ::google::protobuf::MethodDescriptor *method) const {
    GenServicePrivateMethodDecl(ss, method);
}

void PbCoCodeGenerator::GenCoServicePublicMethodDecl(std::stringstream &ss) const {
    const std::size_t indent_num = 27UL;
    ss << g_indent << "mt::Task<int> CallCoMethod(const ::google::protobuf::MethodDescriptor* method,\n"
       << g_indent << std::string(indent_num, ' ') << "::google::protobuf::RpcController* controller,\n"
       << g_indent << std::string(indent_num, ' ') << "const ::google::protobuf::Message& request,\n"
       << g_indent << std::string(indent_num, ' ') << "::google::protobuf::Message& response,\n"
       << g_indent << std::string(indent_num, ' ') << "::google::protobuf::Closure* done);";
    return;
}

void PbCoCodeGenerator::GenCoServiceMethodImpl(std::stringstream &ss, const std::string &class_name,
                                               const ::google::protobuf::MethodDescriptor *method, bool is_async,
                                               const std::string &name_prefix) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    ADD_IFDEF(macro_str, ss);
    const std::size_t indent_num = class_name.size() + method->name().size() + 8UL;
    ss << "void " << class_name << "::" << method->name() << "(::google::protobuf::RpcController* controller,\n"
       << std::string(indent_num, ' ') << "const " << method->input_type()->name() << "* request, "
       << method->output_type()->name() << "* response,\n"
       << std::string(indent_num, ' ') << "::google::protobuf::Closure* done) {\n";

    ss << g_indent << "mt::run(" << method->name() << "(controller, *request, *response, done));\n";
    ss << "}\n";
    ADD_ENDIF(macro_str, ss);
    return;
}

void PbCoCodeGenerator::GenCoServicePublicMethodImpl(std::stringstream &ss, const std::string &class_name,
                                                     const ::google::protobuf::ServiceDescriptor *service) const {
    std::size_t indent_num = class_name.size() + 29UL;
    ss << "mt::Task<int> " << class_name << "::"
       << "CallCoMethod(const ::google::protobuf::MethodDescriptor* method,\n"
       << std::string(indent_num, ' ') << "::google::protobuf::RpcController* controller,\n"
       << std::string(indent_num, ' ') << "const ::google::protobuf::Message& request,\n"
       << std::string(indent_num, ' ') << "::google::protobuf::Message& response,\n"
       << std::string(indent_num, ' ') << "::google::protobuf::Closure* done) {\n";
    ss << g_indent << "switch(method->index()) {\n";
    for (int idx = 0; idx < service->method_count(); ++idx) {
        auto *method = service->method(idx);
        ss << g_indent << g_indent << "case " << idx << ":\n"
           << g_indent << g_indent << g_indent << "co_return co_await " << method->name() << "(controller,\n"
           << g_indent << g_indent << g_indent << g_indent << g_indent
           << "*::google::protobuf::internal::DownCast<const " << method->input_type()->name() << "*>(&request),\n"
           << g_indent << g_indent << g_indent << g_indent << g_indent << "*::google::protobuf::internal::DownCast<"
           << method->output_type()->name() << "*>(&response), done);\n"
           << g_indent << g_indent << g_indent << "break;\n";
    }
    ss << g_indent << g_indent << "default:\n"
       << g_indent << g_indent << g_indent << "GOOGLE_LOG(FATAL) << \"Bad method index; this should never happen.\";\n"
       << g_indent << g_indent << g_indent << "break;\n";
    ss << g_indent << "}\n" << g_indent << "co_return 0;\n}\n";
}

void PbCoCodeGenerator::GenStubDecl(std::stringstream &ss, const ::google::protobuf::ServiceDescriptor *service,
                                    const std::string &name_prefix) const {
    ss << "class " << service->name() << "Stub {\n"
       << "public:\n";
    for (int i = 0; i < service->method_count(); ++i) {
        std::string class_name_prefix = service->method(i)->options().GetExtension(PREFIX_NAME);
        if (class_name_prefix.empty())
            GenStubMethodDecl(ss, service->method(i), name_prefix);
        else
            GenStubMethodDecl(ss, service->method(i), class_name_prefix);
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
}

void PbCoCodeGenerator::GenStubMethodDecl(std::stringstream &ss, const ::google::protobuf::MethodDescriptor *method,
                                          const std::string &name_prefix) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    ADD_IFDEF(macro_str, ss);
    const std::size_t indent_num = 22UL + method->name().size();
    if (method->options().GetExtension(IS_BROADCAST) || method->options().GetExtension(WORLD_BROADCAST)) {
        std::cerr << "not support the option currently|options:IS_BROADCAST,WORLD_BROADCAST" << std::endl;
    } else {
        ss << g_indent << "static mt::Task<int> " << method->name() << "(std::uint64_t uid,\n"
           << g_indent << std::string(indent_num, ' ') << "const " << method->input_type()->name() << "& request, "
           << method->output_type()->name() << "* response = nullptr,\n"
           << g_indent << std::string(indent_num, ' ') << "std::uint32_t timeout = 0);\n";
    }
    ADD_ENDIF(macro_str, ss);
}

void PbCoCodeGenerator::GenStubMethodImpl(std::stringstream &ss, const std::string &class_name,
                                          const ::google::protobuf::MethodDescriptor *method,
                                          const std::string &name_prefix) const {
    std::string macro_str = method->options().GetExtension(WITH_MACRO);
    ADD_IFDEF(macro_str, ss);
    bool is_broadcast = method->options().GetExtension(IS_BROADCAST);
    uint32_t broadcast_world = method->options().GetExtension(WORLD_BROADCAST);
    if (is_broadcast || broadcast_world) {
        std::cerr << "not support the option currently|options:IS_BROADCAST,WORLD_BROADCAST" << std::endl;
    } else {
        uint32_t timeout = g_default_timeout;
        if (method->options().HasExtension(TIME_OUT))
            timeout = method->options().GetExtension(TIME_OUT);

        std::size_t indent_num = class_name.size() + method->name().size() + 17UL;
        ss << "mt::Task<int> " << class_name << "::" << method->name() << "(std::uint64_t uid,\n"
           << std::string(indent_num, ' ') << "const " << method->input_type()->name() << "& request, "
           << method->output_type()->name() << "* response,\n"
           << std::string(indent_num, ' ') << "std::uint32_t timeout) {\n"
           << g_indent << "if (timeout == 0U)\n"
           << g_indent << g_indent << "timeout = " << std::dec << timeout << "U;\n"
           << g_indent << "co_return co_await RpcServiceMgr::GetInst().Rpc(0x" << std::hex
           << method->options().GetExtension(RPC_CMD) << "U, uid, request, response, timeout);\n";
    }
    ss << "}\n";
    ADD_ENDIF(macro_str, ss);
}
