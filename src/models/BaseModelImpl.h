//
// Created by ihor on 2/10/24.
//
#pragma once
#include <string>
#include <json/value.h>
#include <chrono>
#include "decimal.h"
#include "QuerySet.h"

namespace api::v1 {

    class BaseModelImpl {
    public:
        virtual ~BaseModelImpl() = default;

    protected:
        struct ModelFieldHasher {
            std::size_t operator()(std::string_view sv) const;
        };

        template<class V>
        void validateField(const std::string &fieldName, const V &value, Json::Value &fields) const;
        [[nodiscard]] virtual std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const;
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
