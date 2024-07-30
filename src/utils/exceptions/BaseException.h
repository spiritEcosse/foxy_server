#ifndef BASEEXCEPTION_H
#define BASEEXCEPTION_H

#include "backward-cpp/backward.hpp"
#include <exception>
#include <string>
#include <sstream>
#include "sentryHelper.h"

class BaseException : public std::exception {
protected:
    std::string message;
    backward::StackTrace st;
    backward::TraceResolver resolver;

public:
    BaseException() : message("BaseException message") {
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

#endif  // BASEEXCEPTION_H
