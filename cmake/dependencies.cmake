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
    VERSION 1.9.12
    GITHUB_REPOSITORY drogonframework/drogon
    GIT_TAG v1.9.12
    OPTIONS "BUILD_TESTING OFF"
)

# Drogon/trantor compile with -Werror; newer clang flags std::future::get() as
# [[nodiscard]] in syncTaskInQueue. Demote that one warning for upstream targets.
foreach(_t trantor drogon)
    if(TARGET ${_t})
        target_compile_options(${_t} PRIVATE -Wno-error=unused-result)
    endif()
endforeach()

# cpr — disable libpsl (cookie public-suffix validation); we only do bearer-token API calls
set(CPR_CURL_USE_LIBPSL OFF CACHE BOOL "Disable libpsl in bundled curl" FORCE)
# cpr's bundled curl auto-links brotli/zstd if present on the system. We don't request
# br/zstd encodings on outbound API calls, so disable to keep the dep surface minimal.
set(CURL_BROTLI OFF CACHE BOOL "Disable Brotli in bundled curl" FORCE)
set(CURL_ZSTD OFF CACHE BOOL "Disable Zstd in bundled curl" FORCE)
CPMAddPackage(NAME cpr VERSION 1.14.2 GITHUB_REPOSITORY libcpr/cpr GIT_TAG 1.14.2)

# fmt
CPMAddPackage(NAME fmt VERSION 12.1.0 GITHUB_REPOSITORY fmtlib/fmt GIT_TAG 12.1.0)

# zlib
find_library(ZLIB_LIBRARY NAMES z)
if(NOT ZLIB_LIBRARY)
    message(FATAL_ERROR "zlib not found")
endif()

# libunistring — required by libidn2 (curl dependency)
find_library(UNISTRING_LIBRARY NAMES unistring)
if(NOT UNISTRING_LIBRARY)
    message(FATAL_ERROR "libunistring not found")
endif()
