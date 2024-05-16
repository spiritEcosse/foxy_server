#pragma once

#include <drogon/drogon.h>
#include <jwt-cpp/jwt.h>

namespace api::utils::jwt {
    class JWT {
    public:
        JWT(const std::string_view& token, const std::int64_t& expiration) : token(token), expiration(expiration) {}

        [[nodiscard]] std::string getToken() const {
            return this->token;
        }

        [[nodiscard]] std::int64_t getExpiration() const {
            return this->expiration;
        }

        static JWT
        generateToken(const std::map<std::string, ::jwt::traits::kazuho_picojson::value_type, std::less<>>& claims = {},
                      const bool& extension = false);
        static std::map<std::string, std::any, std::less<>> decodeToken(const std::string& encodedToken);
        [[nodiscard]] static std::tuple<drogon::HttpStatusCode, Json::Value>
        verifyGoogleToken(const std::string& token);

    private:
        std::string token;
        std::int64_t expiration;

        static bool verifyToken(const ::jwt::decoded_jwt<::jwt::traits::kazuho_picojson>& jwt);
        static void
        addClaimToAttributes(std::map<std::string, std::any, std::less<>>& attributes,
                             const std::pair<std::string, ::jwt::basic_claim<::jwt::traits::kazuho_picojson>>& claim);
    };
}
