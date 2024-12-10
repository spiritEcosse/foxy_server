#pragma once

#include "IClient.h"
#include "env.h"

namespace api::v1 {
    class Pin;

    class PinterestClient final : public IClient<PinterestClient, Pin> {
        std::string accessToken;
        std::string auth() override;

    public:
        static constexpr std::string_view apiCreatePost = "https://api.pinterest.com/v5/pins";
        static constexpr std::string_view clientName = "Pinterest";

        PinterestClient() {
            getenv("PINTEREST_ACCESS_TOKEN", accessToken);
        }
    };
}
