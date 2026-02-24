project(foxy_tests LANGUAGES CXX)
file(GLOB_RECURSE RESOURCES CONFIGURE_DEPENDS ../src/code/*.* *.cpp *.h)

add_executable(${PROJECT_NAME} ${RESOURCES})
include(GoogleTest)
gtest_add_tests(TARGET ${PROJECT_NAME} TEST_LIST myTests_targets)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE drogon
            fmt::fmt
            cpr
            gtest
            gtest_main
            gmock
            gmock_main
            pthread
            unistring)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(
    ${PROJECT_NAME} PRIVATE ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    TEST_MIGRATIONS_DIR="${CMAKE_SOURCE_DIR}/migrations"
    TEST_FIXTURES_PATH="${CMAKE_SOURCE_DIR}/fixtures.sql"
)
