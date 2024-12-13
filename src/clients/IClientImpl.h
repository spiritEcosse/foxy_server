#pragma once

#include "FileTransferInfo.h"
#include <string>
#include <cpr/cpr.h>
#include "TransparentStringHash.h"

namespace api::v1 {
    class IClientImpl : public BaseClass {
    public:
        using BaseClass::BaseClass;

        [[nodiscard]] static std::string
        calculateOAuthSignature(const std::string_view httpMethod,
                                const std::string_view url,
                                const std::map<std::string, std::string, std::less<>>& params,
                                const std::string_view consumerSecret,
                                const std::string_view tokenSecret);
        [[nodiscard]] static std::string urlEncode(const std::string_view value);
        static bool checkResponses(const std::vector<cpr::Response>& responses, int status_code = 200);
        [[nodiscard]] static bool parseJson(const cpr::Response& response, Json::Value& jsonResponse);
        [[nodiscard]] static bool
        fieldIsMember(const std::string& field, const cpr::Response& response, const Json::Value& jsonResponse);
    };
}
