PROJECT(rpc-demo)
cmake_minimum_required(VERSION 3.2)

include_directories(
    ./src
    ../protobuf-2.6.1/include
    ../llbc/llbc/include)
link_directories(
    ../protobuf-2.6.1/lib
    ../llbc/output/gmake/release64
    ./build)

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
file(GLOB PB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/pb/*.cc")
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
# set(LIBMYRPC_SRC ./src/rpc_meta.pb.cc)
# add_library(myrpc ${LIBMYRPC_SRC})
add_executable(client ${PB_FILES} ${SRC_FILES} ./src/client/client.cpp)
target_link_libraries(client libprotobuf.a  libllbc.so)
add_executable(server ${PB_FILES} ${SRC_FILES} ./src/server/server.cpp)
target_link_libraries(server libprotobuf.a libllbc.so)
