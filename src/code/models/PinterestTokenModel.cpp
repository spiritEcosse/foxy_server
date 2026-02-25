#include "models/PinterestTokenModel.h"
#include <fmt/core.h>

using namespace api::v1;

PinterestTokenModel::PinterestTokenModel(std::string accessToken,
                                         std::chrono::system_clock::time_point accessTokenExpiresAt,
                                         std::string refreshToken,
                                         std::chrono::system_clock::time_point refreshTokenExpiresAt,
                                         std::string scope) :
    accessToken(std::move(accessToken)),
    accessTokenExpiresAt(accessTokenExpiresAt),
    refreshToken(std::move(refreshToken)),
    refreshTokenExpiresAt(refreshTokenExpiresAt),
    scope(std::move(scope)) {}

BaseModel<PinterestTokenModel>::SetMapFieldTypes PinterestTokenModel::getObjectValues() const {
    return {{&Field::accessToken, accessToken},
            {&Field::accessTokenExpiresAt, accessTokenExpiresAt},
            {&Field::refreshToken, refreshToken},
            {&Field::refreshTokenExpiresAt, refreshTokenExpiresAt},
            {&Field::scope, scope},
            {&Field::singleton, true}};
}

std::string PinterestTokenModel::sqlUpsert() {
    return fmt::format(
        R"(INSERT INTO "{}" ({}) VALUES {} ON CONFLICT (singleton) DO UPDATE SET access_token = EXCLUDED.access_token, access_token_expires_at = EXCLUDED.access_token_expires_at, refresh_token = EXCLUDED.refresh_token, refresh_token_expires_at = EXCLUDED.refresh_token_expires_at, scope = EXCLUDED.scope, updated_at = NOW() RETURNING json_build_object({}))",
        tableName,
        fieldsToString(),
        sqlInsertSingle(*this),
        fieldsJsonObject());
}
