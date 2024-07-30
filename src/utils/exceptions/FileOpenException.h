#include <stdexcept>

class FileOpenException : public std::runtime_error {
public:
    explicit FileOpenException(const std::string& message) : std::runtime_error(message) {}
};
