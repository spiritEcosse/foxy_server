#include "sentryHelper.h"
#include "fmt/format.h"

// Definition moved from the header file
void sentryHelper(const std::string& error, const std::string& logger) {
    LOG_ERROR << "logger: " << logger << ", error: " << error;
#if defined(SENTRY_DSN)
    sentry_capture_event(sentry_value_new_message_event(
        /*   level */ SENTRY_LEVEL_ERROR,
        /*  logger */ logger.c_str(),
        /* message */ error.c_str()));
#endif
}

void sentryHelper(const std::exception& e, const std::string& logger) {
    const std::string errorMessage = fmt::format("Exception: {}, Message: {}", typeid(e).name(), e.what());
    LOG_ERROR << "logger: " << logger << ", error: " << errorMessage;
#if defined(SENTRY_DSN)
    sentry_capture_event(sentry_value_new_message_event(
        /*   level */ SENTRY_LEVEL_ERROR,
        /*  logger */ logger.c_str(),
        /* message */ errorMessage.c_str()));
#endif
}
