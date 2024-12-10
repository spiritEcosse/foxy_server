#pragma once

namespace api::v1 {
    class BaseClass {
    public:
        BaseClass() = default;
        BaseClass(const BaseClass&) = delete;
        BaseClass& operator=(const BaseClass&) = delete;
        BaseClass(BaseClass&&) noexcept = default;
        BaseClass& operator=(BaseClass&&) noexcept = default;
        virtual ~BaseClass() = default;
    };
}
