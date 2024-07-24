find_package(Git)

include(ProcessorCount)
ProcessorCount(CORES)

if (NOT EXISTS ${PROJECT_BINARY_DIR}/3rdparty)
    execute_process(COMMAND bash -c "mkdir ${PROJECT_BINARY_DIR}/3rdparty")
endif ()

if (EXISTS "${PROJECT_SOURCE_DIR}/.git")
    message(STATUS "========================Submodule update========================")
    execute_process(COMMAND bash -c "curl https://raw.githubusercontent.com/spiritEcosse/aws-sailfish-sdk/master/install.sh | bash -s -- --func=git_submodule_checkout"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT)

    if (NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git_submodule_checkout failed with : ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif ()
    message(STATUS "========================End of Submodule update========================")

    include(3rdparty/cpm_cmake/cmake/CPM.cmake)

    set(LIB_DECIMAL_FOR_CPP decimal_for_cpp)
    unset(LIB_DECIMAL_FOR_CPP_INCLUDE_DIR CACHE)
    add_subdirectory(3rdparty/${LIB_DECIMAL_FOR_CPP})
    set(LIB_DECIMAL_FOR_CPP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/${LIB_DECIMAL_FOR_CPP}/include
            CACHE PATH "decimal_for_cpp include directory")
    message(STATUS "Decimal include directory: ${LIB_DECIMAL_FOR_CPP_INCLUDE_DIR}")
    include_directories(${LIB_DECIMAL_FOR_CPP_INCLUDE_DIR})

    set(LIB_BACKWARD_CPP backward-cpp)
    unset(LIB_BACKWARD_CPP_INCLUDE_DIR CACHE)
    add_subdirectory(3rdparty/${LIB_BACKWARD_CPP})
endif ()

