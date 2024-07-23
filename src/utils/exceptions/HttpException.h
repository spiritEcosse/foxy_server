#ifndef HTTPEXCEPTION_H
#define HTTPEXCEPTION_H

#include "backward.hpp"
#include <exception>
#include <string>
#include <sstream>
#include <utility>
#include "sentryHelper.h"
#include "fmt/format.h"

class HttpException : public std::exception {
private:
    std::string message;
    backward::StackTrace st;
    backward::TraceResolver resolver;  // Step 2: Declare the TraceResolver

public:
    HttpException(long code, const std::string& response) {
        st.load_here(32);  // Capture up to 32 stack frames
        resolver.load_stacktrace(st);  // Load the stack trace into the resolver
        std::stringstream ss;
        ss << "HTTP request failed with code " << code << " and response: " << response;
        message = ss.str();
    }

    const char* what() const noexcept override {
        return message.c_str();
    }

    void printStackTrace(std::ostream& os) {
        sentryHelper(message, "HttpException");
        os << message << "\n";

        backward::Printer p;
        p.object = true;
        p.color_mode = backward::ColorMode::always;
        p.address = true;
        p.snippet = true;
        p.print(st, std::cerr);
    }
};

#endif  // HTTPEXCEPTION_H
