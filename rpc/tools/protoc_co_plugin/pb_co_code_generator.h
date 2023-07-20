/*
 * @Author: modnarshen
 * @Date: 2023.07.20 11:47:44
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _PB_CO_CODE_GENERATOR_H
#define _PB_CO_CODE_GENERATOR_H 1

#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>

namespace pb_compiler = ::google::protobuf::compiler;

class PbCoCodeGenerator : public pb_compiler::CodeGenerator {
public:
    virtual ~PbCoCodeGenerator() { }
    virtual bool Generate(const ::google::protobuf::FileDescriptor *file, const std::string &parameter,
                          pb_compiler::GeneratorContext *context, std::string *error) const {
        if (file->service_count() == 0)
            return true;

        auto base_name = BaseName(file->name());
        std::unique_ptr<::google::protobuf::io::ZeroCopyOutputStream> header_output(
            context->OpenForInsert(base_name + ".pb.h", "namespace_scope"));
        std::unique_ptr<::google::protobuf::io::ZeroCopyOutputStream> header_include_out_put(
            context->OpenForInsert(base_name + ".pb.h", "includes"));
        std::unique_ptr<::google::protobuf::io::ZeroCopyOutputStream> source_output(
            context->OpenForInsert(base_name + ".pb.cc", "namespace_scope"));
        std::unique_ptr<::google::protobuf::io::ZeroCopyOutputStream> source_include_out_put(
            context->OpenForInsert(base_name + ".pb.cc", "includes"));

        ::google::protobuf::io::CodedOutputStream header_out(header_output.get());
        ::google::protobuf::io::CodedOutputStream header_include_out(header_include_out_put.get());
        ::google::protobuf::io::CodedOutputStream source_out(source_output.get());
        ::google::protobuf::io::CodedOutputStream source_include_out(source_include_out_put.get());

        std::set<std::string> need_head_file;
        CollectHeadFile(file, need_head_file);

        header_include_out.WriteString(GetHeaderIncludeStr(parameter, need_head_file));
        header_out.WriteString(GenRpcDecl(file));

        source_include_out.WriteString(GetSourceIncludeStr(parameter, need_head_file));
        source_out.WriteString(GenRpcImpl(file));

        return true;
    }

private:
    static std::string BaseName(const std::string &str) {
        std::stringstream input(str);
        std::string temp;
        std::getline(input, temp, '.');
        return temp;
    }

    static void CollectHeadFile(const ::google::protobuf::FileDescriptor *file, std::set<std::string> &need_head_file);

    std::string GetHeaderIncludeStr(const std::string &base_path, const std::set<std::string> &need_head_file) const;
    std::string GetSourceIncludeStr(const std::string &base_path, const std::set<std::string> &need_head_file) const;

    std::string GenRpcDecl(const ::google::protobuf::FileDescriptor *file) const;
    std::string GenRpcImpl(const ::google::protobuf::FileDescriptor *file) const;

    void GenServiceDecl(std::stringstream &ss, const ::google::protobuf::ServiceDescriptor *service,
                        const std::string &name_prefix) const;
    void GenServiceImpl(std::stringstream &ss, const ::google::protobuf::ServiceDescriptor *service,
                        const std::string &name_prefix) const;

    void GenServiceProtectedMethodDecl(std::stringstream &ss, const ::google::protobuf::MethodDescriptor *method,
                                       bool is_async, const std::string &name_prefix) const;
    void GenServicePrivateMethodDecl(std::stringstream &ss, const ::google::protobuf::MethodDescriptor *method) const;
    void GenServiceMethodImpl(std::stringstream &ss, const std::string &class_name,
                              const ::google::protobuf::MethodDescriptor *method, bool is_async,
                              const std::string &name_prefix) const;

    void GenStubDecl(std::stringstream &ss, const ::google::protobuf::ServiceDescriptor *service,
                     const std::string &name_prefix) const;
    void GenStubImpl(std::stringstream &ss, const ::google::protobuf::ServiceDescriptor *service,
                     const std::string &name_prefix) const;

    void GenStubMethodDecl(std::stringstream &ss, const ::google::protobuf::MethodDescriptor *method,
                           const std::string &name_prefix) const;
    void GenStubAsyncMethodDecl(std::stringstream &ss, const ::google::protobuf::MethodDescriptor *method,
                                const std::string &name_prefix) const;
    void GenStubMethodImpl(std::stringstream &ss, const std::string &class_name,
                           const ::google::protobuf::MethodDescriptor *method, const std::string &name_prefix) const;
    void GenStubAsyncMethodImpl(std::stringstream &ss, const std::string &class_name,
                                const ::google::protobuf::MethodDescriptor *method,
                                const std::string &name_prefix) const;
};

#endif  // _PB_CO_CODE_GENERATOR_H
