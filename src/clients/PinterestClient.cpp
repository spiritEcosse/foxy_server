#include "PinterestClient.h"

namespace api::v1 {
    std::string PinterestClient::auth() {
        return fmt::format("Bearer {}", accessToken);
    }
}