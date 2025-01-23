set(ENV_VARS
        PINTEREST_ACCESS_TOKEN
        PINTEREST_BOARD_ID
        PINTEREST_API_HOST
        FOXY_CLIENT
        FOXY_ADMIN
        FOXY_HTTP_PORT
        APP_CLOUD_NAME
        CONFIG_APP_PATH
        ENVIRONMENT
        YOUTUBE_ACCESS_TOKEN
        APP_BUCKET_HOST
        TWITTER_API_KEY
        TWITTER_API_SECRET
        TWITTER_ACCESS_TOKEN
        TWITTER_ACCESS_TOKEN_SECRET
        TWITTER_BEARER_TOKEN
)

foreach (VAR IN LISTS ENV_VARS)
    set(${VAR} "$ENV{${VAR}}")
    if (NOT ${VAR})
        message(FATAL_ERROR "${VAR} environment variable is not set.")
    endif ()
    target_compile_definitions(${PROJECT_NAME} PUBLIC -D${VAR}="${${VAR}}")
endforeach ()

if (DEFINED ENV{SENTRY_DSN})
    set(SENTRY_DSN "$ENV{SENTRY_DSN}")
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DSENTRY_DSN="${SENTRY_DSN}")
endif ()
