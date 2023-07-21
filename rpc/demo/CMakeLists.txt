PROJECT(rpc-demo)
cmake_minimum_required(VERSION 3.17)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROTOCOL_DIR}/common
    ${PROTOCOL_DIR}/inner
    ${PROTOBUF_ROOT_DIR}/include
    ${3RD_DIR}/llbc/llbc/include)
link_directories(
    ${PROTOBUF_ROOT_DIR}/lib
    ${3RD_DIR}/llbc/output/gmake/release64)

# 生成协议 proto 的 .pb.h 和 .pb.cc 文件
file(GLOB protocol_file_list ${PROTOCOL_COMMON_DIR}/*.proto ${PROTOCOL_INNER_DIR}/*.proto)
foreach(_file ${protocol_file_list})
    string(REPLACE ".proto" ".pb.h" file_h ${_file})
    string(REPLACE ".proto" ".pb.cc" file_cc ${_file})
	get_filename_component(_dir ${_file} DIRECTORY)
    add_custom_command(
            OUTPUT ${file_cc} ${file_h}
            COMMAND ${PROTOC_TOOL} --cpp_out=${_dir} --rpc_out=${_dir} --proto_path=${PROTOCOL_COMMON_DIR}:${PROTOCOL_INNER_DIR} --plugin=protoc-gen-rpc=${PROTOC_RPC_PLUGIN} ${_file}
            DEPENDS ${_file} ${PROTOC_RPC_PLUGIN}
            COMMENT "Generating protobuf files for ${_file} ${COMMAND}"
    )
    list(APPEND PB_FILES ${file_cc})
endforeach()

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
# file(GLOB PB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/protocol/*.cc")
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
# set(LIBMYRPC_SRC ./src/rpc_meta.pb.cc)
# add_library(myrpc ${LIBMYRPC_SRC})
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
add_executable(client ${PB_FILES} ${SRC_FILES} ./client/client.cpp)
target_link_libraries(client libprotobuf.a libllbc.so fmt::fmt mt)
add_executable(server ${PB_FILES} ${SRC_FILES} ./server/server.cpp)
target_link_libraries(server libprotobuf.a libllbc.so fmt::fmt mt)
add_executable(server2 ${PB_FILES} ${SRC_FILES} ./server/server2.cpp)
target_link_libraries(server2 libprotobuf.a libllbc.so fmt::fmt mt)
