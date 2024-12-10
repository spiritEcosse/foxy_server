#pragma once

#include <exception>
#include <utility>

class RequiredFieldsException final : public std::exception, api::v1::BaseClass {
private:
    Json::Value requiredFields;
    std::string message;

public:
    explicit RequiredFieldsException(Json::Value fields) : requiredFields(std::move(fields)) {}

    explicit RequiredFieldsException(std::string msg) : message(std::move(msg)) {}

    [[nodiscard]] const char* what() const noexcept override {
        return message.c_str();
    }

    [[nodiscard]] Json::Value getRequiredFields() const {
        return requiredFields;
    }
};
