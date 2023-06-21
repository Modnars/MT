cmake_minimum_required(VERSION 3.17)

set(PROJECT_VERSION 1.0.0)
set(TARGET_NAME "mt")

project(${TARGET_NAME}
        VERSION ${PROJECT_VERSION}
        LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -Wall -Werror")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
message(CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
        # For clangd index GNU header files.
        set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MT_INCLUDE
        include/mt/concept/awaitable.h
        include/mt/concept/future.h
        include/mt/concept/promise.h
        include/mt/concept/schedulable.h
        include/mt/net/addrinfo_guard.h
        include/mt/net/connect.h
        include/mt/net/server.h
        include/mt/net/stream.h
        include/mt/selector/event.h
        include/mt/selector/selector_epoll.h
        include/mt/selector/selector_kqueue.h
        include/mt/selector/selector.h
        include/mt/util/macros.h
        include/mt/util/noncopyable.h
        include/mt/call_stack.h
        include/mt/event_loop.h
        include/mt/exception.h
        include/mt/gather.h
        include/mt/handle.h
        include/mt/result.h
        include/mt/sleep.h
        include/mt/task.h
        include/mt/void_value.h
        include/mt/wait_for.h
        )
set(3RD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rd")

include_directories(${CMAKE_SOURCE_DIR}/include)
add_library(${TARGET_NAME} SHARED
        ${MT_INCLUDE}
        src/event_loop.cc
        )
target_link_libraries(${TARGET_NAME} PUBLIC fmt::fmt)

add_subdirectory(test)
add_subdirectory(3rd)
add_subdirectory(rpc)

set(MODELS_CMAKE_FILES
    "cmake/mtConfig.cmake"
    "cmake/mtConfigVersion.cmake"
)
