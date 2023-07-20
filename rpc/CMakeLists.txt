set(RPC_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROTOCOL_DIR "${RPC_ROOT_DIR}/protocol")
set(PROTOCOL_COMMON_DIR "${PROTOCOL_DIR}/common")
set(PROTOCOL_INNER_DIR "${PROTOCOL_DIR}/inner")

set(PROTOBUF_ROOT_DIR "${3RD_DIR}/protobuf-3.20.3")
set(PROTOC_TOOL "${PROTOBUF_ROOT_DIR}/bin/protoc")  # 其他平台构建时需要修改 PROTOC 路径

add_subdirectory(demo)
add_subdirectory(tools)
