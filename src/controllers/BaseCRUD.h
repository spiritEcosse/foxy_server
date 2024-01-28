//
// Created by ihor on 13.01.2024.
//

#ifndef BASE_H
#define BASE_H

#include <drogon/drogon.h>

namespace api::v1 {
    template<class T, class R>
    class BaseCRUD {
    protected:
        void handleSqlResult(const drogon::orm::Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr, bool isCreate) const;
        void executeSqlQuery(std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr, const std::string& query, bool isCreate) const;
        void handleSqlError(const drogon::orm::DrogonDbException &e, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const;
        [[nodiscard]] virtual drogon::HttpResponsePtr checkBody(const drogon::HttpRequestPtr &req) const;
        [[nodiscard]] static Json::Value getJsonResponse(const drogon::orm::Result &r);
        void getItem(
            const drogon::HttpRequestPtr &req, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr, std::function<void(T)> successCallback) const;
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
    };
}

#endif  //BASE_H
