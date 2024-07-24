message(STATUS "========================Start of ${LIB_BACKWARD_CPP} ========================")

add_subdirectory(backward-cpp)
# Add Backward to your target (either Backward::Interface, Backward::Object, or Backward::Backward)
target_link_libraries(${PROJECT_NAME} PUBLIC Backward::Interface)

message(STATUS "========================End of ${LIB_BACKWARD_CPP} ========================")
