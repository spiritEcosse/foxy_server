#include <fmt/chrono.h>
#include <fmt/core.h>

#include "BaseModelImpl.h"

namespace api::v1 {

    std::string BaseModelImpl::timePointToString(std::chrono::system_clock::time_point tp) {
        auto time_t = std::chrono::system_clock::to_time_t(tp);

        struct tm local_time {};

        localtime_r(&time_t, &local_time);

        std::string time_string = fmt::format("{:%Y-%m-%d %H:%M:%S}", local_time);

        auto duration = tp.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        duration -= seconds;
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

        return fmt::format("{}.{}", time_string, milliseconds.count());
    }

    BaseModelImpl::JoinMap BaseModelImpl::joinMap() const {
        return {};
    }

    std::size_t BaseModelImpl::ModelFieldHasher::operator()(std::string_view sv) const {
        std::hash<std::string_view> hasher;
        return hasher(sv);
    }

}
