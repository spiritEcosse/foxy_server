cmake_minimum_required(VERSION 3.22)
project(foxy_server)

set(BUILD_EXAMPLES OFF)
set(BUILD_CTL OFF)
set(BUILD_TESTING OFF)

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

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty")
include(3rdparty)
add_executable(foxy_server main.cpp gallery_ItemController.h gallery_ItemController.cc gallery_ItemController.cc gallery_PageController.h gallery_PageController.cc Page.h Image.cc Image.h Item.cc Item.h Page.cc)
set(CMAKE_SYSTEM_PROCESSOR arm)
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
