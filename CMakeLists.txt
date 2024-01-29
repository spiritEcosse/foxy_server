cmake_minimum_required(VERSION 3.22)
project(foxy_server)

set(BUILD_EXAMPLES OFF)
set(BUILD_CTL OFF)
#set(BUILD_TESTING OFF)

set(CMAKE_CXX_STANDARD 20)

get_cmake_property(_variableNames VARIABLES)
list(SORT _variableNames)
foreach(_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

execute_process(COMMAND bash -c "cp -f config.json ${PROJECT_BINARY_DIR}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE COPY_CONFIG)

if (NOT COPY_CONFIG EQUAL "0")
    message(FATAL_ERROR "cp config.json failed with : ${COPY_CONFIG}, please check")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    link_directories(/usr/lib/x86_64-linux-gnu/)
endif()

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
        VERSION 1.9.0
        GITHUB_REPOSITORY drogonframework/drogon
        GIT_TAG v1.9.0
)

# start https://github.com/an-tao/trantor/issues/296
set(EMPTY_STRING "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(EMPTY_STRING "''")
endif()

execute_process(COMMAND bash -c "sed -i ${EMPTY_STRING} '396s/.*(bool)logger;//' ${PROJECT_BINARY_DIR}/_deps/drogon-src/trantor/trantor/utils/Logger.cc"
        RESULT_VARIABLE FIX_WARNINGS)

if (NOT FIX_WARNINGS EQUAL "0")
    message(FATAL_ERROR "sed ,removing line 396, failed with : ${FIX_WARNINGS}, please check")
endif()

execute_process(COMMAND bash -c "sed -i ${EMPTY_STRING} '387s/void Logger::enableSpdLog(int index, std::shared_ptr<spdlog::logger> logger)/void Logger::enableSpdLog(int index, [[maybe_unused]] std::shared_ptr<spdlog::logger> logger)/' ${PROJECT_BINARY_DIR}/_deps/drogon-src/trantor/trantor/utils/Logger.cc"
        RESULT_VARIABLE FIX_WARNINGS)

if (NOT FIX_WARNINGS EQUAL "0")
    message(FATAL_ERROR "sed, changing 387 line, failed with : ${FIX_WARNINGS}, please check")
endif()
# end https://github.com/an-tao/trantor/issues/296

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
        VERSION V2.0_NODEBCRYPT
        GITHUB_REPOSITORY hilch/Bcrypt.cpp
        GIT_TAG V2.0_NODEBCRYPT
)
if (BUILD_TESTING)
    execute_process(COMMAND bash -c "sed -i ${EMPTY_STRING} '11,12s/^/#/' ${PROJECT_BINARY_DIR}/_deps/bcrypt-src/CMakeLists.txt"
            RESULT_VARIABLE EXCLUDE_BASH_SCRIPT)
    if (NOT EXCLUDE_BASH_SCRIPT EQUAL "0")
        message(FATAL_ERROR "sed failed with : ${EXCLUDE_BASH_SCRIPT}, please check")
    endif()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt)
# End adding hilch/Bcrypt.cpp

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
endif()
# End Tests
