#pragma once

#include <exception>
#include <string>
#include <sstream>
#include "sentry_catcher/sentryHelper.h"

namespace api::v1 {
    class BaseException : public std::exception, public BaseClass {
    private:
        std::string message = "BaseException message";

    protected:
        [[nodiscard]] const std::string& getMessage() const {
            return message;
        }

        void setMessage(std::string_view msg) {
            message = msg;
        }

    public:
        BaseException() = default;

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }

        virtual void printStackTrace(std::ostream& os) const {
            os << message << "\n";
            sentryHelper(message, "BaseException");
        }
    };
}
