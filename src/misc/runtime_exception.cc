#include <utility>

#include "d0/misc/runtime_exception.h"

namespace d0 {

RuntimeException::RuntimeException(std::string message)
    : message_{std::move(message)} {}

RuntimeException::~RuntimeException() = default;

const char *RuntimeException::what() const { return message_.c_str(); }

} // namespace d0