cmake_minimum_required(VERSION 3.22)
project(foxy_server)

# Set CMP0115 policy to NEW
cmake_policy(SET CMP0115 NEW)

set(BUILD_EXAMPLES OFF)
set(BUILD_CTL OFF)
set(BUILD_TESTING OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# start https://github.com/an-tao/trantor/issues/296
set(EMPTY_STRING "")
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(EMPTY_STRING "''")
    set(CMAKE_BUILD_RPATH "${CMAKE_LLVM_LIB_DIR}")
endif ()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

CPMAddPackage(
        NAME cmake-scripts
        VERSION 24.08.1
        GITHUB_REPOSITORY StableCoder/cmake-scripts
        GIT_TAG 24.08.1
)
#include(code-coverage)
#add_code_coverage_all_targets(EXCLUDE tests/* _deps/*) # Adds the 'ccov-all' target set and sets it to exclude all files in test/ folders.

# start src
file(GLOB_RECURSE RESOURCES CONFIGURE_DEPENDS src/*.*)
add_executable(foxy_server ${RESOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
# end src

include_directories(${CMAKE_SOURCE_DIR}/src/code/orm)
include_directories(${CMAKE_SOURCE_DIR}/src/code/utils/db)
include_directories(${CMAKE_SOURCE_DIR}/src/code/utils)
include_directories(${CMAKE_SOURCE_DIR}/src/code/models)
include_directories(${CMAKE_SOURCE_DIR}/src/code/utils/jwt)
include_directories(${CMAKE_SOURCE_DIR}/src/code/utils/exceptions)
include_directories(${CMAKE_SOURCE_DIR}/src/code/utils/request)
include_directories(${CMAKE_SOURCE_DIR}/src/code/controllers)
include_directories(${CMAKE_SOURCE_DIR}/src/code/clients)
include_directories(${CMAKE_SOURCE_DIR}/src/code/clients/models)
include_directories(${CMAKE_SOURCE_DIR}/src/code/auth)
include_directories(${CMAKE_SOURCE_DIR}/src/code/filters)
include_directories(${CMAKE_SOURCE_DIR}/src/code/sentry_catcher)

include(${CMAKE_CURRENT_SOURCE_DIR}/envs.cmake)

#Start adding libuuid
CPMAddPackage(
        NAME libuuid
        GIT_REPOSITORY https://github.com/karelzak/util-linux.git
        GIT_TAG v2.40.2
)
find_library(UUID_LIBRARY uuid)
if (NOT UUID_LIBRARY)
    message(FATAL_ERROR "libuuid not found")
endif ()
target_link_libraries(${PROJECT_NAME} PRIVATE ${UUID_LIBRARY})
# End adding libuuid


# start drogon
CPMAddPackage(
        NAME drogon
        VERSION v1.9.8
        GITHUB_REPOSITORY drogonframework/drogon
        GIT_TAG v1.9.4
        OPTIONS "BUILD_TESTING OFF"
)

target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
# end drogon

# Start adding cpr
CPMAddPackage(
        NAME cpr
        VERSION 1.11.0
        GITHUB_REPOSITORY libcpr/cpr
        GIT_TAG 1.11.0
)
target_link_libraries(${PROJECT_NAME} PRIVATE cpr)
# End adding cpr

# Find zlib library
find_library(ZLIB_LIBRARY NAMES z)

# Check if zlib was found
if (NOT ZLIB_LIBRARY)
    message(FATAL_ERROR "zlib not found")
endif ()

# Link zlib to your target
target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARY})

# Find libbfd
find_library(LIBBFD_FOUND bfd PATHS ${PATH_LIBBFD})
if (NOT LIBBFD_FOUND)
    message(FATAL_ERROR " libbfd not found")
endif ()

# Find libunwind
find_library(LIBUNWIND_FOUND unwind)
if (NOT LIBUNWIND_FOUND)
    message(FATAL_ERROR "libunwind not found")
endif ()


#find_library(OAUTH_FOUND oauth)
#if (NOT OAUTH_FOUND)
#    message(FATAL_ERROR "oauth not found")
#endif ()

#target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBBFD_FOUND} ${LIBUNWIND_FOUND} ${OAUTH_FOUND})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBBFD_FOUND} ${LIBUNWIND_FOUND})

if (DEFINED SENTRY_DSN)
    # start adding sentry
    set(SENTRY_BACKEND crashpad)

    CPMAddPackage(
            NAME sentry
            VERSION 0.7.10
            GITHUB_REPOSITORY getsentry/sentry-native
            GIT_TAG 0.7.10
            OPTIONS "SENTRY_BACKEND=${SENTRY_BACKEND}"
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE sentry)
    # end sentry
endif ()


include(sanitizers)
if (ENVIRONMENT STREQUAL "dev")
    # Address + Leak sanitizers
    add_sanitizer_support(address leak undefined)
    target_link_libraries(${PROJECT_NAME} PRIVATE -fsanitize=address)

    # Thread + Undefined Behavior sanitizers
    #    add_sanitizer_support(thread undefined)

    # Memory + Undefined Behavior sanitizers
    #    add_sanitizer_support(memory undefined)
endif ()

#target_code_coverage(${PROJECT_NAME} AUTO ALL EXCLUDE tests/* _deps/*) # As an executable target, adds to the 'ccov' and ccov-all' targets, and the reports will exclude the non-covered.cpp file, and any files in a test/ folder.

# Start Formatting
include(formatting)
clang_format(clang-format ${RESOURCES} tests/*.*)
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

option(ENABLE_TESTS "Enable testing support" OFF)

if (ENABLE_TESTS)
    message(STATUS "Testing is enabled.")
    # Start adding gtest
    CPMAddPackage(
            NAME googletest
            GITHUB_REPOSITORY google/googletest
            GIT_TAG v1.15.2
            VERSION 1.15.2
            OPTIONS
            "INSTALL_GTEST OFF"
            "gtest_force_shared_crt ON"
    )
    set(GTEST_PARALLEL TRUE)
    # End adding gtest
    enable_testing()
    add_subdirectory(tests)
else ()
    message(STATUS "Testing is disabled.")
endif ()
