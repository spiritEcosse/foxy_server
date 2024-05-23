cmake_minimum_required(VERSION 3.22)
project(foxy_server)

set(BUILD_EXAMPLES OFF)
set(BUILD_CTL OFF)
#set(BUILD_TESTING OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# start https://github.com/an-tao/trantor/issues/296
set(EMPTY_STRING "")
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(EMPTY_STRING "''")
    set(CMAKE_BUILD_RPATH "${CMAKE_LLVM_LIB_DIR}")
endif ()

set(CMAKE_CXX_STANDARD 20)

get_cmake_property(_variableNames VARIABLES)
list(SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach ()

add_definitions(-DCMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}")

execute_process(COMMAND bash -c "cp -f config.json ${PROJECT_BINARY_DIR}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE COPY_CONFIG)

if (NOT COPY_CONFIG EQUAL "0")
    message(FATAL_ERROR "cp config.json failed with : ${COPY_CONFIG}, please check")
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    link_directories(/usr/lib/x86_64-linux-gnu/)
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty" "${CMAKE_BINARY_DIR}/_deps/cmake-scripts-src/")
include(3rdparty)

# start src
file(GLOB_RECURSE RESOURCES CONFIGURE_DEPENDS src/*.cc src/*.cpp /src/*.h)
add_executable(foxy_server ${RESOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
# end src

set(CMAKE_SYSTEM_PROCESSOR arm) # why this?

# start drogon
CPMAddPackage(
        NAME drogon
        GITHUB_REPOSITORY drogonframework/drogon
        GIT_TAG master
)
target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
# end drogon

# Start adding jwt-cpp
CPMAddPackage(
        NAME jwt-cpp
        VERSION 0.7.0
        GITHUB_REPOSITORY Thalhammer/jwt-cpp
        GIT_TAG v0.7.0
)
target_link_libraries(${PROJECT_NAME} PRIVATE jwt-cpp)
# End adding jwt-cpp

# Start adding hilch/Bcrypt.cpp

CPMAddPackage(
        NAME bcrypt
        GITHUB_REPOSITORY hilch/Bcrypt.cpp
        GIT_TAG master
)

target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt)
# End adding hilch/Bcrypt.cpp

# start sentry
set(SENTRY_BACKEND crashpad)

CPMAddPackage(
        NAME sentry
        VERSION 0.7.0
        GITHUB_REPOSITORY getsentry/sentry-native
        GIT_TAG 0.7.0
        OPTIONS "SENTRY_BACKEND=${SENTRY_BACKEND}"
)
execute_process(COMMAND bash -c "sed -n '191p' ${PROJECT_BINARY_DIR}/_deps/sentry-src/external/crashpad/snapshot/CMakeLists.txt | tr -d '\n'"
        OUTPUT_VARIABLE LINE_191_CONTENT)

if ("${LINE_191_CONTENT}" STREQUAL "")
    execute_process(COMMAND bash -c "sed -i ${EMPTY_STRING} '191i\\
target_sources(crashpad_snapshot PRIVATE x86/cpuid_reader.cc x86/cpuid_reader.h)' ${PROJECT_BINARY_DIR}/_deps/sentry-src/external/crashpad/snapshot/CMakeLists.txt"
            RESULT_VARIABLE FIX_MATCHING_ARCHITECTURE_SENTRY)
    message(STATUS "FIX_MATCHING_ARCHITECTURE_SENTRY : ${FIX_MATCHING_ARCHITECTURE_SENTRY}")
    if (NOT FIX_MATCHING_ARCHITECTURE_SENTRY EQUAL "0")
        message(FATAL_ERROR "sed, changing architecture matching in sentry failed with : ${FIX_MATCHING_ARCHITECTURE_SENTRY}, please check")
    endif ()
endif ()
target_link_libraries(${PROJECT_NAME} PRIVATE sentry)
# end sentry


# Start Formatting
CPMAddPackage(
        NAME cmake-scripts
        VERSION 23.09
        GITHUB_REPOSITORY StableCoder/cmake-scripts
        GIT_TAG 23.09
)
include(formatting)

file(GLOB_RECURSE ALL_CODE_FILES ${PROJECT_SOURCE_DIR}/src/*.cc ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/*.h ${PROJECT_SOURCE_DIR}/tests/*.cc
        ${PROJECT_SOURCE_DIR}/tests/*.h)

clang_format(format ${ALL_CODE_FILES})

file(GLOB_RECURSE CMAKE_FILES ${PROJECT_SOURCE_DIR}/src/CMakeLists.txt
        ${PROJECT_SOURCE_DIR}/tests/CMakeLists.txt)

cmake_format(cmake-format ${CMAKE_FILES} CMakeLists.txt)

# End Formatting

# Start adding fmt
CPMAddPackage(
        NAME fmt
        VERSION 10.2.1
        GITHUB_REPOSITORY fmtlib/fmt
        GIT_TAG 10.2.1
)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
# End adding fmt

# Tests
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif ()
# End Tests
