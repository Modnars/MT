project(protoc_co_plugin)
cmake_minimum_required(VERSION 3.17)

file(GLOB_RECURSE SOURCES *.cc)

set(proto_list
    ${PROTOCOL_INNER_DIR}/cmd.proto
    ${PROTOCOL_INNER_DIR}/rpc_options.proto
    )
foreach(rpc_pb_file ${proto_list})
    get_filename_component(proto_file_prefix ${rpc_pb_file} NAME_WE)
    set(rpc_file_cc ${PROTOCOL_INNER_DIR}/${proto_file_prefix}.pb.cc)
    set(rpc_file_h ${PROTOCOL_INNER_DIR}/${proto_file_prefix}.pb.h)
    add_custom_command(
        OUTPUT ${rpc_file_cc} ${rpc_file_h}
        COMMAND ${PROTOC_TOOL} --cpp_out=${PROTOCOL_INNER_DIR} --proto_path=${PROTOCOL_INNER_DIR} ${rpc_pb_file}
        DEPENDS ${rpc_pb_file}
        COMMENT "Generating protobuf files for ${rpc_pb_file}"
        )
    list(APPEND SOURCES ${rpc_file_cc})
endforeach()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROTOCOL_DIR}/inner
    ${PROTOBUF_ROOT_DIR}/include
    )
link_directories(${PROTOBUF_ROOT_DIR}/lib)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    libprotoc.a
    libprotobuf.a
    pthread
    )

set(EXECUTABLE_OUTPUT_PATH "${TOOLS_DIR}/bin")
