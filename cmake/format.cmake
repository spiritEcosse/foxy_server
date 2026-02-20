# Note: CONFIGURE_DEPENDS is intentionally omitted from both globs.
# Using it caused an infinite cmake re-run loop: the glob matched *.cmake files
# inside .cache/CPM/, which CPM populates during configure. Each new package
# added new files there, making the glob result change on every build and
# triggering another cmake re-run. Without CONFIGURE_DEPENDS, new source files
# added after configure won't appear in the format target until cmake is re-run manually.

# C++ sources — grab everything, exclude build/ and .cache/
file(
    GLOB_RECURSE FORMAT_CPP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cppm"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
list(FILTER FORMAT_CPP_SOURCES EXCLUDE REGEX ".*/build/.*|.*/.cache/.*")

# CMake files — grab everything, exclude build/ and .cache/
file(
    GLOB_RECURSE FORMAT_CMAKE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")
list(FILTER FORMAT_CMAKE_FILES EXCLUDE REGEX ".*/build/.*|.*/.cache/.*")

include(${cmake-scripts_SOURCE_DIR}/formatting.cmake)
clang_format(clang-format ${FORMAT_CPP_SOURCES})
cmake_format(cmake-format ${FORMAT_CMAKE_FILES})
