//
// Created by ihor on 15.01.2024.
//

#ifndef REQUIREDFIELDSEXCEPTION_H
#define REQUIREDFIELDSEXCEPTION_H

#include <exception>
#include <nlohmann/json.hpp>
#include <utility>

class RequiredFieldsException : public std::exception {
private:
    Json::Value requiredFields;

public:
    explicit RequiredFieldsException(Json::Value fields) : requiredFields(std::move(fields)) {}

    [[nodiscard]] const char* what() const noexcept override {
        return "Required fields are missing";
    }

    [[nodiscard]] Json::Value getRequiredFields() const {
        return requiredFields;
    }
};

#endif  //REQUIREDFIELDSEXCEPTION_H
