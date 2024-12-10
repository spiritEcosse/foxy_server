#pragma once
#include "BaseException.h"
#include "fmt/format.h"

namespace api::v1 {
    class FileOpenException final : public BaseException {
    public:
        explicit FileOpenException(const std::string& filename) {
            setMessage(fmt::format("Failed to open file: {}", filename));
        }
    };
}