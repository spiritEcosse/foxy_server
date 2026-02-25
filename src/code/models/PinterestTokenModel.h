#pragma once

#include "models/BaseModel.h"

namespace api::v1 {
    class PinterestTokenModel final : public BaseModel<PinterestTokenModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "pinterest_token";

        struct Field : BaseModel::Field {
            static inline const auto accessToken = makeField("access_token");
            static inline const auto accessTokenExpiresAt = makeField("access_token_expires_at");
            static inline const auto refreshToken = makeField("refresh_token");
            static inline const auto refreshTokenExpiresAt = makeField("refresh_token_expires_at");
            static inline const auto scope = makeField("scope");
            static inline const auto singleton = makeField("singleton");

            Field() : BaseModel::Field() {
                constexpr std::array fields{&accessToken,
                                            &accessTokenExpiresAt,
                                            &refreshToken,
                                            &refreshTokenExpiresAt,
                                            &scope,
                                            &singleton};
                registerFields(fields);
            }
        };

        std::string accessToken;
        std::chrono::system_clock::time_point accessTokenExpiresAt;
        std::string refreshToken;
        std::chrono::system_clock::time_point refreshTokenExpiresAt;
        std::string scope;

        explicit PinterestTokenModel(std::string accessToken,
                                     std::chrono::system_clock::time_point accessTokenExpiresAt,
                                     std::string refreshToken,
                                     std::chrono::system_clock::time_point refreshTokenExpiresAt,
                                     std::string scope);

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] std::string sqlUpsert();
    };
}
