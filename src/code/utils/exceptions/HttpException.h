#pragma once

#include "BaseException.h"
#include "fmt/format.h"

namespace api::v1 {
    class HttpException final : public BaseException {
    public:
        HttpException(long code, const std::string& response) {
            setMessage(fmt::format("HTTP request failed with code {} and response: {}", code, response));
        }
    };
}
