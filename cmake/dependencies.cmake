# jsoncpp — must come before drogon (built from source for libc++ ABI compatibility)
CPMAddPackage(
    NAME jsoncpp
    VERSION 1.9.6
    GITHUB_REPOSITORY open-source-parsers/jsoncpp
    GIT_TAG 1.9.6
    OPTIONS
    "JSONCPP_WITH_TESTS OFF"
    "JSONCPP_WITH_POST_BUILD_UNITTEST OFF"
    "BUILD_SHARED_LIBS OFF"
)
set(JSONCPP_INCLUDE_DIRS "${jsoncpp_SOURCE_DIR}/include" CACHE PATH "" FORCE)
set(JSONCPP_LIBRARIES jsoncpp_static CACHE STRING "" FORCE)
set(Jsoncpp_FOUND TRUE CACHE BOOL "" FORCE)
if(NOT TARGET Jsoncpp_lib)
    add_library(Jsoncpp_lib INTERFACE IMPORTED GLOBAL)
    target_link_libraries(Jsoncpp_lib INTERFACE jsoncpp_static)
    target_include_directories(Jsoncpp_lib INTERFACE "${jsoncpp_SOURCE_DIR}/include")
endif()

# drogon
CPMAddPackage(
    NAME drogon
    VERSION v1.9.8
    GITHUB_REPOSITORY drogonframework/drogon
    GIT_TAG v1.9.4
    OPTIONS "BUILD_TESTING OFF"
)

# cpr
CPMAddPackage(NAME cpr VERSION 1.11.0 GITHUB_REPOSITORY libcpr/cpr GIT_TAG 1.11.0)

# fmt
CPMAddPackage(NAME fmt VERSION 12.1.0 GITHUB_REPOSITORY fmtlib/fmt GIT_TAG 12.1.0)

# decimal_for_cpp (header-only)
CPMAddPackage(NAME decimal_for_cpp GITHUB_REPOSITORY vpiotr/decimal_for_cpp GIT_TAG 599372e DOWNLOAD_ONLY YES)
include_directories(${decimal_for_cpp_SOURCE_DIR}/include)

# libuuid — system library, installed via uuid-dev in Dockerfile.build
find_library(UUID_LIBRARY uuid)
if(NOT UUID_LIBRARY)
    message(FATAL_ERROR "libuuid not found")
endif()

# zlib
find_library(ZLIB_LIBRARY NAMES z)
if(NOT ZLIB_LIBRARY)
    message(FATAL_ERROR "zlib not found")
endif()
