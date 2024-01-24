//
// Created by ihor on 22.01.2024.
//

#ifndef TIME_H
#define TIME_H
#include <chrono>

long long getCurrentTimeSinceEpochInMilliseconds() {
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );
    return ms.count();
}

#endif //TIME_H
