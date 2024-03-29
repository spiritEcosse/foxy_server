project(create_user)

file(GLOB_RECURSE RESOURCES CONFIGURE_DEPENDS main.cpp)
add_executable(${PROJECT_NAME} ${RESOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
