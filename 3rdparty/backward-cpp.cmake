message(STATUS "========================Start of ${LIB_BACKWARD_CPP} ========================")

add_subdirectory(${LIB_BACKWARD_CPP})
# Assuming backward-cpp is located in the "3rdparty/backward-cpp" directory
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/${LIB_BACKWARD_CPP})

# Add Backward to your target (either Backward::Interface, Backward::Object, or Backward::Backward)
target_link_libraries(${PROJECT_NAME} PUBLIC Backward::Interface)

message(STATUS "========================End of ${LIB_BACKWARD_CPP} ========================")
