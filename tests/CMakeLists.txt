project(foxy_tests LANGUAGES CXX)
file(GLOB_RECURSE RESOURCES CONFIGURE_DEPENDS ../src/**/*.cc ../src/**/*.cpp ../src/**/*.h)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} ${RESOURCES} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE drogon fmt::fmt sentry cpr gtest
        gtest_main
        gmock
        gmock_main
        pthread  # Required on Linux
)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_CURRENT_SOURCE_DIR}/../envs.cmake)

# Make sure the include directories are set
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${GTEST_INCLUDE_DIRS}
        ${GMOCK_INCLUDE_DIRS}
)

ParseAndAddDrogonTests(${PROJECT_NAME})
execute_process(COMMAND bash -c "cp -f config.json ${PROJECT_BINARY_DIR}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE COPY_CONFIG)

if (NOT COPY_CONFIG EQUAL "0")
    message(FATAL_ERROR "cp config.json failed with : ${COPY_CONFIG}, please check")
endif ()

execute_process(COMMAND bash -c "cp -f ../helper.sql ${PROJECT_BINARY_DIR}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE COPY_CONFIG)

if (NOT COPY_CONFIG EQUAL "0")
    message(FATAL_ERROR "cp ../helper.sql failed with : ${COPY_CONFIG}, please check")
endif ()

execute_process(COMMAND bash -c "cp -f ../fixtures.sql ${PROJECT_BINARY_DIR}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE COPY_CONFIG)

if (NOT COPY_CONFIG EQUAL "0")
    message(FATAL_ERROR "cp ../fixtures.sql failed with : ${COPY_CONFIG}, please check")
endif ()
