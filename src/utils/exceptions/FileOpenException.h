#ifndef FILEOPENEXCEPTION_H
#define FILEOPENEXCEPTION_H

#include "BaseException.h"
#include "fmt/format.h"

class FileOpenException : public BaseException {
public:
    explicit FileOpenException(const std::string& filename) {
        message = fmt::format("Failed to open file: {}", filename);
    }
};

#endif  // FILEOPENEXCEPTION_H
