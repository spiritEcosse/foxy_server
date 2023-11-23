cmake_minimum_required(VERSION 3.24)
project(foxy_server)


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

target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
