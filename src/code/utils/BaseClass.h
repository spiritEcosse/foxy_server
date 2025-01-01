#pragma once
#include <vector>

namespace api::v1 {
    class BaseClass {
    public:
        BaseClass() = default;
        BaseClass(const BaseClass&) = delete;
        BaseClass& operator=(const BaseClass&) = delete;
        BaseClass(BaseClass&&) noexcept = default;
        BaseClass& operator=(BaseClass&&) noexcept = default;
        virtual ~BaseClass() = default;

        template<typename T>
        std::vector<T> concatVectors(const std::vector<T>& v1, const std::vector<T>& v2) const {
            std::vector<T> result(v1);
            result.insert(result.end(), v2.begin(), v2.end());
            return result;
        }
    };
}
