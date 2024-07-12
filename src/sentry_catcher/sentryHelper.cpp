#include "sentryHelper.h"

// Definition moved from the header file
void sentryHelper(const std::string& error, const std::string& logger) {
    LOG_ERROR << logger << ", " << error;
    sentry_capture_event(sentry_value_new_message_event(
        /*   level */ SENTRY_LEVEL_ERROR,
        /*  logger */ logger.c_str(),
        /* message */ error.c_str()));
}
