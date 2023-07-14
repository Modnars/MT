cmake_minimum_required(VERSION 3.17)

set(PROJECT_VERSION 1.0.0)
set(TARGET_NAME "mt")

project(${TARGET_NAME}
        VERSION ${PROJECT_VERSION}
        LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -Wall -Werror -g")
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
add_definitions(-DENABLE_CXX20_COROUTINE=1)

file(GLOB_RECURSE MT_INCLUDE include/*.h)
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
