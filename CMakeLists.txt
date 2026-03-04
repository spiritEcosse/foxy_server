cmake_minimum_required(VERSION 3.22)
project(foxy_server)
cmake_policy(SET CMP0115 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "This project requires Clang. Got: ${CMAKE_CXX_COMPILER_ID}")
endif()
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "21")
    message(FATAL_ERROR "Clang >= 21 required. Got: ${CMAKE_CXX_COMPILER_VERSION}")
endif()

add_compile_options(-ferror-limit=1 $<$<CONFIG:Debug>:-ftemplate-backtrace-limit=0>)

set(BUILD_EXAMPLES OFF)
set(BUILD_CTL OFF)
set(BUILD_TESTING OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-DCMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}")

# Git hooks
execute_process(
    COMMAND git config core.hooksPath .githooks
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    ERROR_QUIET)

# Package manager & dependencies
include(cmake/cpm.cmake)
include(cmake/cmake-scripts.cmake)
include(cmake/dependencies.cmake)

# Executable
file(GLOB_RECURSE RESOURCES src/*.*)
add_executable(foxy_server ${RESOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_SOURCE_DIR}/src/code)

target_precompile_headers(foxy_server PRIVATE <drogon/drogon.h>)

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE drogon cpr fmt::fmt ${ZLIB_LIBRARY} ${UNISTRING_LIBRARY})

# Post-target: sentry (conditional), formatting, sanitizers, tests
include(cmake/sentry.cmake)
include(cmake/format.cmake)
include(cmake/sanitizers.cmake)
include(cmake/tests.cmake)
