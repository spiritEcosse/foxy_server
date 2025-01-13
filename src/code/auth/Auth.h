#pragma once

#include <drogon/drogon.h>
#include <drogon/HttpController.h>
#include "BaseCRUD.h"
#include <UserModel.h>

namespace api::v1 {
    class Auth final : public drogon::HttpController<Auth>, public BaseCRUD<UserModel, Auth> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Auth::googleLogin, "google_login", drogon::Post, drogon::Options);
        METHOD_ADD(Auth::googleLoginAdmin, "admin/google_login", drogon::Post, drogon::Options);
        METHOD_LIST_END

        void googleLogin(const drogon::HttpRequestPtr &request,
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
        void googleLoginAdmin(const drogon::HttpRequestPtr &request,
                              std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
    };
}