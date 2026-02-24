# SENTRY_DSN stays compile-time: it controls #if defined(SENTRY_DSN) conditional compilation
if(DEFINED ENV{SENTRY_DSN})
    set(SENTRY_DSN "$ENV{SENTRY_DSN}")
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DSENTRY_DSN="${SENTRY_DSN}")

    set(SENTRY_BACKEND crashpad)
    CPMAddPackage(
        NAME sentry
        VERSION 0.13.0
        GITHUB_REPOSITORY getsentry/sentry-native
        GIT_TAG 0.13.0
        OPTIONS "SENTRY_BACKEND=${SENTRY_BACKEND}"
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE sentry)
endif()
