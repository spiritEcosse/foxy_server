#pragma once
#include "BaseClass.h"

#include <string>
#include <json/value.h>
#include <chrono>
#include "decimal.h"
#include "QuerySet.h"
#include "TransparentStringHash.h"

#include <unordered_map>

namespace api::v1 {

    class BaseModelImpl : public BaseClass {
    public:
        using BaseClass::BaseClass;
        using AllFields =
            decltype(std::unordered_map<std::string, const BaseField *, TransparentStringHash, std::equal_to<>>());
        using JoinMap = decltype(std::unordered_map<std::string,
                                                    std::pair<const BaseClass *, const BaseClass *>,
                                                    TransparentStringHash,
                                                    std::equal_to<>>());

    protected:
        struct ModelFieldHasher {
            std::size_t operator()(std::string_view sv) const;
        };

        template<class V>
        void validateField(const std::string &fieldName, const V &value, Json::Value &fields) const;
        [[nodiscard]] virtual JoinMap joinMap() const;
        [[nodiscard]] static std::string timePointToString(std::chrono::system_clock::time_point tp);
    };

    template<class V>
    void BaseModelImpl::validateField(const std::string &fieldName, const V &value, Json::Value &fields) const {
        using VDecayed = std::decay_t<V>;
        if constexpr(std::is_same_v<VDecayed, int>) {
            if(!value) {
                fields[fieldName] = fieldName + " is required";
            }
        } else if constexpr(std::is_same_v<VDecayed, std::string_view> || std::is_same_v<VDecayed, std::string>) {
            if(value.empty()) {
                fields[fieldName] = fieldName + " is required";
            }
        } else if constexpr(std::is_same_v<VDecayed, dec::decimal<2>>) {
            if(value == dec::decimal<2>(0)) {
                fields[fieldName] = fieldName + " is required";
            }
        } else if constexpr(std::is_same_v<VDecayed, double>) {
            if(value == 0) {
                fields[fieldName] = fieldName + " is required";
            }
        }
    }

}
