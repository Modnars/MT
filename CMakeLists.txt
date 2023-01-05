cmake_minimum_required(VERSION 3.18)

set(PROJECT_VERSION 1.0.0)
set(TARGET_NAME "mt")

project(${TARGET_NAME}
        VERSION ${PROJECT_VERSION}
        LANGUAGES CXX)

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
message(CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MT_INCLUDE
        include/mt/selector/event.h
        include/mt/selector/selector_epoll.h
        include/mt/selector/selector_kqueue.h
        include/mt/selector/selector.h
        include/mt/util/macro.h
        include/mt/util/noncopyable.h
        include/mt/event_loop.h
        include/mt/handle.h
        )

include_directories(${CMAKE_SOURCE_DIR}/include)
add_library(${TARGET_NAME} SHARED
        ${MT_INCLUDE}
        src/event_loop.cc
        )
target_link_libraries(${TARGET_NAME} PUBLIC fmt::fmt)

add_subdirectory(test)
add_subdirectory(3rd)

set(MODELS_CMAKE_FILES
    "cmake/mtConfig.cmake"
    "cmake/mtConfigVersion.cmake"
)
