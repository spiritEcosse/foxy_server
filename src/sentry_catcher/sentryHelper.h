#pragma once

#include <sentry.h>
#include <string>
#include <drogon/drogon.h>

// Declaration only
void sentryHelper(const std::string& error, const std::string& logger);
void sentryHelper(const std::exception& error, const std::string& logger);
