#pragma once

#include <exception>
#include <format>
#include <string>

namespace d0 {

class RuntimeException : public std::exception {
public:
  RuntimeException(std::string message);
  template <typename... Args>
  RuntimeException(const std::string &format, Args &&...args)
      : RuntimeException(std::format(format, args...)) {}
  ~RuntimeException() override;

  [[nodiscard]] const char *what() const override;

private:
  std::string message_;
};

} // namespace d0