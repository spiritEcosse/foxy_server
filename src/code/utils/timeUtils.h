#pragma once
#include <chrono>

long long getCurrentTimeSinceEpochInMilliseconds() {
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    return ms.count();
}
