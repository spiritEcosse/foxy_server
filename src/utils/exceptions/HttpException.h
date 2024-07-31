#ifndef HTTPEXCEPTION_H
#define HTTPEXCEPTION_H

#include "BaseException.h"
#include "fmt/format.h"

class HttpException : public BaseException {
public:
    HttpException(long code, const std::string& response) {
        setMessage(fmt::format("HTTP request failed with code {} and response: {}", code, response));
    }
};

#endif  // HTTPEXCEPTION_H
