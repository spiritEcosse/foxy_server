//
// Created by ihor on 13.01.2024.
//

#ifndef BASE_H
#define BASE_H

#include <drogon/drogon.h>

namespace api::v1 {
    template<class T, class R>
    class BaseCRUD {
    public:
        virtual ~BaseCRUD() = default;
        virtual void getOne(const drogon::HttpRequestPtr &req,
                            std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                            const std::string &) const;
        virtual void createItem(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
        virtual void createItems(const drogon::HttpRequestPtr &req,
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
        virtual void getList(const drogon::HttpRequestPtr &req,
                             std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
        virtual void updateItem(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                const std::string &) const;
        virtual void deleteItem(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                const std::string &) const;
        [[nodiscard]] static Json::Value getJsonResponse(const drogon::orm::Result &r);
    };
}

#endif  //BASE_H
