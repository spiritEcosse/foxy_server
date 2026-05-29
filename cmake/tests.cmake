option(ENABLE_TESTS "Enable testing support" OFF)

if(ENABLE_TESTS)
    message(STATUS "Testing is enabled.")
    CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.17.0
        VERSION 1.17.0
        OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt ON"
    )
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Testing is disabled.")
endif()
