#include <drogon/drogon.h>
#include <future>
#include <thread>
#include "drogon/drogon_test.h"
#include <fstream>
#include <sstream>
#include <JWT.h>
#include "fmt/format.h"
#include "bcrypt.h"
#include <iomanip>
#include <utility>
#include <variant>

using namespace drogon;
using namespace api::utils::jwt;

const std::string host = "http://127.0.0.1:8850";
const std::string userEmail = "user1@example.com";
using fieldsValMap = std::map<std::string, std::variant<int, bool, std::string, double>>;

std::string convertDateTimeFormat(const std::string &input) {
    std::tm tm = {};
    std::stringstream ss(input);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    std::stringstream output;
    output << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");

    // Handle fractional seconds separately
    if(input.size() > 19) {  // If fractional seconds are present
        size_t pos = input.find('.');
        if(pos != std::string::npos) {
            output << input.substr(pos);
        }
    }

    return output.str();
}

void executeCommandsFromSqlFile(const std::shared_ptr<orm::DbClient> &dbClient, const std::string &sqlFilePath) {
    try {
        // Open the SQL file
        std::ifstream sqlFile(sqlFilePath);

        // Check if the file was opened successfully
        if(!sqlFile) {
            throw std::invalid_argument("Couldn't open SQL file");
        }

        // Read the entire file into a string
        std::stringstream sqlCommands;
        sqlCommands << sqlFile.rdbuf();

        // Close the file
        sqlFile.close();
        dbClient->execSqlSync(sqlCommands.str());
    } catch(const std::system_error &e) {
        LOG_ERROR << e.what();
    } catch(const std::invalid_argument &e) {
        LOG_ERROR << e.what();
    }
}

void setUpBeforeEachTest(const std::shared_ptr<orm::DbClient> &dbClient) {
    executeCommandsFromSqlFile(dbClient, "./helper.sql");
    executeCommandsFromSqlFile(dbClient, "./fixtures.sql");
}

DROGON_TEST(Create) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkFields = [TEST_CTX,
                            dbClient](const HttpResponsePtr &resp, fieldsValMap &expectedValues, std::string entity) {
            std::cout << entity << std::endl;
            auto respJson = *resp->getJsonObject();
            Json::StreamWriterBuilder builder;
            const std::string jsonString = Json::writeString(builder, respJson);
            std::cout << jsonString << std::endl;
            REQUIRE(resp->getStatusCode() == k201Created);
            REQUIRE(resp->contentType() == CT_APPLICATION_JSON);
            REQUIRE(dbClient != nullptr);

            auto result = dbClient->execSqlSync(
                fmt::format(R"(SELECT created_at, updated_at FROM "{}" ORDER BY id desc LIMIT 1)", std::move(entity)));
            expectedValues["created_at"] = convertDateTimeFormat(result[0]["created_at"].as<std::string>());
            expectedValues["updated_at"] = convertDateTimeFormat(result[0]["updated_at"].as<std::string>());

            for(const auto &[key, value]: expectedValues) {
                std::visit(
                    [&](const auto &arg) {
                        using Type = std::decay_t<decltype(arg)>;
                        if constexpr(std::is_same_v<Type, int>) {
                            CHECK(respJson[key].asInt() == std::get<int>(value));
                        } else if constexpr(std::is_same_v<Type, bool>) {
                            CHECK(respJson[key].asBool() == std::get<bool>(value));
                        } else if constexpr(std::is_same_v<Type, std::string>) {
                            if(key == "src") {
                                CHECK(respJson[key].asString() ==
                                      fmt::format("https:///{}", std::get<std::string>(value)));
                            } else if(key != "password") {
                                CHECK(respJson[key].asString() == std::get<std::string>(value));
                            }
                        } else if constexpr(std::is_same_v<Type, double>) {
                            CHECK(respJson[key].asDouble() == std::get<double>(value));
                        }
                    },
                    value);
            }
        };

        auto sendHttpRequest = [TEST_CTX,
                                checkFields](std::string path, fieldsValMap &expectedValues, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Post);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
            Json::Value jsonValue;
            for(const auto &[key, value]: expectedValues) {
                std::visit(
                    [&](const auto &arg) {
                        jsonValue[key] = arg;
                    },
                    value);
            }

            Json::StreamWriterBuilder writer;
            std::string jsonString = Json::writeString(writer, jsonValue);
            req->setBody(std::move(jsonString));
            client->sendRequest(
                req,
                [TEST_CTX, checkFields, expectedValues, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                    REQUIRE(res == ReqResult::Ok);
                    REQUIRE(resp != nullptr);
                    checkFields(resp, expectedValues, std::move(entity));
                });
        };

        Json::StreamWriterBuilder writer;
        std::string path = "/api/v1/item/admin";
        std::string entity = "item";
        fieldsValMap expectedValues = {
            {std::string("description"), "mock description"},
            {std::string("meta_description"), "mock meta description"},
            {std::string("price"), 100.0},
            {std::string("shipping_profile_id"), 1},
            {std::string("slug"), "mock-slug"},
            {std::string("title"), "mock title"},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/page/admin";
        entity = "page";
        expectedValues = {
            {std::string("description"), "mock description"},
            {std::string("meta_description"), "mock meta description"},
            {std::string("canonical_url"), "mock canonical url"},
            {std::string("slug"), "mock-slug"},
            {std::string("title"), "mock title"},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/user/admin";
        entity = "user";
        expectedValues = {
            {std::string("email"), "mock email"},
            {std::string("password"), "mock password"},
            {std::string("first_name"), "mock first name"},
            {std::string("last_name"), "mock last name"},
            {std::string("birthday"), "2024-01-14"},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/shippingprofile/admin";
        entity = "shipping_profile";
        expectedValues = {
            {std::string("title"), "mock title"},
            {std::string("processing_time"), 1},
            {std::string("country_id"), 1},
            {std::string("postal_code"), "mock postal code"},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/shippingrate/admin";
        entity = "shipping_rate";
        expectedValues = {
            {std::string("shipping_profile_id"), 1},
            {std::string("delivery_days_min"), 1},
            {std::string("delivery_days_max"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/review/admin";
        entity = "review";
        expectedValues = {
            {std::string("status"), "failed"},
            {std::string("user_id"), 1},
            {std::string("comment"), "mock comment"},
            {std::string("item_id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        // create basket 3
        dbClient->execSqlSync("INSERT INTO basket (user_id) VALUES (2)");
        entity = "order";
        path = "/api/v1/order/admin";
        expectedValues = {
            {std::string("status"), "completed"},
            {std::string("basket_id"), 3},
            {std::string("total"), 1.0},
            {std::string("total_ex_taxes"), 1.0},
            {std::string("tax_rate"), 1.0},
            {std::string("taxes"), 1.0},
            {std::string("user_id"), 1},
            {std::string("reference"), "mock reference"},
            {std::string("address_id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/media/admin";
        entity = "media";
        expectedValues = {
            {std::string("src"), "mock_src"},
            {std::string("item_id"), 1},
            {std::string("sort"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/country/admin";
        entity = "country";
        expectedValues = {
            {std::string("title"), "mock title"},
            {std::string("code"), "mock code"},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/basket/admin";
        entity = "basket";
        expectedValues = {
            {std::string("user_id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/basketitem/admin";
        entity = "basket_item";
        expectedValues = {
            {std::string("basket_id"), 3},
            {std::string("item_id"), 1},
            {std::string("quantity"), 1},
            {std::string("price"), 1.0},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/address/admin";
        entity = "address";
        expectedValues = {
            {std::string("address"), "mock address"},
            {std::string("country_id"), 1},
            {std::string("city"), "mock city"},
            {std::string("zipcode"), "mock zipcode"},
            {std::string("user_id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);
    });
};

DROGON_TEST(CheckMissingFields) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);
    });

    auto checkFields = [TEST_CTX](const HttpResponsePtr &resp, const std::vector<std::string> &missingFields) {
        REQUIRE(resp->getStatusCode() == k400BadRequest);
        REQUIRE(resp->contentType() == CT_APPLICATION_JSON);
        auto respJson = *resp->jsonObject();

        for(const auto &key: missingFields) {
            CHECK(respJson[key] == key + " is required");
        }

        for(const auto &key: respJson.getMemberNames()) {
            bool isKeyMissing = std::find(missingFields.begin(), missingFields.end(), key) != missingFields.end();
            CHECK(isKeyMissing);
        }
    };

    auto sendHttpRequest = [TEST_CTX, checkFields](std::string path, const std::vector<std::string> &missingFields) {
        auto client = HttpClient::newHttpClient(host);
        auto req = HttpRequest::newHttpRequest();
        req->setPath(std::move(path));
        req->setMethod(drogon::Post);
        JWT jwtGenerated = JWT::generateToken({
            {"email", picojson::value(userEmail)},
        });

        // Set the Authorization header
        req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
        req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        req->setBody("{}");
        client->sendRequest(req,
                            [TEST_CTX, checkFields, missingFields](ReqResult res, const HttpResponsePtr &resp) mutable {
                                REQUIRE(res == ReqResult::Ok);
                                REQUIRE(resp != nullptr);
                                checkFields(resp, missingFields);
                            });
    };

    std::vector<std::string> missingFields =
        {"description", "meta_description", "price", "shipping_profile_id", "slug", "title"};
    std::string path = "/api/v1/item/admin";
    sendHttpRequest(path, missingFields);

    missingFields = {"title", "description", "meta_description", "canonical_url", "slug"};
    path = "/api/v1/page/admin";
    sendHttpRequest(path, missingFields);

    missingFields = {"email", "password", "first_name", "last_name", "birthday"};
    path = "/api/v1/user/admin";
    sendHttpRequest(path, missingFields);

    missingFields = {"title", "processing_time", "country_id", "postal_code"};
    path = "/api/v1/shippingprofile/admin";
    sendHttpRequest(path, missingFields);

    missingFields = {"shipping_profile_id", "delivery_days_min", "delivery_days_max"};
    path = "/api/v1/shippingrate/admin";
    sendHttpRequest(path, missingFields);

    missingFields = {"status", "user_id", "comment", "item_id"};
    path = "/api/v1/review/admin";
    sendHttpRequest(path, missingFields);

    missingFields = {"basket_id", "total", "total_ex_taxes", "tax_rate", "taxes", "user_id", "reference", "address_id"};
    path = "/api/v1/order/admin";
    sendHttpRequest(path, missingFields);

    missingFields = {"src", "item_id", "sort"};
    path = "/api/v1/media/admin";
    sendHttpRequest(path, missingFields);

    missingFields = {"title", "code"};
    path = "/api/v1/country/admin";
    sendHttpRequest(path, missingFields);

    missingFields = {"user_id"};
    path = "/api/v1/basket/admin";
    sendHttpRequest(path, missingFields);

    missingFields = {"basket_id", "item_id"};
    path = "/api/v1/basketitem/admin";
    sendHttpRequest(path, missingFields);

    missingFields = {"country_id", "address", "city", "zipcode", "user_id"};
    path = "/api/v1/address/admin";
    sendHttpRequest(path, missingFields);
}

DROGON_TEST(Update) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkFields = [TEST_CTX,
                            dbClient](const HttpResponsePtr &resp, fieldsValMap &expectedValues, std::string entity) {
            std::cout << entity << std::endl;
            auto respJson = *resp->getJsonObject();
            Json::StreamWriterBuilder builder;
            const std::string jsonString = Json::writeString(builder, respJson);
            std::cout << jsonString << std::endl;
            REQUIRE(resp->getStatusCode() == k200OK);
            REQUIRE(resp->contentType() == CT_APPLICATION_JSON);
            REQUIRE(dbClient != nullptr);

            auto result = dbClient->execSqlSync(
                fmt::format(R"(SELECT created_at, updated_at FROM "{}" WHERE id = 1)", std::move(entity)));
            expectedValues["created_at"] = convertDateTimeFormat(result[0]["created_at"].as<std::string>());
            expectedValues["updated_at"] = convertDateTimeFormat(result[0]["updated_at"].as<std::string>());

            for(const auto &[key, value]: expectedValues) {
                std::visit(
                    [&](const auto &arg) {
                        using Type = std::decay_t<decltype(arg)>;
                        if constexpr(std::is_same_v<Type, int>) {
                            CHECK(respJson[key].asInt() == std::get<int>(value));
                        } else if constexpr(std::is_same_v<Type, bool>) {
                            CHECK(respJson[key].asBool() == std::get<bool>(value));
                        } else if constexpr(std::is_same_v<Type, std::string>) {
                            if(key == "src") {
                                CHECK(respJson[key].asString() ==
                                      fmt::format("https:///{}", std::get<std::string>(value)));
                            } else if(key != "password") {
                                CHECK(respJson[key].asString() == std::get<std::string>(value));
                            }
                        } else if constexpr(std::is_same_v<Type, double>) {
                            CHECK(respJson[key].asDouble() == std::get<double>(value));
                        }
                    },
                    value);
            }
        };

        auto sendHttpRequest = [TEST_CTX,
                                checkFields](std::string path, fieldsValMap &expectedValues, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Put);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
            Json::Value jsonValue;
            for(const auto &[key, value]: expectedValues) {
                std::visit(
                    [&](const auto &arg) {
                        jsonValue[key] = arg;
                    },
                    value);
            }

            Json::StreamWriterBuilder writer;
            std::string jsonString = Json::writeString(writer, jsonValue);
            req->setBody(std::move(jsonString));
            client->sendRequest(
                req,
                [TEST_CTX, checkFields, expectedValues, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                    REQUIRE(res == ReqResult::Ok);
                    REQUIRE(resp != nullptr);
                    checkFields(resp, expectedValues, std::move(entity));
                });
        };

        std::string path = "/api/v1/item/admin/1";
        std::string entity = "item";
        fieldsValMap expectedValues = {
            {std::string("description"), "mock description"},
            {std::string("meta_description"), "mock meta description"},
            {std::string("price"), 100.0},
            {std::string("shipping_profile_id"), 1},
            {std::string("slug"), "mock-slug"},
            {std::string("title"), "mock title"},
            {std::string("enabled"), true},
            {std::string("id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/page/admin/1";
        entity = "page";
        expectedValues = {
            {std::string("description"), "mock description"},
            {std::string("meta_description"), "mock meta description"},
            {std::string("canonical_url"), "mock canonical url"},
            {std::string("slug"), "mock-slug"},
            {std::string("title"), "mock title"},
            {std::string("id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/user/admin/1";
        entity = "user";
        expectedValues = {
            {std::string("email"), "mock email"},
            {std::string("password"), "mock password"},
            {std::string("first_name"), "mock first name"},
            {std::string("last_name"), "mock last name"},
            {std::string("birthday"), "2024-01-14"},
            {std::string("id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/shippingprofile/admin/1";
        entity = "shipping_profile";
        expectedValues = {
            {std::string("title"), "mock title"},
            {std::string("processing_time"), 1},
            {std::string("country_id"), 1},
            {std::string("postal_code"), "mock postal code"},
            {std::string("id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/shippingrate/admin/1";
        entity = "shipping_rate";
        expectedValues = {
            {std::string("shipping_profile_id"), 1},
            {std::string("delivery_days_min"), 1},
            {std::string("delivery_days_max"), 1},
            {std::string("id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/review/admin/1";
        entity = "review";
        expectedValues = {
            {std::string("status"), "failed"},
            {std::string("user_id"), 1},
            {std::string("comment"), "mock comment"},
            {std::string("item_id"), 1},
            {std::string("id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        // create basket 3
        dbClient->execSqlSync("INSERT INTO basket (user_id) VALUES (2)");
        path = "/api/v1/order/admin/1";
        entity = "order";
        expectedValues = {
            {std::string("status"), "completed"},
            {std::string("basket_id"), 3},
            {std::string("total"), 1.0},
            {std::string("total_ex_taxes"), 1.0},
            {std::string("tax_rate"), 1.0},
            {std::string("taxes"), 1.0},
            {std::string("user_id"), 1},
            {std::string("reference"), "mock reference"},
            {std::string("address_id"), 1},
            {std::string("id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/media/admin/1";
        entity = "media";
        expectedValues = {
            {std::string("src"), "mock_src"},
            {std::string("item_id"), 1},
            {std::string("sort"), 1},
            {std::string("id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/country/admin/1";
        entity = "country";
        expectedValues = {
            {std::string("title"), "mock title"},
            {std::string("code"), "mock code"},
            {std::string("id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/basket/admin/1";
        entity = "basket";
        expectedValues = {
            {std::string("user_id"), 1},
            {std::string("id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/basketitem/admin/1";
        entity = "basket_item";
        expectedValues = {
            {std::string("basket_id"), 1},
            {std::string("item_id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);

        path = "/api/v1/address/admin/1";
        entity = "address";
        expectedValues = {
            {std::string("address"), "mock address"},
            {std::string("city"), "mock city"},
            {std::string("country_id"), 1},
            {std::string("zipcode"), "mock zipcode"},
            {std::string("user_id"), 1},
            {std::string("id"), 1},
        };
        sendHttpRequest(path, expectedValues, entity);
    });
};

DROGON_TEST(DeleteItem) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/item/admin/1", "item");
    });
};

DROGON_TEST(DeletePage) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/page/admin/1", "page");
    });
};

DROGON_TEST(DeleteUser) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/user/admin/1", "user");
    });
};

DROGON_TEST(DeleteShippingProfile) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/shippingprofile/admin/1", "shipping_profile");
    });
};

DROGON_TEST(DeleteShippingRate) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/shippingrate/admin/1", "shipping_rate");
    });
};

DROGON_TEST(DeleteReview) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/review/admin/1", "review");
    });
};

DROGON_TEST(DeleteOrder) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/order/admin/1", "order");
    });
};

DROGON_TEST(DeleteMedia) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/media/admin/1", "media");
    });
};

DROGON_TEST(DeleteCountry) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/country/admin/1", "country");
    });
};

DROGON_TEST(DeleteBasket) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/basket/admin/1", "basket");
    });
};

DROGON_TEST(DeleteBasketItem) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/basketitem/admin/1", "basket_item");
    });
};

DROGON_TEST(DeleteAddress) {
    drogon::app().getLoop()->runInLoop([TEST_CTX]() {
        const auto dbClient = drogon::app().getDbClient("tests");
        setUpBeforeEachTest(dbClient);

        auto checkDbObject = [TEST_CTX, dbClient](const HttpResponsePtr &resp, std::string entity) {
            std::cout << entity << std::endl;
            REQUIRE(resp->getStatusCode() == k204NoContent);
            REQUIRE(dbClient != nullptr);
            auto result = dbClient->execSqlSync(fmt::format(R"(SELECT id FROM "{}" WHERE id = 1)", std::move(entity)));
            REQUIRE(result.empty());
        };

        auto sendHttpRequest = [TEST_CTX, checkDbObject](std::string path, std::string entity) {
            auto client = HttpClient::newHttpClient(host);
            auto req = HttpRequest::newHttpRequest();
            req->setPath(std::move(path));
            req->setMethod(drogon::Delete);
            JWT jwtGenerated = JWT::generateToken({
                {"email", picojson::value(userEmail)},
            });
            // Set the Authorization header
            req->addHeader("Authorization", fmt::format("Bearer {}", jwtGenerated.getToken()));
            req->setContentTypeCode(drogon::CT_APPLICATION_JSON);

            client->sendRequest(req,
                                [TEST_CTX, checkDbObject, entity](ReqResult res, const HttpResponsePtr &resp) mutable {
                                    REQUIRE(res == ReqResult::Ok);
                                    REQUIRE(resp != nullptr);
                                    checkDbObject(resp, std::move(entity));
                                });
        };

        sendHttpRequest("/api/v1/address/admin/1", "address");
    });
};

int main(int argc, char **argv) {
    // Initialize the Drogon application
    app().loadConfigFile("./config.json");

    std::promise<void> appStartedPromise;
    std::future<void> appStartedFuture = appStartedPromise.get_future();

    // Start the main loop on another thread
    std::thread appThread([&appStartedPromise]() {
        // Queues the promise to be fulfilled after starting the loop
        app().getLoop()->queueInLoop([&appStartedPromise]() {
            appStartedPromise.set_value();
        });

        // Run the event loop
        app().run();
    });

    // Wait until the event loop has started
    appStartedFuture.get();

    // Run the tests
    int status = test::run(argc, argv);

    // Ask the event loop to shut down and wait
    app().getLoop()->queueInLoop([]() {
        app().quit();
    });

    // Wait for the application thread to finish
    appThread.join();

    return status;
}
