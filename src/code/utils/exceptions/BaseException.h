#pragma once

#include "backward-cpp/backward.hpp"
#include <exception>
#include <string>
#include <sstream>
#include "sentryHelper.h"

namespace api::v1 {
    class BaseException : public std::exception, public BaseClass {
    private:
        std::string message = "BaseException message";
        backward::StackTrace st;
        backward::TraceResolver resolver;

    protected:
        [[nodiscard]] const std::string& getMessage() const {
            return message;
        }

        void setMessage(std::string_view msg) {
            message = msg;
        }

        [[nodiscard]] const backward::StackTrace& getStackTrace() const {
            return st;
        }

        [[nodiscard]] const backward::TraceResolver& getResolver() const {
            return resolver;
        }

    public:
        BaseException() {
            st.load_here(32);
            resolver.load_stacktrace(st);
        }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }

        virtual void printStackTrace(std::ostream& os) const {
            os << message << "\n";
            backward::Printer p;
            p.object = true;
            p.color_mode = backward::ColorMode::always;
            p.address = true;
            p.snippet = true;
            p.print(st, os);
            sentryHelper(message, "BaseException");
        }
    };
}