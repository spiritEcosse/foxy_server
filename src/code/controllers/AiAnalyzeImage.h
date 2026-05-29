#pragma once

#include "drogon/HttpController.h"

#include <list>
#include <mutex>
#include <thread>

namespace api::v1 {
    class AiAnalyzeImage final : public drogon::HttpController<AiAnalyzeImage> {
        // Class-scope statics with dynamic init force this TU to emit a
        // _GLOBAL__sub_I_ initializer, which also constructs the inherited
        // HttpController<T>::registrator_ — without it, the route 404s
        // (see commit 2afed0c and PinterestOAuth pattern).
        static inline std::mutex workerMutex{};
        static inline std::list<std::jthread> workerThreads{};

    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(AiAnalyzeImage::analyze,
                   "admin",
                   drogon::Post,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_LIST_END

        void analyze(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
    };
}  // namespace api::v1
