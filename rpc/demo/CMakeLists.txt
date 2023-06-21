PROJECT(rpc-demo)
cmake_minimum_required(VERSION 3.17)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    ${3RD_DIR}/protobuf-2.6.1/include
    ${3RD_DIR}/llbc/llbc/include)
link_directories(
    ${3RD_DIR}/protobuf-2.6.1/lib
    ${3RD_DIR}/llbc/output/gmake/release64)

# 生成协议 proto 的 .pb.h 和 .pb.cc 文件
set(PROTOC_TOOL ${3RD_DIR}/protobuf-2.6.1/bin/protoc)  # 其他平台构建时需要修改 PROTOC 路径
set(PROTOCOL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/protocol")
file(GLOB protocol_file_list ${PROTOCOL_DIR}/*.proto)
foreach(_file ${protocol_file_list})
    string(REPLACE ".proto" ".pb.h" file_h ${_file})
    string(REPLACE ".proto" ".pb.cc" file_cc ${_file})
	get_filename_component(_dir ${_file} DIRECTORY)
    add_custom_command(
            OUTPUT ${file_cc} ${file_h}
            COMMAND ${PROTOC_TOOL} --cpp_out=${_dir} --proto_path=${PROTOCOL_DIR} ${_file}
            DEPENDS ${_file} 
            COMMENT "Generating protobuf files for ${_file}"
    )
    list(APPEND PB_FILES ${file_cc})
endforeach()

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
# file(GLOB PB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/protocol/*.cc")
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
# set(LIBMYRPC_SRC ./src/rpc_meta.pb.cc)
# add_library(myrpc ${LIBMYRPC_SRC})
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
add_executable(client ${PB_FILES} ${SRC_FILES} ./src/client/client.cpp)
target_link_libraries(client libprotobuf.a libllbc.so)
add_executable(server ${PB_FILES} ${SRC_FILES} ./src/server/server.cpp)
target_link_libraries(server libprotobuf.a libllbc.so)
