//
// Created by ihor on 15.01.2024.
//

#ifndef REQUIREDFIELDSEXCEPTION_H
#define REQUIREDFIELDSEXCEPTION_H

#include <exception>
#include <utility>

class RequiredFieldsException : public std::exception {
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

#endif  //REQUIREDFIELDSEXCEPTION_H
